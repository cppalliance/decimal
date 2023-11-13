// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DECIMAL64_HPP
#define BOOST_DECIMAL_DECIMAL64_HPP

#include <cinttypes>
#include <cassert>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <cwchar>
#if !defined(BOOST_DECIMAL_DISABLE_IOSTREAM)
#include <iostream>
#endif
#include <limits>
#include <type_traits>
#include <sstream>

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/bit_cast.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/emulated128.hpp>
#include <boost/decimal/detail/fast_float/compute_float32.hpp>
#include <boost/decimal/detail/fast_float/compute_float64.hpp>
#include <boost/decimal/detail/fenv_rounding.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <boost/decimal/detail/parser.hpp>
#include <boost/decimal/detail/power_tables.hpp>
#include <boost/decimal/detail/ryu/ryu_generic_128.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/utilities.hpp>
#include <boost/decimal/detail/normalize.hpp>
#include <boost/decimal/detail/to_integral.hpp>
#include <boost/decimal/detail/to_float.hpp>
#include <boost/decimal/detail/to_decimal.hpp>
#include <boost/decimal/detail/promotion.hpp>
#include <boost/decimal/detail/io.hpp>
#include <boost/decimal/detail/comparison.hpp>
#include <boost/decimal/detail/mixed_decimal_arithmetic.hpp>
#include <boost/decimal/detail/check_non_finite.hpp>
#include <boost/decimal/detail/shrink_significand.hpp>
#include <boost/decimal/detail/cmath/isfinite.hpp>
#include <boost/decimal/detail/cmath/fpclassify.hpp>
#include <boost/decimal/detail/cmath/abs.hpp>
#include <boost/decimal/detail/cmath/floor.hpp>
#include <boost/decimal/detail/cmath/ceil.hpp>

namespace boost {
namespace decimal {
namespace detail {

// See IEEE 754 section 3.5.2
static constexpr auto d64_inf_mask = UINT64_C(0b0'11110'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
static constexpr auto d64_nan_mask = UINT64_C(0b0'11111'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
static constexpr auto d64_snan_mask = UINT64_C(0b0'11111'10000000'0000000000'0000000000'0000000000'0000000000'0000000000);
static constexpr auto d64_comb_inf_mask = UINT64_C(0b0'11110'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
static constexpr auto d64_comb_nan_mask = UINT64_C(0b0'11111'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
static constexpr auto d64_exp_snan_mask = UINT64_C(0b0'00000'10000000'0000000000'0000000000'0000000000'0000000000'0000000000);

// Masks to update the significand based on the combination field
// In these first three 00, 01, or 10 are the leading 2 bits of the exp
// and the trailing 3 bits are to be concatenated onto the significand (23 bits total)
//
//    Comb.  Exponent          Significand
// s 00 TTT (00)eeeeeeee (0TTT)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
// s 01 TTT (01)eeeeeeee (0TTT)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
// s 10 TTT (10)eeeeeeee (0TTT)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
static constexpr std::uint64_t d64_sign_mask = UINT64_C(0b1'00000'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
static constexpr std::uint64_t d64_combination_field_mask = UINT64_C(0b0'11111'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
static constexpr std::uint64_t d64_exponent_mask = UINT64_C(0b0'00000'11111111'0000000000'0000000000'0000000000'0000000000'0000000000);
static constexpr std::uint64_t d64_significand_mask = UINT64_C(0b0'00000'00000000'1111111111'1111111111'1111111111'1111111111'1111111111);
static constexpr std::uint64_t d64_significand_bits = UINT64_C(50);
static constexpr std::uint64_t d64_exponent_bits = UINT64_C(8);

static constexpr std::uint64_t d64_comb_01_mask = UINT64_C(0b0'01000'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
static constexpr std::uint64_t d64_comb_10_mask = UINT64_C(0b0'10000'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
static constexpr std::uint64_t d64_comb_00_01_10_significand_bits = UINT64_C(0b0'00111'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);

// This mask is used to determine if we use the masks above or below since 11 TTT is invalid
static constexpr std::uint64_t d64_comb_11_mask = UINT64_C(0b0'11000'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
static constexpr std::uint64_t d64_comb_11_exp_bits = UINT64_C(0b0'00110'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
static constexpr std::uint64_t d64_comb_11_significand_bits = UINT64_C(0b0'00001'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);

// For these masks the first two bits of the combination field imply 100 T as the
// leading bits of the significand and then bits 3 and 4 are the exp
//
//    Comb.  Exponent          Significand
// s 1100 T (00)eeeeeeee (100T)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
// s 1101 T (01)eeeeeeee (100T)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
// s 1110 T (10)eeeeeeee (100T)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
static constexpr std::uint64_t d64_comb_1101_mask = UINT64_C(0b0'11010'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
static constexpr std::uint64_t d64_comb_1110_mask = UINT64_C(0b0'11100'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);

// Powers of 2 used to determine the size of the significand
static constexpr std::uint64_t d64_no_combination = 0b1111111111'1111111111'1111111111'1111111111'1111111111;
static constexpr std::uint64_t d64_big_combination = 0b111'1111111111'1111111111'1111111111'1111111111'1111111111;

// Exponent fields
static constexpr std::uint64_t d64_max_exp_no_combination = 0b11111111;
static constexpr std::uint64_t d64_exp_one_combination = 0b1'11111111;
static constexpr std::uint64_t d64_max_biased_exp = 0b10'11111111;
static constexpr std::uint64_t d64_small_combination_field_mask = 0b111'0000000000'0000000000'0000000000'0000000000'0000000000;
static constexpr std::uint64_t d64_big_combination_field_mask = 0b1'0000000000'0000000000'0000000000'0000000000'0000000000;

// Constexpr construction form an uint64_t without having to memcpy
static constexpr std::uint64_t d64_construct_sign_mask = 0b1'00000'00000000'0000000000'0000000000'0000000000'0000000000'0000000000;
static constexpr std::uint64_t d64_construct_combination_mask = 0b0'11111'00000000'0000000000'0000000000'0000000000'0000000000'0000000000;
static constexpr std::uint64_t d64_construct_exp_mask = 0b0'00000'11111111'0000000000'0000000000'0000000000'0000000000'0000000000;
static constexpr std::uint64_t d64_construct_significand_mask = d64_no_combination;

struct decimal64_components
{
    std::uint64_t sig;
    std::int32_t exp;
    bool sign;
};

} //namespace detail

#if defined(__GNUC__) && __GNUC__ >= 8
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif

class decimal64 final
{
private:

    std::uint64_t bits_ {};

    // Returns the un-biased (quantum) exponent
    constexpr auto unbiased_exponent() const noexcept -> std::uint64_t;

    // Returns the biased exponent
    constexpr auto biased_exponent() const noexcept -> std::int32_t;

    // Allows direct editing of the exp
    template <typename T, std::enable_if_t<detail::is_integral_v<T>, bool> = true>
    constexpr auto edit_exponent(T exp) noexcept -> void;

    // Returns the significand complete with the bits implied from the combination field
    constexpr auto full_significand() const noexcept -> std::uint64_t;
    constexpr auto isneg() const noexcept -> bool;
    constexpr auto edit_sign(bool sign) noexcept -> void;

    // Attempts conversion to integral type:
    // If this is nan sets errno to EINVAL and returns 0
    // If this is not representable sets errno to ERANGE and returns 0
    template <typename Decimal, typename TargetType>
    friend constexpr auto to_integral(Decimal val) noexcept -> TargetType;

    template <typename Decimal, typename TargetType>
    friend BOOST_DECIMAL_CXX20_CONSTEXPR auto to_float(Decimal val) noexcept -> TargetType;

    template <typename TargetType, typename Decimal>
    friend constexpr auto to_decimal(Decimal val) noexcept -> TargetType;

    // Debug bit pattern
    friend constexpr auto from_bits(std::uint64_t bits) noexcept -> decimal64;
    friend constexpr auto to_bits(decimal64 rhs) noexcept -> std::uint64_t;

    // Equality template between any integer type and decimal64
    template <typename Decimal, typename Integer>
    friend constexpr auto mixed_equality_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    template <typename Decimal1, typename Decimal2>
    friend constexpr auto mixed_decimal_equality_impl(Decimal1 lhs, Decimal2 rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                             detail::is_decimal_floating_point_v<Decimal2>), bool>;

    // Template to compare operator< for any integer type and decimal64
    template <typename Decimal, typename Integer>
    friend constexpr auto less_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    template <typename Decimal1, typename Decimal2>
    friend constexpr auto mixed_decimal_less_impl(Decimal1 lhs, Decimal2 rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                             detail::is_decimal_floating_point_v<Decimal2>), bool>;

    template <typename T1, typename T2>
    friend constexpr auto d64_add_impl(T1 lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                                       T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept
                                       -> detail::decimal64_components;

    template <typename T1, typename T2>
    friend constexpr auto d64_sub_impl(T1 lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                                       T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign,
                                       bool abs_lhs_bigger) noexcept -> detail::decimal64_components;

    template <typename T1, typename T2>
    friend constexpr auto d64_mul_impl(T1 lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                                       T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> detail::decimal64_components;

    friend constexpr auto d64_generic_div_impl(detail::decimal64_components lhs, detail::decimal64_components rhs,
                                               detail::decimal64_components& q) noexcept -> void;

    friend constexpr auto d64_div_impl(decimal64 lhs, decimal64 rhs, decimal64& q, decimal64& r) noexcept -> void;

    friend constexpr auto d64_mod_impl(decimal64 lhs, decimal64 rhs, const decimal64& q, decimal64& r) noexcept -> void;

    template <typename T>
    friend constexpr auto ilogb(T d) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, int>;

public:
    // 3.2.3.1 construct/copy/destroy
    constexpr decimal64() noexcept = default;

    // 3.2.2.2 Conversion form floating-point type
    template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool> = true>
    explicit BOOST_DECIMAL_CXX20_CONSTEXPR decimal64(Float val) noexcept;

    template <typename Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool> = true>
    constexpr decimal64(Decimal val) noexcept;

    // 3.2.3.3 Conversion from integral type
    template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool> = true>
    explicit constexpr decimal64(Integer val) noexcept;

    // 3.2.3.4 Conversion to integral type
    explicit constexpr operator int() const noexcept;
    explicit constexpr operator unsigned() const noexcept;
    explicit constexpr operator long() const noexcept;
    explicit constexpr operator unsigned long() const noexcept;
    explicit constexpr operator long long() const noexcept;
    explicit constexpr operator unsigned long long() const noexcept;
    explicit constexpr operator std::int8_t() const noexcept;
    explicit constexpr operator std::uint8_t() const noexcept;
    explicit constexpr operator std::int16_t() const noexcept;
    explicit constexpr operator std::uint16_t() const noexcept;

    template <typename Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool> = true>
    explicit constexpr operator Decimal() const noexcept;

    // 3.2.6 Conversion to floating-point type
    explicit BOOST_DECIMAL_CXX20_CONSTEXPR operator float() const noexcept;
    explicit BOOST_DECIMAL_CXX20_CONSTEXPR operator double() const noexcept;
    explicit BOOST_DECIMAL_CXX20_CONSTEXPR operator long double() const noexcept;

    #ifdef BOOST_DECIMAL_HAS_FLOAT16
    explicit constexpr operator std::float16_t() const noexcept;
    #endif
    #ifdef BOOST_DECIMAL_HAS_FLOAT32
    explicit constexpr operator std::float32_t() const noexcept;
    #endif
    #ifdef BOOST_DECIMAL_HAS_FLOAT64
    explicit constexpr operator std::float64_t() const noexcept;
    #endif
    #ifdef BOOST_DECIMAL_HAS_BRAINFLOAT16
    explicit constexpr operator std::bfloat16_t() const noexcept;
    #endif

    // 3.2.5 initialization from coefficient and exponent:
    template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1>, bool> = true>
    constexpr decimal64(T1 coeff, T2 exp, bool sign = false) noexcept;

    // cmath functions that are easier as friends
    friend constexpr auto signbit     BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64 rhs) noexcept -> bool;
    friend constexpr auto isnan       BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64 rhs) noexcept -> bool;
    friend constexpr auto isinf       BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64 rhs) noexcept -> bool;
    friend constexpr auto issignaling BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64 rhs) noexcept -> bool;
    friend constexpr auto isnormal    BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64 rhs) noexcept -> bool;

    // 3.2.7 unary arithmetic operators:
    friend constexpr auto operator+(decimal64 rhs) noexcept -> decimal64;
    friend constexpr auto operator-(decimal64 rhs) noexcept -> decimal64;

    // 3.2.8 Binary arithmetic operators
    friend constexpr auto operator+(decimal64 lhs, decimal64 rhs) noexcept -> decimal64;

    template <typename Integer>
    friend constexpr auto operator+(decimal64 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>;

    template <typename Integer>
    friend constexpr auto operator+(Integer lhs, decimal64 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>;

    friend constexpr auto operator-(decimal64 lhs, decimal64 rhs) noexcept -> decimal64;

    template <typename Integer>
    friend constexpr auto operator-(decimal64 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>;

    template <typename Integer>
    friend constexpr auto operator-(Integer lhs, decimal64 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>;

    friend constexpr auto operator*(decimal64 lhs, decimal64 rhs) noexcept -> decimal64;

    template <typename Integer>
    friend constexpr auto operator*(decimal64 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>;

    template <typename Integer>
    friend constexpr auto operator*(Integer lhs, decimal64 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>;

    friend constexpr auto operator/(decimal64 lhs, decimal64 rhs) noexcept -> decimal64;

    template <typename Integer>
    friend constexpr auto operator/(decimal64 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>;

    template <typename Integer>
    friend constexpr auto operator/(Integer lhs, decimal64 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>;

    friend constexpr auto operator%(decimal64 lhs, decimal64 rhs) noexcept -> decimal64;

    // 3.2.3.5 Increment and Decrement
    constexpr auto operator++()    noexcept -> decimal64&;
    constexpr auto operator++(int) noexcept -> decimal64;  // NOLINT : C++14 so constexpr implies const
    constexpr auto operator--()    noexcept -> decimal64&;
    constexpr auto operator--(int) noexcept -> decimal64;  // NOLINT : C++14 so constexpr implies const

    // 3.2.3.6 Compound assignment
    constexpr auto operator+=(decimal64 rhs) noexcept -> decimal64&;

    template <typename Integer>
    constexpr auto operator+=(Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64&>;

    template <typename Decimal>
    constexpr auto operator+=(Decimal rhs) noexcept
        -> std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, decimal64&>;

    constexpr auto operator-=(decimal64 rhs) noexcept -> decimal64&;

    template <typename Integer>
    constexpr auto operator-=(Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64&>;

    template <typename Decimal>
    constexpr auto operator-=(Decimal rhs) noexcept
        -> std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, decimal64&>;

    constexpr auto operator*=(decimal64 rhs) noexcept -> decimal64&;

    template <typename Integer>
    constexpr auto operator*=(Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64&>;

    template <typename Decimal>
    constexpr auto operator*=(Decimal rhs) noexcept
        -> std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, decimal64&>;

    constexpr auto operator/=(decimal64 rhs) noexcept -> decimal64&;

    template <typename Integer>
    constexpr auto operator/=(Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64&>;

    template <typename Decimal>
    constexpr auto operator/=(Decimal rhs) noexcept
        -> std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, decimal64&>;

    // 3.2.9 Comparison operators:
    // Equality
    friend constexpr auto operator==(decimal64 lhs, decimal64 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator==(decimal64 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator==(Integer lhs, decimal64 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    // Inequality
    friend constexpr auto operator!=(decimal64 lhs, decimal64 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator!=(decimal64 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator!=(Integer lhs, decimal64 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    // Less
    friend constexpr auto operator<(decimal64 lhs, decimal64 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator<(decimal64 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator<(Integer lhs, decimal64 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    // Less equal
    friend constexpr auto operator<=(decimal64 lhs, decimal64 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator<=(decimal64 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator<=(Integer lhs, decimal64 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    // Greater
    friend constexpr auto operator>(decimal64 lhs, decimal64 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator>(decimal64 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator>(Integer lhs, decimal64 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    // Greater equal
    friend constexpr auto operator>=(decimal64 lhs, decimal64 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator>=(decimal64 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator>=(Integer lhs, decimal64 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    // C++20 spaceship
    #ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR
    friend constexpr auto operator<=>(decimal64 lhs, decimal64 rhs) noexcept -> std::partial_ordering;

    template <typename Integer>
    friend constexpr auto operator<=>(decimal64 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, std::partial_ordering>;

    template <typename Integer>
    friend constexpr auto operator<=>(Integer lhs, decimal64 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, std::partial_ordering>;
    #endif

    // 3.2.10 Formatted input:
    template <typename charT, typename traits, typename DecimalType>
    friend auto operator>>(std::basic_istream<charT, traits>& is, DecimalType& d)
        -> std::enable_if_t<detail::is_decimal_floating_point_v<DecimalType>, std::basic_istream<charT, traits>&>;

    // 3.2.11 Formatted output:
    template <typename charT, typename traits, typename DecimalType>
    friend auto operator<<(std::basic_ostream<charT, traits>& os, const DecimalType& d)
        -> std::enable_if_t<detail::is_decimal_floating_point_v<DecimalType>, std::basic_ostream<charT, traits>&>;

    // 3.6.4 Same Quantum
    friend constexpr auto samequantumd64(decimal64 lhs, decimal64 rhs) noexcept -> bool;

    // 3.6.5 Quantum exponent
    friend constexpr auto quantexpd64(decimal64 x) noexcept -> int;

    // 3.6.6 Quantize
    friend constexpr auto quantized64(decimal64 lhs, decimal64 rhs) noexcept -> decimal64;

    // Bit-wise operators
    friend constexpr auto operator&(decimal64 lhs, decimal64 rhs) noexcept -> decimal64;

    template <typename Integer>
    friend constexpr auto operator&(decimal64 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>;

    template <typename Integer>
    friend constexpr auto operator&(Integer lhs, decimal64 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>;

    friend constexpr auto operator|(decimal64 lhs, decimal64 rhs) noexcept -> decimal64;

    template <typename Integer>
    friend constexpr auto operator|(decimal64 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>;

    template <typename Integer>
    friend constexpr auto operator|(Integer lhs, decimal64 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>;

    friend constexpr auto operator^(decimal64 lhs, decimal64 rhs) noexcept -> decimal64;

    template <typename Integer>
    friend constexpr auto operator^(decimal64 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>;

    template <typename Integer>
    friend constexpr auto operator^(Integer lhs, decimal64 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>;

    friend constexpr auto operator<<(decimal64 lhs, decimal64 rhs) noexcept -> decimal64;

    template <typename Integer>
    friend constexpr auto operator<<(decimal64 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>;

    template <typename Integer>
    friend constexpr auto operator<<(Integer lhs, decimal64 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>;

    friend constexpr auto operator>>(decimal64 lhs, decimal64 rhs) noexcept -> decimal64;

    template <typename Integer>
    friend constexpr auto operator>>(decimal64 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>;

    template <typename Integer>
    friend constexpr auto operator>>(Integer lhs, decimal64 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>;

    friend constexpr auto operator~(decimal64 lhs) noexcept -> decimal64;

    // <cmath> functions that need to be friends
    template <typename T>
    friend constexpr auto frexp10(T num, int* expptr) noexcept
    -> std::enable_if_t<detail::is_decimal_floating_point_v<T>,
            std::conditional_t<std::is_same<T, decimal32>::value, std::uint32_t, std::uint64_t>>;

    friend constexpr auto copysignd64(decimal64 mag, decimal64 sgn) noexcept -> decimal64;
    friend constexpr auto fmad64(decimal64 x, decimal64 y, decimal64 z) noexcept -> decimal64;
    friend constexpr auto scalbnd64(decimal64 num, int exp) noexcept -> decimal64;
    friend constexpr auto scalblnd64(decimal64 num, long exp) noexcept -> decimal64;
};

#if defined(__GNUC__) && __GNUC__ >= 8
#  pragma GCC diagnostic pop
#endif

constexpr auto from_bits(std::uint64_t bits) noexcept -> decimal64
{
    decimal64 result;
    result.bits_ = bits;

    return result;
}

constexpr auto to_bits(decimal64 rhs) noexcept -> std::uint64_t
{
    return rhs.bits_;
}

// 3.2.5 initialization from coefficient and exponent:
template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1>, bool>>
constexpr decimal64::decimal64(T1 coeff, T2 exp, bool sign) noexcept
{
    using Unsigned_Integer = detail::make_unsigned_t<T1>;

    bits_ = UINT64_C(0);
    bool isneg {false};
    Unsigned_Integer unsigned_coeff {detail::make_positive_unsigned(coeff)};
    BOOST_DECIMAL_IF_CONSTEXPR (detail::is_signed_v<T1>)
    {
        if (coeff < 0 || sign)
        {
            bits_ |= detail::d64_sign_mask;
            isneg = true;
        }
    }
    else
    {
        if (sign)
        {
            bits_ |= detail::d64_sign_mask;
            isneg = true;
        }
    }

    // If the coeff is not in range make it so
    auto unsigned_coeff_digits {detail::num_digits(unsigned_coeff)};
    const bool reduced {unsigned_coeff_digits > detail::precision_v<decimal64>};
    while (unsigned_coeff_digits > detail::precision_v<decimal64> + 1)
    {
        unsigned_coeff /= 10;
        ++exp;
        --unsigned_coeff_digits;
    }

    // Round as required
    if (reduced)
    {
        exp += detail::fenv_round<decimal64>(unsigned_coeff, isneg);
    }

    auto reduced_coeff {static_cast<std::uint64_t>(unsigned_coeff)};
    bool big_combination {false};

    if (reduced_coeff == 0)
    {
        exp = 0;
    }
    else if (reduced_coeff <= detail::d64_no_combination)
    {
        // If the coefficient fits directly we don't need to use the combination field
        bits_ |= (reduced_coeff & detail::d64_significand_mask);
    }
    else if (reduced_coeff <= detail::d64_big_combination)
    {
        // Break the number into 3 bits for the combination field and 50 bits for the significand field

        // Use the least significant 50 bits to set the significand
        bits_ |= (reduced_coeff & detail::d64_significand_mask);

        // Now set the combination field (maximum of 3 bits)
        std::uint64_t remaining_bits {reduced_coeff & detail::d64_small_combination_field_mask};
        remaining_bits <<= detail::d64_exponent_bits;
        bits_ |= remaining_bits;
    }
    else
    {
        // Have to use the full combination field
        bits_ |= detail::d64_comb_11_mask;
        big_combination = true;

        bits_ |= (reduced_coeff & detail::d64_significand_mask);
        const auto remaining_bit {reduced_coeff & detail::d64_big_combination_field_mask};

        if (remaining_bit)
        {
            bits_ |= detail::d64_comb_11_significand_bits;
        }
    }

    // If the exponent fits we do not need to use the combination field
    auto biased_exp {static_cast<std::uint64_t>(exp + detail::bias_v<decimal64>)};
    const std::uint64_t biased_exp_low_eight_bits {(biased_exp & detail::d64_max_exp_no_combination) << detail::d64_significand_bits};

    if (biased_exp <= detail::d64_max_exp_no_combination)
    {
        bits_ |= biased_exp_low_eight_bits;
    }
    else if (biased_exp <= detail::d64_exp_one_combination)
    {
        if (big_combination)
        {
            bits_ |= (detail::d64_comb_1101_mask | biased_exp_low_eight_bits);
        }
        else
        {
            bits_ |= (detail::d64_comb_01_mask | biased_exp_low_eight_bits);
        }
    }
    else if (biased_exp <= detail::d64_max_biased_exp)
    {
        if (big_combination)
        {
            bits_ |= (detail::d64_comb_1110_mask | biased_exp_low_eight_bits);
        }
        else
        {
            bits_ |= (detail::d64_comb_10_mask | biased_exp_low_eight_bits);
        }
    }
    else
    {
        // The value is probably infinity

        // If we can offset some extra power in the coefficient try to do so
        const auto coeff_dig {detail::num_digits(reduced_coeff)};
        if (coeff_dig < detail::precision_v<decimal64>)
        {
            for (auto i {coeff_dig}; i <= detail::precision_v<decimal64>; ++i)
            {
                reduced_coeff *= 10;
                --biased_exp;
                --exp;
                if (biased_exp == detail::d64_max_biased_exp)
                {
                    break;
                }
            }

            if (detail::num_digits(reduced_coeff) <= detail::precision_v<decimal64>)
            {
                *this = decimal64(reduced_coeff, exp, isneg);
            }
            else
            {
                if (exp < 0)
                {
                    *this = decimal64(0, 0, isneg);
                }
                else
                {
                    bits_ = detail::d64_comb_inf_mask;
                }
            }
        }
        else
        {
            bits_ = detail::d64_comb_inf_mask;
        }
    }
}

template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool>>
BOOST_DECIMAL_CXX20_CONSTEXPR decimal64::decimal64(Float val) noexcept
{
    if (val != val)
    {
        *this = from_bits(detail::d64_nan_mask);
    }
    else if (val == std::numeric_limits<Float>::infinity() || val == -std::numeric_limits<Float>::infinity())
    {
        *this = from_bits(detail::d64_inf_mask);
    }
    else
    {
        const auto components {detail::ryu::floating_point_to_fd128(val)};

        #ifdef BOOST_DECIMAL_DEBUG
        std::cerr << "Mant: " << components.mantissa
                  << "\nExp: " << components.exponent
                  << "\nSign: " << components.sign << std::endl;
        #endif

        if (components.exponent > detail::emax_v<decimal64>)
        {
            *this = from_bits(detail::d64_inf_mask);
        }
        else
        {
            *this = decimal64 {components.mantissa, components.exponent, components.sign};
        }
    }
}

template <typename Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool>>
constexpr decimal64::decimal64(Decimal val) noexcept
{
    *this = to_decimal<decimal64>(val);
}

template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool>>
constexpr decimal64::decimal64(Integer val) noexcept // NOLINT : Incorrect parameter is never used
{
    *this = decimal64{val, 0};
}

constexpr decimal64::operator int() const noexcept
{
    return to_integral<decimal64, int>(*this);
}

constexpr decimal64::operator unsigned() const noexcept
{
    return to_integral<decimal64, unsigned>(*this);
}

constexpr decimal64::operator long() const noexcept
{
    return to_integral<decimal64, long>(*this);
}

constexpr decimal64::operator unsigned long() const noexcept
{
    return to_integral<decimal64, unsigned long>(*this);
}

constexpr decimal64::operator long long() const noexcept
{
    return to_integral<decimal64, long long>(*this);
}

constexpr decimal64::operator unsigned long long() const noexcept
{
    return to_integral<decimal64, unsigned long long>(*this);
}

constexpr decimal64::operator std::int8_t() const noexcept
{
    return to_integral<decimal64, std::int8_t>(*this);
}

constexpr decimal64::operator std::uint8_t() const noexcept
{
    return to_integral<decimal64, std::uint8_t>(*this);
}

constexpr decimal64::operator std::int16_t() const noexcept
{
    return to_integral<decimal64, std::int16_t>(*this);
}

constexpr decimal64::operator std::uint16_t() const noexcept
{
    return to_integral<decimal64, std::uint16_t>(*this);
}

template <typename Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool>>
constexpr decimal64::operator Decimal() const noexcept
{
    return to_decimal<Decimal>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal64::operator float() const noexcept
{
    return to_float<decimal64, float>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal64::operator double() const noexcept
{
    return to_float<decimal64, double>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal64::operator long double() const noexcept
{
    // TODO(mborland): Don't have an exact way of converting to various long doubles
    return static_cast<long double>(to_float<decimal64, double>(*this));
}

#ifdef BOOST_DECIMAL_HAS_FLOAT16
constexpr decimal64::operator std::float16_t() const noexcept
{
    return static_cast<std::float16_t>(to_float<decimal64, float>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_FLOAT32
constexpr decimal64::operator std::float32_t() const noexcept
{
    return static_cast<std::float32_t>(to_float<decimal64, float>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_FLOAT64
constexpr decimal64::operator std::float64_t() const noexcept
{
    return static_cast<std::float64_t>(to_float<decimal64, double>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_BRAINFLOAT16
constexpr decimal64::operator std::bfloat16_t() const noexcept
{
    return static_cast<std::bfloat16_t>(to_float<decimal64, float>(*this));
}
#endif

constexpr auto decimal64::unbiased_exponent() const noexcept -> std::uint64_t
{
    std::uint64_t expval {};

    const auto exp_comb_bits {(bits_ & detail::d64_comb_11_mask)};

    if (exp_comb_bits == detail::d64_comb_11_mask)
    {
        // bits 2 and 3 are the exp part of the combination field
        expval = (bits_ & detail::d64_comb_11_exp_bits) >> (detail::d64_significand_bits + 1);
    }
    else if (exp_comb_bits == detail::d64_comb_10_mask)
    {
        expval = UINT64_C(0b1000000000);
    }
    else if (exp_comb_bits == detail::d64_comb_01_mask)
    {
        expval = UINT64_C(0b0100000000);
    }

    expval |= (bits_ & detail::d64_exponent_mask) >> detail::d64_significand_bits;

    return expval;
}

constexpr auto decimal64::biased_exponent() const noexcept -> std::int32_t
{
    return static_cast<std::int32_t>(unbiased_exponent()) - detail::bias_v<decimal64>;
}

constexpr auto decimal64::full_significand() const noexcept -> std::uint64_t
{
    std::uint64_t significand {};

    if ((bits_ & detail::d64_comb_11_mask) == detail::d64_comb_11_mask)
    {
        // Only need the one bit of T because the other 3 are implied
        if ((bits_ & detail::d64_comb_11_significand_bits) == detail::d64_comb_11_significand_bits)
        {
            significand = UINT64_C(0b1001'0000000000'0000000000'0000000000'0000000000'0000000000);
        }
        else
        {
            significand = UINT64_C(0b1000'0000000000'0000000000'0000000000'0000000000'0000000000);
        }
    }
    else
    {
        // Last three bits in the combination field, so we need to shift past the exp field
        // which is next
        significand |= (bits_ & detail::d64_comb_00_01_10_significand_bits) >> detail::d64_exponent_bits;
    }

    significand |= (bits_ & detail::d64_significand_mask);

    return significand;
}

constexpr auto decimal64::isneg() const noexcept -> bool
{
    return static_cast<bool>(bits_ & detail::d64_sign_mask);
}

template <typename T, std::enable_if_t<detail::is_integral_v<T>, bool>>
constexpr auto decimal64::edit_exponent(T expval) noexcept -> void
{
    *this = decimal64(this->full_significand(), expval, this->isneg());
}

constexpr auto decimal64::edit_sign(bool sign) noexcept -> void
{
    if (sign)
    {
        bits_ |= detail::d64_sign_mask;
    }
    else
    {
        bits_ &= ~detail::d64_sign_mask;
    }
}

constexpr auto signbit BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64 rhs) noexcept -> bool
{
    return rhs.bits_ & detail::d64_sign_mask;
}

constexpr auto isnan BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64 rhs) noexcept -> bool
{
    return (rhs.bits_ & detail::d64_nan_mask) == detail::d64_nan_mask;
}

constexpr auto isinf BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64 rhs) noexcept -> bool
{
    return ((rhs.bits_ & detail::d64_nan_mask) == detail::d64_inf_mask);
}

constexpr auto issignaling BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64 rhs) noexcept -> bool
{
    return (rhs.bits_ & detail::d64_snan_mask) == detail::d64_snan_mask;
}

constexpr auto isnormal BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64 rhs) noexcept -> bool
{
    // Check for de-normals
    const auto sig {rhs.full_significand()};
    const auto exp {rhs.unbiased_exponent()};

    if (exp <= detail::precision_v<decimal64> - 1)
    {
        return false;
    }

    return (sig != 0) && isfinite(rhs);
}

constexpr auto operator+(decimal64 rhs) noexcept -> decimal64
{
    return rhs;
}

constexpr auto operator-(decimal64 rhs) noexcept-> decimal64
{
    rhs.bits_ ^= detail::d64_sign_mask;
    return rhs;
}

template<typename T1, typename T2>
constexpr auto d64_add_impl(T1 lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                            T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> detail::decimal64_components
{
    const bool sign {lhs_sign};

    auto delta_exp {lhs_exp > rhs_exp ? lhs_exp - rhs_exp : rhs_exp - lhs_exp};

    if (delta_exp > detail::precision_v<decimal64> + 1)
    {
        // If the difference in exponents is more than the digits of accuracy
        // we return the larger of the two
        //
        // e.g. 1e20 + 1e-20 = 1e20

        return {lhs_sig, lhs_exp, lhs_sign};
    }
    else if (delta_exp == detail::precision_v<decimal64> + 1)
    {
        // Only need to see if we need to add one to the
        // significand of the bigger value
        //
        // e.g. 1.234567e5 + 9.876543e-2 = 1.234568e5

        if (rhs_sig >= UINT64_C(5'000'000'000'000'000))
        {
            ++lhs_sig;
            return {lhs_sig, lhs_exp, lhs_sign};
        }
        else
        {
            return {lhs_sig, lhs_exp, lhs_sign};
        }
    }

    // The two numbers can be added together without special handling
    //
    // If we can add to the lhs sig rather than dividing we can save some precision
    // 64-bit sign int can have 19 digits, and our normalized significand has 16
    if (delta_exp <= 3)
    {
        while (delta_exp > 0)
        {
            lhs_sig *= 10;
            --delta_exp;
            --lhs_exp;
        }
    }
    else
    {
        lhs_sig *= 1000;
        delta_exp -= 3;
        lhs_exp -= 3;
    }

    while (delta_exp > 1)
    {
        rhs_sig /= 10;
        --delta_exp;
    }

    if (delta_exp == 1)
    {
        detail::fenv_round<decimal64>(rhs_sig, rhs_sign);
    }

    // Both of the significands are well under 64-bits, so we can fit them into int64_t without issue
    const auto new_sig {static_cast<std::uint64_t>(lhs_sig) + static_cast<std::uint64_t>(rhs_sig)};
    const auto new_exp {lhs_exp};
    const auto res_sig {detail::make_positive_unsigned(new_sig)};

    #ifdef BOOST_DECIMAL_DEBUG_ADD
    std::cerr << "Res Sig: " << new_sig
              << "\nRes Exp: " << new_exp
              << "\nRes Neg: " << sign << std::endl;
    #endif

    return {res_sig, new_exp, sign};
}

template <typename T1, typename T2>
constexpr auto d64_sub_impl(T1 lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                            T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign,
                            bool abs_lhs_bigger) noexcept -> detail::decimal64_components
{
    auto delta_exp {lhs_exp > rhs_exp ? lhs_exp - rhs_exp : rhs_exp - lhs_exp};
    auto signed_sig_lhs {detail::make_signed_value(lhs_sig, lhs_sign)};
    auto signed_sig_rhs {detail::make_signed_value(rhs_sig, rhs_sign)};

    if (delta_exp > detail::precision_v<decimal64> + 1)
    {
        // If the difference in exponents is more than the digits of accuracy
        // we return the larger of the two
        //
        // e.g. 1e20 - 1e-20 = 1e20
        return abs_lhs_bigger ? detail::decimal64_components{detail::shrink_significand<std::uint64_t>(lhs_sig, lhs_exp), lhs_exp, false} :
                                detail::decimal64_components{detail::shrink_significand<std::uint64_t>(rhs_sig, rhs_exp), rhs_exp, true};
    }

    // The two numbers can be subtracted together without special handling

    auto& sig_bigger {abs_lhs_bigger ? signed_sig_lhs : signed_sig_rhs};
    auto& exp_bigger {abs_lhs_bigger ? lhs_exp : rhs_exp};
    auto& sig_smaller {abs_lhs_bigger ? signed_sig_rhs : signed_sig_lhs};
    auto& smaller_sign {abs_lhs_bigger ? rhs_sign : lhs_sign};

    if (delta_exp == 1)
    {
        sig_bigger *= 10;
        --delta_exp;
        --exp_bigger;
    }
    else if (delta_exp >= 2)
    {
        sig_bigger *= 100;
        delta_exp -= 2;
        exp_bigger -= 2;
    }

    while (delta_exp > 1)
    {
        sig_smaller /= 10;
        --delta_exp;
    }

    if (delta_exp == 1)
    {
        detail::fenv_round<decimal64>(sig_smaller, smaller_sign);
    }

    // Both of the significands are less than 9'999'999'999'999'999, so we can safely
    // cast them to signed 64-bit ints to calculate the new significand
    std::int64_t new_sig {}; // NOLINT : Value is never used but can't leave uninitialized in constexpr function

    if (rhs_sign && !lhs_sign)
    {
        new_sig = signed_sig_lhs + signed_sig_rhs;
    }
    else
    {
        new_sig = signed_sig_lhs - signed_sig_rhs;
    }

    const auto new_exp {abs_lhs_bigger ? lhs_exp : rhs_exp};
    const auto new_sign {new_sig < 0};
    const auto res_sig {detail::make_positive_unsigned(new_sig)};

    return {res_sig, new_exp, new_sign};
}

template <typename T1, typename T2>
constexpr auto d64_mul_impl(T1 lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                            T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> detail::decimal64_components
{
    #ifdef BOOST_DECIMAL_HAS_INT128
    using unsigned_int128_type = boost::decimal::detail::uint128_t;
    #else
    using unsigned_int128_type = boost::decimal::detail::uint128;
    #endif

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "sig lhs: " << sig_lhs
              << "\nexp lhs: " << exp_lhs
              << "\nsig rhs: " << sig_rhs
              << "\nexp rhs: " << exp_rhs;
    #endif

    bool sign {lhs_sign != rhs_sign};

    // Once we have the normalized significands and exponents all we have to do is
    // multiply the significands and add the exponents

    auto res_sig {static_cast<unsigned_int128_type>(lhs_sig) * static_cast<unsigned_int128_type>(rhs_sig)};
    auto res_exp {lhs_exp + rhs_exp};

    const auto sig_dig {detail::num_digits(res_sig)};

    if (sig_dig > std::numeric_limits<std::uint64_t>::digits10)
    {
        res_sig /= detail::powers_of_10[sig_dig - std::numeric_limits<std::uint64_t>::digits10];
        res_exp += sig_dig - std::numeric_limits<std::uint64_t>::digits10;
    }

    const auto res_sig_64 {static_cast<std::uint64_t>(res_sig)};

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "\nres sig: " << res_sig_64
              << "\nres exp: " << res_exp << std::endl;
    #endif

    // Always return positive zero
    if (res_sig_64 == 0)
    {
        sign = false;
    }

    return {res_sig_64, res_exp, sign};
}

constexpr auto d64_generic_div_impl(detail::decimal64_components lhs, detail::decimal64_components rhs,
                                    detail::decimal64_components& q) noexcept -> void
{
    #ifdef BOOST_DECIMAL_HAS_INT128
    using unsigned_int128_type = boost::decimal::detail::uint128_t;
    #else
    using unsigned_int128_type = boost::decimal::detail::uint128;
    #endif

    bool sign {lhs.sign != rhs.sign};

    // If rhs is greater than we need to offset the significands to get the correct values
    // e.g. 4/8 is 0 but 40/8 yields 5 in integer maths
    const auto big_sig_lhs {static_cast<unsigned_int128_type>(lhs.sig) * detail::powers_of_10[detail::precision_v<decimal64>]};
    lhs.exp -= detail::precision_v<decimal64>;

    auto res_sig {big_sig_lhs / static_cast<unsigned_int128_type>(rhs.sig)};
    auto res_exp {lhs.exp - rhs.exp};

    const auto sig_dig {detail::num_digits(res_sig)};

    if (sig_dig > std::numeric_limits<std::uint64_t>::digits10)
    {
        res_sig /= detail::powers_of_10[sig_dig - std::numeric_limits<std::uint64_t>::digits10];
        res_exp += sig_dig - std::numeric_limits<std::uint64_t>::digits10;
    }

    const auto res_sig_64 {static_cast<std::uint64_t>(res_sig)};

    if (res_sig_64 == 0)
    {
        sign = false;
    }

    // Let the constructor handle shrinking it back down and rounding correctly
    q = detail::decimal64_components{res_sig_64, res_exp, sign};
}

constexpr auto d64_div_impl(decimal64 lhs, decimal64 rhs, decimal64& q, decimal64& r) noexcept -> void
{
    // Check pre-conditions
    constexpr decimal64 zero {0, 0};
    constexpr decimal64 nan {boost::decimal::from_bits(boost::decimal::detail::d64_snan_mask)};
    constexpr decimal64 inf {boost::decimal::from_bits(boost::decimal::detail::d64_inf_mask)};

    const bool sign {lhs.isneg() != rhs.isneg()};

    const auto lhs_fp {fpclassify(lhs)};
    const auto rhs_fp {fpclassify(rhs)};

    if (lhs_fp == FP_NAN || rhs_fp == FP_NAN)
    {
        q = nan;
        r = nan;
        return;
    }

    switch (lhs_fp)
    {
        case FP_INFINITE:
            q = sign ? -inf : inf;
            r = zero;
            return;
        case FP_ZERO:
            q = sign ? -zero : zero;
            r = sign ? -zero : zero;
            return;
        default:
            static_cast<void>(lhs);
    }

    switch (rhs_fp)
    {
        case FP_ZERO:
            q = inf;
            r = zero;
            return;
        case FP_INFINITE:
            q = sign ? -zero : zero;
            r = lhs;
            return;
        default:
            static_cast<void>(rhs);
    }

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize<decimal64>(sig_lhs, exp_lhs);

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize<decimal64>(sig_rhs, exp_rhs);

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "sig lhs: " << sig_lhs
              << "\nexp lhs: " << exp_lhs
              << "\nsig rhs: " << sig_rhs
              << "\nexp rhs: " << exp_rhs << std::endl;
    #endif

    detail::decimal64_components lhs_components {sig_lhs, exp_lhs, lhs.isneg()};
    detail::decimal64_components rhs_components {sig_rhs, exp_rhs, rhs.isneg()};
    detail::decimal64_components q_components {};

    d64_generic_div_impl(lhs_components, rhs_components, q_components);

    q = decimal64(q_components.sig, q_components.exp, q_components.sign);
}

constexpr auto d64_mod_impl(decimal64 lhs, decimal64 rhs, const decimal64& q, decimal64& r) noexcept -> void
{
    constexpr decimal64 zero {0, 0};

    // https://en.cppreference.com/w/cpp/numeric/math/fmod
    auto q_trunc {q > zero ? floor(q) : ceil(q)};
    r = lhs - (decimal64(q_trunc) * rhs);
}

constexpr auto operator+(decimal64 lhs, decimal64 rhs) noexcept -> decimal64
{
    constexpr decimal64 zero {0, 0};

    const auto res {detail::check_non_finite(lhs, rhs)};
    if (res != zero)
    {
        return res;
    }

    bool lhs_bigger {lhs > rhs};
    if (lhs.isneg() && rhs.isneg())
    {
        lhs_bigger = !lhs_bigger;
    }

    // Ensure that lhs is always the larger for ease of impl
    if (!lhs_bigger)
    {
        detail::swap(lhs, rhs);
    }

    if (!lhs.isneg() && rhs.isneg())
    {
        return lhs - abs(rhs);
    }

    auto lhs_sig {lhs.full_significand()};
    auto lhs_exp {lhs.biased_exponent()};
    detail::normalize<decimal64>(lhs_sig, lhs_exp);

    auto rhs_sig {rhs.full_significand()};
    auto rhs_exp {rhs.biased_exponent()};
    detail::normalize<decimal64>(rhs_sig, rhs_exp);

    const auto result {d64_add_impl(lhs_sig, lhs_exp, lhs.isneg(),
                                    rhs_sig, rhs_exp, rhs.isneg())};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator+(decimal64 lhs, Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>
{
    if (isnan(lhs) || isinf(lhs))
    {
        return lhs;
    }

    bool lhs_bigger {lhs > rhs};
    if (lhs.isneg() && (rhs < 0))
    {
        lhs_bigger = !lhs_bigger;
    }
    bool abs_lhs_bigger {abs(lhs) > detail::make_positive_unsigned(rhs)};

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize<decimal64>(sig_lhs, exp_lhs);
    auto lhs_components {detail::decimal64_components{sig_lhs, exp_lhs, lhs.isneg()}};

    auto sig_rhs {static_cast<std::uint64_t>(detail::make_positive_unsigned(rhs))};
    std::int32_t exp_rhs {0};
    detail::normalize<decimal64>(sig_rhs, exp_rhs);
    auto unsigned_sig_rhs = detail::shrink_significand<std::uint64_t>(detail::make_positive_unsigned(sig_rhs), exp_rhs);
    auto rhs_components {detail::decimal64_components{unsigned_sig_rhs, exp_rhs, (rhs < 0)}};

    if (!lhs_bigger)
    {
        detail::swap(lhs_components, rhs_components);
        lhs_bigger = !lhs_bigger;
        abs_lhs_bigger = !abs_lhs_bigger;
    }

    detail::decimal64_components result {};

    #ifdef BOOST_DECIMAL_DEBUG_ADD
    std::cerr << "Lhs sig: " << lhs_components.sig
              << "\nLhs exp: " << lhs_components.exp
              << "\nRhs sig: " << rhs_components.sig
              << "\nRhs exp: " << rhs_components.exp << std::endl;
    #endif

    if (!lhs_components.sign && rhs_components.sign)
    {
        result = d64_sub_impl(lhs_components.sig, lhs_components.exp, lhs_components.sign,
                              rhs_components.sig, rhs_components.exp, rhs_components.sign,
                              abs_lhs_bigger);
    }
    else
    {
        result = d64_add_impl(lhs_components.sig, lhs_components.exp, lhs_components.sign,
                              rhs_components.sig, rhs_components.exp, rhs_components.sign);
    }

    return decimal64(result.sig, result.exp, result.sign);
}

template <typename Integer>
constexpr auto operator+(Integer lhs, decimal64 rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>
{
    return rhs + lhs;
}

// NOLINTNEXTLINE : If subtraction is actually addition than use operator+ and vice versa
constexpr auto operator-(decimal64 lhs, decimal64 rhs) noexcept -> decimal64
{
    constexpr decimal64 zero {0, 0};

    const auto res {detail::check_non_finite(lhs, rhs)};
    if (res != zero)
    {
        return res;
    }

    if (!lhs.isneg() && rhs.isneg())
    {
        return lhs + (-rhs);
    }

    const bool abs_lhs_bigger {abs(lhs) > abs(rhs)};

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize<decimal64>(sig_lhs, exp_lhs);

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize<decimal64>(sig_rhs, exp_rhs);

    const auto result {d64_sub_impl(sig_lhs, exp_lhs, lhs.isneg(),
                                    sig_rhs, exp_rhs, rhs.isneg(),
                                    abs_lhs_bigger)};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator-(decimal64 lhs, Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>
{
    if (isinf(lhs) || isnan(lhs))
    {
        return lhs;
    }

    if (!lhs.isneg() && (rhs < 0))
    {
        return lhs + detail::make_positive_unsigned(rhs);
    }

    const bool abs_lhs_bigger {abs(lhs) > detail::make_positive_unsigned(rhs)};

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize<decimal64>(sig_lhs, exp_lhs);
    auto lhs_components {detail::decimal64_components{sig_lhs, exp_lhs, lhs.isneg()}};

    auto sig_rhs {static_cast<std::uint64_t>(detail::make_positive_unsigned(rhs))};
    std::int32_t exp_rhs {0};
    detail::normalize<decimal64>(sig_rhs, exp_rhs);
    auto unsigned_sig_rhs = detail::shrink_significand<std::uint64_t>(detail::make_positive_unsigned(sig_rhs), exp_rhs);
    auto rhs_components {detail::decimal64_components{unsigned_sig_rhs, exp_rhs, (rhs < 0)}};

    const auto result {d64_sub_impl(lhs_components.sig, lhs_components.exp, lhs_components.sign,
                                    rhs_components.sig, rhs_components.exp, rhs_components.sign,
                                    abs_lhs_bigger)};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator-(Integer lhs, decimal64 rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>
{
    if (isinf(rhs) || isnan(rhs))
    {
        return rhs;
    }

    if (lhs >= 0 && rhs.isneg())
    {
        return lhs + (-rhs);
    }

    const bool abs_lhs_bigger {detail::make_positive_unsigned(lhs) > abs(rhs)};

    auto sig_lhs {static_cast<std::uint64_t>(detail::make_positive_unsigned(lhs))};
    std::int32_t exp_lhs {0};
    detail::normalize<decimal64>(sig_lhs, exp_lhs);
    auto unsigned_sig_lhs = detail::shrink_significand<std::uint64_t>(detail::make_positive_unsigned(sig_lhs), exp_lhs);
    auto lhs_components {detail::decimal64_components{unsigned_sig_lhs, exp_lhs, (lhs < 0)}};

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize<decimal64>(sig_rhs, exp_rhs);
    auto rhs_components {detail::decimal64_components{sig_rhs, exp_rhs, rhs.isneg()}};

    const auto result {d64_sub_impl(lhs_components.sig, lhs_components.exp, lhs_components.sign,
                                    rhs_components.sig, rhs_components.exp, rhs_components.sign,
                                    abs_lhs_bigger)};

    return {result.sig, result.exp, result.sign};
}

constexpr auto operator*(decimal64 lhs, decimal64 rhs) noexcept -> decimal64
{
    constexpr decimal64 zero {0, 0};

    const auto non_finite {detail::check_non_finite(lhs, rhs)};
    if (non_finite != zero)
    {
        return non_finite;
    }

    auto lhs_sig {lhs.full_significand()};
    auto lhs_exp {lhs.biased_exponent()};
    detail::normalize<decimal64>(lhs_sig, lhs_exp);

    auto rhs_sig {rhs.full_significand()};
    auto rhs_exp {rhs.biased_exponent()};
    detail::normalize<decimal64>(rhs_sig, rhs_exp);

    const auto result {d64_mul_impl(lhs_sig, lhs_exp, lhs.isneg(),
                                    rhs_sig, rhs_exp, rhs.isneg())};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator*(decimal64 lhs, Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>
{
    if (isnan(lhs) || isinf(lhs))
    {
        return lhs;
    }

    auto lhs_sig {lhs.full_significand()};
    auto lhs_exp {lhs.biased_exponent()};
    detail::normalize<decimal64>(lhs_sig, lhs_exp);
    auto lhs_components {detail::decimal64_components{lhs_sig, lhs_exp, lhs.isneg()}};

    auto rhs_sig {static_cast<std::uint64_t>(detail::make_positive_unsigned(rhs))};
    std::int32_t rhs_exp {0};
    detail::normalize<decimal64>(rhs_sig, rhs_exp);
    auto unsigned_sig_rhs {detail::shrink_significand<std::uint64_t>(detail::make_positive_unsigned(rhs_sig), rhs_exp)};
    auto rhs_components {detail::decimal64_components{unsigned_sig_rhs, rhs_exp, (rhs < 0)}};

    const auto result {d64_mul_impl(lhs_components.sig, lhs_components.exp, lhs_components.sign,
                                    rhs_components.sig, rhs_components.exp, rhs_components.sign)};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator*(Integer lhs, decimal64 rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>
{
    return rhs * lhs;
}

constexpr auto operator/(decimal64 lhs, decimal64 rhs) noexcept -> decimal64
{
    decimal64 q {};
    decimal64 r {};
    d64_div_impl(lhs, rhs, q, r);

    return q;
}

template <typename Integer>
constexpr auto operator/(decimal64 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>
{
    // Check pre-conditions
    constexpr decimal64 zero {0, 0};
    constexpr decimal64 nan {boost::decimal::from_bits(boost::decimal::detail::d64_snan_mask)};
    constexpr decimal64 inf {boost::decimal::from_bits(boost::decimal::detail::d64_inf_mask)};

    const bool sign {lhs.isneg() != (rhs < 0)};

    const auto lhs_fp {fpclassify(lhs)};

    switch (lhs_fp)
    {
        case FP_NAN:
            return nan;
        case FP_INFINITE:
            return inf;
        case FP_ZERO:
            return sign ? -zero : zero;
        default:
            static_cast<void>(lhs);
    }

    if (rhs == 0)
    {
        return sign ? -inf : inf;
    }

    auto lhs_sig {lhs.full_significand()};
    auto lhs_exp {lhs.biased_exponent()};
    detail::normalize<decimal64>(lhs_sig, lhs_exp);

    detail::decimal64_components lhs_components {lhs_sig, lhs_exp, lhs.isneg()};

    auto rhs_sig {static_cast<std::uint64_t>(detail::make_positive_unsigned(rhs))};
    std::int32_t rhs_exp {};
    detail::decimal64_components rhs_components {detail::shrink_significand<std::uint64_t>(rhs_sig, rhs_exp), rhs_exp, rhs < 0};
    detail::decimal64_components q_components {};

    d64_generic_div_impl(lhs_components, rhs_components, q_components);

    return decimal64(q_components.sig, q_components.exp, q_components.sign);
}

template <typename Integer>
constexpr auto operator/(Integer lhs, decimal64 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>
{
    // Check pre-conditions
    constexpr decimal64 zero {0, 0};
    constexpr decimal64 inf {boost::decimal::from_bits(boost::decimal::detail::d64_inf_mask)};
    constexpr decimal64 nan {boost::decimal::from_bits(boost::decimal::detail::d64_snan_mask)};

    const bool sign {(lhs < 0) != rhs.isneg()};

    const auto rhs_fp {fpclassify(rhs)};

    if (rhs_fp == FP_NAN)
    {
        return nan;
    }

    switch (rhs_fp)
    {
        case FP_INFINITE:
            return sign ? -zero : zero;
        case FP_ZERO:
            return sign ? -inf : inf;
        default:
            static_cast<void>(lhs);
    }

    auto rhs_sig {rhs.full_significand()};
    auto rhs_exp {rhs.biased_exponent()};
    detail::normalize<decimal64>(rhs_sig, rhs_exp);

    detail::decimal64_components lhs_components {detail::make_positive_unsigned(lhs), 0, lhs < 0};
    detail::decimal64_components rhs_components {rhs_sig, rhs_exp, rhs.isneg()};
    detail::decimal64_components q_components {};

    d64_generic_div_impl(lhs_components, rhs_components, q_components);

    return decimal64(q_components.sig, q_components.exp, q_components.sign);
}

constexpr auto operator%(decimal64 lhs, decimal64 rhs) noexcept -> decimal64
{
    decimal64 q {};
    decimal64 r {};
    d64_div_impl(lhs, rhs, q, r);
    d64_mod_impl(lhs, rhs, q, r);

    return r;
}

constexpr auto decimal64::operator++() noexcept -> decimal64&
{
    constexpr decimal64 one{1, 0};
    *this = *this + one;
    return *this;
}

constexpr auto decimal64::operator++(int) noexcept -> decimal64
{
    return ++(*this);
}

constexpr auto decimal64::operator--() noexcept -> decimal64&
{
    constexpr decimal64 one{1, 0};
    *this = *this - one;
    return *this;
}

constexpr auto decimal64::operator--(int) noexcept -> decimal64
{
    return --(*this);
}

constexpr auto decimal64::operator+=(decimal64 rhs) noexcept -> decimal64&
{
    *this = *this + rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal64::operator+=(Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64&>
{
    *this = *this + rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal64::operator+=(Decimal rhs) noexcept
    -> std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, decimal64&>
{
    *this = *this + rhs;
    return *this;
}

constexpr auto decimal64::operator-=(decimal64 rhs) noexcept -> decimal64&
{
    *this = *this - rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal64::operator-=(Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64&>
{
    *this = *this - rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal64::operator-=(Decimal rhs) noexcept
    -> std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, decimal64&>
{
    *this = *this - rhs;
    return *this;
}

constexpr auto decimal64::operator*=(decimal64 rhs) noexcept -> decimal64&
{
    *this = *this * rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal64::operator*=(Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64&>
{
    *this = *this * rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal64::operator*=(Decimal rhs) noexcept
    -> std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, decimal64&>
{
    *this = *this * rhs;
    return *this;
}

constexpr auto decimal64::operator/=(decimal64 rhs) noexcept -> decimal64&
{
    *this = *this / rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal64::operator/=(Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64&>
{
    *this = *this / rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal64::operator/=(Decimal rhs) noexcept
    -> std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, decimal64&>
{
    *this = *this / rhs;
    return *this;
}

constexpr auto operator==(decimal64 lhs, decimal64 rhs) noexcept -> bool
{
    // Check for IEEE requirement that nan != nan
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }

    return equal_parts_impl<decimal64>(lhs.full_significand(), lhs.biased_exponent(), lhs.isneg(),
                                       rhs.full_significand(), rhs.biased_exponent(), rhs.isneg());
}

template <typename Integer>
constexpr auto operator==(decimal64 lhs, Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return mixed_equality_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator==(Integer lhs, decimal64 rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return mixed_equality_impl(rhs, lhs);
}

constexpr auto operator!=(decimal64 lhs, decimal64 rhs) noexcept -> bool
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator!=(decimal64 lhs, Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator!=(Integer lhs, decimal64 rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return !(lhs == rhs);
}

constexpr auto operator<(decimal64 lhs, decimal64 rhs) noexcept -> bool
{
    if (isnan(lhs) || isnan(rhs) ||
        (!lhs.isneg() && rhs.isneg()))
    {
        return false;
    }
    else if (lhs.isneg() && !rhs.isneg())
    {
        return true;
    }
    else if (isfinite(lhs) && isinf(rhs))
    {
        if (!rhs.isneg())
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return less_parts_impl<decimal64>(lhs.full_significand(), lhs.biased_exponent(), lhs.isneg(),
                                      rhs.full_significand(), rhs.biased_exponent(), rhs.isneg());
}

template <typename Integer>
constexpr auto operator<(decimal64 lhs, Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return less_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator<(Integer lhs, decimal64 rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    if (isnan(rhs))
    {
        return false;
    }

    return !less_impl(rhs, lhs) && lhs != rhs;
}

constexpr auto operator<=(decimal64 lhs, decimal64 rhs) noexcept -> bool
{
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }

    return !(rhs < lhs);
}

template <typename Integer>
constexpr auto operator<=(decimal64 lhs, Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    if (isnan(lhs))
    {
        return false;
    }

    return !(rhs < lhs);
}

template <typename Integer>
constexpr auto operator<=(Integer lhs, decimal64 rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    if (isnan(rhs))
    {
        return false;
    }

    return !(rhs < lhs);
}

constexpr auto operator>(decimal64 lhs, decimal64 rhs) noexcept -> bool
{
    return rhs < lhs;
}

template <typename Integer>
constexpr auto operator>(decimal64 lhs, Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return rhs < lhs;
}

template <typename Integer>
constexpr auto operator>(Integer lhs, decimal64 rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return rhs < lhs;
}

constexpr auto operator>=(decimal64 lhs, decimal64 rhs) noexcept -> bool
{
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }

    return !(lhs < rhs);
}

template <typename Integer>
constexpr auto operator>=(decimal64 lhs, Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    if (isnan(lhs))
    {
        return false;
    }

    return !(lhs < rhs);
}

template <typename Integer>
constexpr auto operator>=(Integer lhs, decimal64 rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    if (isnan(rhs))
    {
        return false;
    }

    return !(lhs < rhs);
}

#ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR

constexpr auto operator<=>(decimal64 lhs, decimal64 rhs) noexcept -> std::partial_ordering
{
    if (lhs < rhs)
    {
        return std::partial_ordering::less;
    }
    else if (lhs > rhs)
    {
        return std::partial_ordering::greater;
    }
    else if (lhs == rhs)
    {
        return std::partial_ordering::equivalent;
    }

    return std::partial_ordering::unordered;
}

template <typename Integer>
constexpr auto operator<=>(decimal64 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, std::partial_ordering>
{
    if (lhs < rhs)
    {
        return std::partial_ordering::less;
    }
    else if (lhs > rhs)
    {
        return std::partial_ordering::greater;
    }
    else if (lhs == rhs)
    {
        return std::partial_ordering::equivalent;
    }

    return std::partial_ordering::unordered;
}

template <typename Integer>
constexpr auto operator<=>(Integer lhs, decimal64 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, std::partial_ordering>
{
    if (lhs < rhs)
    {
        return std::partial_ordering::less;
    }
    else if (lhs > rhs)
    {
        return std::partial_ordering::greater;
    }
    else if (lhs == rhs)
    {
        return std::partial_ordering::equivalent;
    }

    return std::partial_ordering::unordered;
}

#endif

constexpr auto operator&(decimal64 lhs, decimal64 rhs) noexcept -> decimal64
{
    return from_bits(lhs.bits_ & rhs.bits_);
}

template <typename Integer>
constexpr auto operator&(decimal64 lhs, Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>
{
    return from_bits(lhs.bits_ & static_cast<std::uint64_t>(rhs));
}

template <typename Integer>
constexpr auto operator&(Integer lhs, decimal64 rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>
{
    return from_bits(static_cast<std::uint64_t>(lhs) & rhs.bits_);
}

constexpr auto operator|(decimal64 lhs, decimal64 rhs) noexcept -> decimal64
{
    return from_bits(lhs.bits_ | rhs.bits_);
}

template <typename Integer>
constexpr auto operator|(decimal64 lhs, Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>
{
    return from_bits(lhs.bits_ | static_cast<std::uint64_t>(rhs));
}

template <typename Integer>
constexpr auto operator|(Integer lhs, decimal64 rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>
{
    return from_bits(static_cast<std::uint64_t>(lhs) | rhs.bits_);
}

constexpr auto operator^(decimal64 lhs, decimal64 rhs) noexcept -> decimal64
{
    return from_bits(lhs.bits_ ^ rhs.bits_);
}

template <typename Integer>
constexpr auto operator^(decimal64 lhs, Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>
{
    return from_bits(lhs.bits_ ^ static_cast<std::uint64_t>(rhs));
}

template <typename Integer>
constexpr auto operator^(Integer lhs, decimal64 rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>
{
    return from_bits(static_cast<std::uint64_t>(lhs) ^ rhs.bits_);
}

constexpr auto operator<<(decimal64 lhs, decimal64 rhs) noexcept -> decimal64
{
    return from_bits(lhs.bits_ << rhs.bits_);
}

template <typename Integer>
constexpr auto operator<<(decimal64 lhs, Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>
{
    return from_bits(lhs.bits_ << static_cast<std::uint64_t>(rhs));
}

template <typename Integer>
constexpr auto operator<<(Integer lhs, decimal64 rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>
{
    return from_bits(static_cast<std::uint64_t>(lhs) << rhs.bits_);
}

constexpr auto operator>>(decimal64 lhs, decimal64 rhs) noexcept -> decimal64
{
    return from_bits(lhs.bits_ >> rhs.bits_);
}

template <typename Integer>
constexpr auto operator>>(decimal64 lhs, Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>
{
    return from_bits(lhs.bits_ >> static_cast<std::uint64_t>(rhs));
}

template <typename Integer>
constexpr auto operator>>(Integer lhs, decimal64 rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal64>
{
    return from_bits(static_cast<std::uint64_t>(lhs) >> rhs.bits_);
}

constexpr auto operator~(decimal64 lhs) noexcept -> decimal64
{
    return from_bits(~lhs.bits_);
}

// 3.6.4
// Effects: determines if the quantum exponents of x and y are the same.
// If both x and y are NaN, or infinity, they have the same quantum exponents;
// if exactly one operand is infinity or exactly one operand is NaN, they do not have the same quantum exponents.
// The samequantum functions raise no exception.
constexpr auto samequantumd64(decimal64 lhs, decimal64 rhs) noexcept -> bool
{
    const auto lhs_fp {fpclassify(lhs)};
    const auto rhs_fp {fpclassify(rhs)};

    if ((lhs_fp == FP_NAN && rhs_fp == FP_NAN) || (lhs_fp == FP_INFINITE && rhs_fp == FP_INFINITE))
    {
        return true;
    }
    if ((lhs_fp == FP_NAN || rhs_fp == FP_INFINITE) || (rhs_fp == FP_NAN || lhs_fp == FP_INFINITE))
    {
        return false;
    }

    return lhs.unbiased_exponent() == rhs.unbiased_exponent();
}

// 3.6.5
// Effects: if x is finite, returns its quantum exponent.
// Otherwise, a domain error occurs and INT_MIN is returned.
constexpr auto quantexpd64(decimal64 x) noexcept -> int
{
    if (!isfinite(x))
    {
        return INT_MIN;
    }

    return static_cast<int>(x.unbiased_exponent());
}

// 3.6.6
// Returns: a number that is equal in value (except for any rounding) and sign to x,
// and which has an exponent set to be equal to the exponent of y.
// If the exponent is being increased, the value is correctly rounded according to the current rounding mode;
// if the result does not have the same value as x, the "inexact" floating-point exception is raised.
// If the exponent is being decreased and the significand of the result has more digits than the type would allow,
// the "invalid" floating-point exception is raised and the result is NaN.
// If one or both operands are NaN the result is NaN.
// Otherwise, if only one operand is infinity, the "invalid" floating-point exception is raised and the result is NaN.
// If both operands are infinity, the result is DEC_INFINITY, with the same sign as x, converted to the type of x.
// The quantize functions do not signal underflow.
constexpr auto quantized64(decimal64 lhs, decimal64 rhs) noexcept -> decimal64
{
    // Return the correct type of nan
    if (isnan(lhs))
    {
        return lhs;
    }
    else if (isnan(rhs))
    {
        return rhs;
    }

    // If one is infinity then return a signaling NAN
    if (isinf(lhs) != isinf(rhs))
    {
        return boost::decimal::from_bits(boost::decimal::detail::d64_snan_mask);
    }
    else if (isinf(lhs) && isinf(rhs))
    {
        return lhs;
    }

    return {lhs.full_significand(), rhs.biased_exponent(), lhs.isneg()};
}

constexpr auto scalblnd64(decimal64 num, long exp) noexcept -> decimal64
{
    constexpr decimal64 zero {0, 0};

    if (num == zero || exp == 0 || isinf(num) || isnan(num))
    {
        return num;
    }

    num.edit_exponent(num.biased_exponent() + exp);

    return num;
}

constexpr auto scalbnd64(decimal64 num, int expval) noexcept -> decimal64
{
    return scalblnd64(num, static_cast<long>(expval));
}

constexpr auto copysignd64(decimal64 mag, decimal64 sgn) noexcept -> decimal64
{
    mag.edit_sign(sgn.isneg());
    return mag;
}

constexpr auto fmad64(decimal64 x, decimal64 y, decimal64 z) noexcept -> decimal64
{
    // First calculate x * y without rounding
    constexpr decimal64 zero {0, 0};

    const auto res {detail::check_non_finite(x, y)};
    if (res != zero)
    {
        return res;
    }

    auto sig_lhs {x.full_significand()};
    auto exp_lhs {x.biased_exponent()};
    detail::normalize<decimal64>(sig_lhs, exp_lhs);

    auto sig_rhs {y.full_significand()};
    auto exp_rhs {y.biased_exponent()};
    detail::normalize<decimal64>(sig_rhs, exp_rhs);

    auto mul_result {d64_mul_impl(sig_lhs, exp_lhs, x.isneg(), sig_rhs, exp_rhs, y.isneg())};
    const decimal64 dec_result {mul_result.sig, mul_result.exp, mul_result.sign};

    const auto res_add {detail::check_non_finite(dec_result, z)};
    if (res_add != zero)
    {
        return res_add;
    }

    bool lhs_bigger {dec_result > z};
    if (dec_result.isneg() && z.isneg())
    {
        lhs_bigger = !lhs_bigger;
    }
    bool abs_lhs_bigger {abs(dec_result) > abs(z)};

    detail::normalize<decimal64>(mul_result.sig, mul_result.exp);

    auto sig_z {z.full_significand()};
    auto exp_z {z.biased_exponent()};
    detail::normalize<decimal64>(sig_z, exp_z);
    detail::decimal64_components z_components {sig_z, exp_z, z.isneg()};

    if (!lhs_bigger)
    {
        detail::swap(mul_result, z_components);
        abs_lhs_bigger = !abs_lhs_bigger;
    }

    detail::decimal64_components result {};

    if (!mul_result.sign && z_components.sign)
    {
        result = d64_sub_impl(mul_result.sig, mul_result.exp, mul_result.sign,
                              z_components.sig, z_components.exp, z_components.sign,
                              abs_lhs_bigger);
    }
    else
    {
        result = d64_add_impl(mul_result.sig, mul_result.exp, mul_result.sign,
                              z_components.sig, z_components.exp, z_components.sign);
    }

    return {result.sig, result.exp, result.sign};
}

} //namespace decimal
} //namespace boost

namespace std {

template <>
#ifdef BOOST_MSVC
class numeric_limits<boost::decimal::decimal64>
#else
struct numeric_limits<boost::decimal::decimal64>
#endif
{

#ifdef BOOST_MSVC
    public:
#endif

    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_specialized = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_signed = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_integer = false;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_exact = false;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool has_infinity = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool has_quiet_NaN = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool has_signaling_NaN = true;

    // These members were deprecated in C++23
    #if ((!defined(_MSC_VER) && (__cplusplus <= 202002L)) || (defined(_MSC_VER) && (_MSVC_LANG <= 202002L)))
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr std::float_denorm_style has_denorm = std::denorm_present;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool has_denorm_loss = true;
    #endif

    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr std::float_round_style round_style = std::round_indeterminate;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_iec559 = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_bounded = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_modulo = false;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  digits = 16;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  digits10 = digits;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  max_digits10 = digits;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  radix = 10;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  min_exponent = -382;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  min_exponent10 = min_exponent;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  max_exponent = 385;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int  max_exponent10 = max_exponent;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool traps = numeric_limits<std::uint64_t>::traps;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool tinyness_before = true;

    // Member functions
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto (min)        () -> boost::decimal::decimal64 { return {1, min_exponent}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto (max)        () -> boost::decimal::decimal64 { return {9'999'999'999'999'999, max_exponent}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto lowest       () -> boost::decimal::decimal64 { return {-9'999'999'999'999'999, max_exponent}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto epsilon      () -> boost::decimal::decimal64 { return {1, -16}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto round_error  () -> boost::decimal::decimal64 { return epsilon(); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto infinity     () -> boost::decimal::decimal64 { return boost::decimal::from_bits(boost::decimal::detail::d64_inf_mask); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto quiet_NaN    () -> boost::decimal::decimal64 { return boost::decimal::from_bits(boost::decimal::detail::d64_nan_mask); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto signaling_NaN() -> boost::decimal::decimal64 { return boost::decimal::from_bits(boost::decimal::detail::d64_snan_mask); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto denorm_min   () -> boost::decimal::decimal64 { return {1, boost::decimal::detail::etiny_v<boost::decimal::decimal64>}; }
};

} // Namespace std

#endif //BOOST_DECIMAL_DECIMAL64_HPP
