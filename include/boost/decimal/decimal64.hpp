// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DECIMAL64_HPP
#define BOOST_DECIMAL_DECIMAL64_HPP

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
#include <boost/decimal/detail/comparison.hpp>
#include <boost/decimal/detail/mixed_decimal_arithmetic.hpp>
#include <boost/decimal/detail/check_non_finite.hpp>
#include <boost/decimal/detail/shrink_significand.hpp>
#include <boost/decimal/detail/cmath/isfinite.hpp>
#include <boost/decimal/detail/cmath/fpclassify.hpp>
#include <boost/decimal/detail/cmath/abs.hpp>
#include <boost/decimal/detail/cmath/floor.hpp>
#include <boost/decimal/detail/cmath/ceil.hpp>
#include <boost/decimal/detail/add_impl.hpp>
#include <boost/decimal/detail/sub_impl.hpp>
#include <boost/decimal/detail/mul_impl.hpp>
#include <boost/decimal/detail/div_impl.hpp>
#include <boost/decimal/detail/promote_significand.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE

#include <cerrno>
#include <cinttypes>
#include <climits>
#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <cstring>

#if !defined(BOOST_DECIMAL_DISABLE_IOSTREAM)
#include <cwchar>
#include <iostream>
#endif

#endif // BOOST_DECIMAL_BUILD_MODULE

namespace boost {
namespace decimal {
namespace detail {

// See IEEE 754 section 3.5.2
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d64_inf_mask = UINT64_C(0b0'11110'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d64_nan_mask = UINT64_C(0b0'11111'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d64_snan_mask = UINT64_C(0b0'11111'10000000'0000000000'0000000000'0000000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d64_comb_inf_mask = UINT64_C(0b0'11110'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d64_comb_nan_mask = UINT64_C(0b0'11111'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d64_exp_snan_mask = UINT64_C(0b0'00000'10000000'0000000000'0000000000'0000000000'0000000000'0000000000);

// Masks to update the significand based on the combination field
// In these first three 00, 01, or 10 are the leading 2 bits of the exp
// and the trailing 3 bits are to be concatenated onto the significand (23 bits total)
//
//    Comb.  Exponent          Significand
// s 00 TTT (00)eeeeeeee (0TTT)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
// s 01 TTT (01)eeeeeeee (0TTT)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
// s 10 TTT (10)eeeeeeee (0TTT)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_sign_mask = UINT64_C(0b1'00000'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_combination_field_mask = UINT64_C(0b0'11111'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_exponent_mask = UINT64_C(0b0'00000'11111111'0000000000'0000000000'0000000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_significand_mask = UINT64_C(0b0'00000'00000000'1111111111'1111111111'1111111111'1111111111'1111111111);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_significand_bits = UINT64_C(50);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_exponent_bits = UINT64_C(8);

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_comb_01_mask = UINT64_C(0b0'01000'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_comb_10_mask = UINT64_C(0b0'10000'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_comb_00_01_10_significand_bits = UINT64_C(0b0'00111'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);

// This mask is used to determine if we use the masks above or below since 11 TTT is invalid
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_comb_11_mask = UINT64_C(0b0'11000'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_comb_11_exp_bits = UINT64_C(0b0'00110'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_comb_11_significand_bits = UINT64_C(0b0'00001'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);

// For these masks the first two bits of the combination field imply 100 T as the
// leading bits of the significand and then bits 3 and 4 are the exp
//
//    Comb.  Exponent          Significand
// s 1100 T (00)eeeeeeee (100T)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
// s 1101 T (01)eeeeeeee (100T)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
// s 1110 T (10)eeeeeeee (100T)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_comb_1101_mask = UINT64_C(0b0'11010'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_comb_1110_mask = UINT64_C(0b0'11100'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);

// Powers of 2 used to determine the size of the significand
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_no_combination = 0b1111111111'1111111111'1111111111'1111111111'1111111111;
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_big_combination = 0b111'1111111111'1111111111'1111111111'1111111111'1111111111;

// Exponent fields
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_max_exp_no_combination = 0b11111111;
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_exp_one_combination = 0b1'11111111;
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_max_biased_exp = 0b10'11111111;
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_small_combination_field_mask = 0b111'0000000000'0000000000'0000000000'0000000000'0000000000;
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_big_combination_field_mask = 0b1'0000000000'0000000000'0000000000'0000000000'0000000000;

// Constexpr construction form an uint64_t without having to memcpy
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_construct_sign_mask = 0b1'00000'00000000'0000000000'0000000000'0000000000'0000000000'0000000000;
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_construct_combination_mask = 0b0'11111'00000000'0000000000'0000000000'0000000000'0000000000'0000000000;
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_construct_exp_mask = 0b0'00000'11111111'0000000000'0000000000'0000000000'0000000000'0000000000;
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_construct_significand_mask = d64_no_combination;

struct decimal64_components
{
    using significand_type = std::uint64_t;
    using biased_exponent_type = std::int32_t;

    significand_type sig;
    biased_exponent_type exp;
    bool sign;
};

} //namespace detail

#if defined(__GNUC__) && __GNUC__ >= 8
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif

BOOST_DECIMAL_EXPORT class decimal64 final
{
public:
    using significand_type = std::uint64_t;
    using exponent_type = std::uint64_t;
    using biased_exponent_type = std::int32_t;

private:

    std::uint64_t bits_ {};

    // Returns the un-biased (quantum) exponent
    constexpr auto unbiased_exponent() const noexcept -> exponent_type;

    // Returns the biased exponent
    constexpr auto biased_exponent() const noexcept -> biased_exponent_type;

    // Allows direct editing of the exp
    template <typename T>
    constexpr auto edit_exponent(T exp) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, T, void);

    // Returns the significand complete with the bits implied from the combination field
    constexpr auto full_significand() const noexcept -> significand_type;
    constexpr auto isneg() const noexcept -> bool;
    constexpr auto edit_sign(bool sign) noexcept -> void;

    // Attempts conversion to integral type:
    // If this is nan sets errno to EINVAL and returns 0
    // If this is not representable sets errno to ERANGE and returns 0
    template <typename Decimal, typename TargetType>
    friend constexpr auto to_integral(Decimal val) noexcept
        BOOST_DECIMAL_REQUIRES_TWO_RETURN(detail::is_decimal_floating_point_v, Decimal, detail::is_integral_v, TargetType, TargetType);

    template <typename Decimal, typename TargetType>
    friend BOOST_DECIMAL_CXX20_CONSTEXPR auto to_float(Decimal val) noexcept
        BOOST_DECIMAL_REQUIRES_TWO_RETURN(detail::is_decimal_floating_point_v, Decimal, detail::is_floating_point_v, TargetType, TargetType);

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetType, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal>
    friend constexpr auto to_decimal(Decimal val) noexcept -> TargetType;

    // Debug bit pattern
    friend constexpr auto from_bits(std::uint64_t bits) noexcept -> decimal64;
    friend constexpr auto to_bits(decimal64 rhs) noexcept -> std::uint64_t;

    // Equality template between any integer type and decimal64
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, BOOST_DECIMAL_INTEGRAL Integer>
    friend constexpr auto mixed_equality_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
    friend constexpr auto mixed_decimal_equality_impl(Decimal1 lhs, Decimal2 rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                             detail::is_decimal_floating_point_v<Decimal2>), bool>;

    // Template to compare operator< for any integer type and decimal64
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, BOOST_DECIMAL_INTEGRAL Integer>
    friend constexpr auto less_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
    friend constexpr auto mixed_decimal_less_impl(Decimal1 lhs, Decimal2 rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                             detail::is_decimal_floating_point_v<Decimal2>), bool>;

    friend constexpr auto d64_div_impl(decimal64 lhs, decimal64 rhs, decimal64& q, decimal64& r) noexcept -> void;

    friend constexpr auto d64_mod_impl(decimal64 lhs, decimal64 rhs, const decimal64& q, decimal64& r) noexcept -> void;

    template <typename T>
    friend constexpr auto ilogb(T d) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, T, int);

    template <typename T>
    friend constexpr auto logb(T num) noexcept
        BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T);

public:
    // 3.2.3.1 construct/copy/destroy
    constexpr decimal64() noexcept = default;
    constexpr decimal64& operator=(const decimal64& rhs) noexcept = default;
    constexpr decimal64& operator=(decimal64&& rhs) noexcept = default;
    constexpr decimal64(const decimal64& rhs) noexcept = default;
    constexpr decimal64(decimal64&& rhs) noexcept = default;

    // 3.2.2.2 Conversion form floating-point type
    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_REAL Float>
    #else
    template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool> = true>
    #endif
    #ifndef BOOST_DECIMAL_ALLOW_IMPLICIT_CONVERSIONS
    explicit
    #endif
    BOOST_DECIMAL_CXX20_CONSTEXPR decimal64(Float val) noexcept;

    template <typename Float>
    BOOST_DECIMAL_CXX20_CONSTEXPR auto operator=(const Float& val) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_floating_point_v, Float, decimal64&);

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal>
    #else
    template <typename Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool> = true>
    #endif
    explicit constexpr decimal64(Decimal val) noexcept;

    // 3.2.3.3 Conversion from integral type
    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_INTEGRAL Integer>
    #else
    template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool> = true>
    #endif
    constexpr decimal64(Integer val) noexcept;

    template <typename Integer>
    constexpr auto operator=(const Integer& val) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64&);

    // 3.2.3.4 Conversion to integral type
    explicit constexpr operator bool() const noexcept;
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

    #ifdef BOOST_DECIMAL_HAS_INT128
    explicit constexpr operator detail::int128_t() const noexcept;
    explicit constexpr operator detail::uint128_t() const noexcept;
    #endif


    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool> = true>
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
    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
    #else
    template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1> && detail::is_integral_v<T2>, bool> = true>
    #endif
    constexpr decimal64(T1 coeff, T2 exp, bool sign = false) noexcept;

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_INTEGRAL T>
    #else
    template <typename T, std::enable_if_t<detail::is_integral_v<T>, bool> = true>
    #endif
    constexpr decimal64(bool coeff, T exp, bool sign = false) noexcept;

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
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64);

    template <typename Integer>
    friend constexpr auto operator+(Integer lhs, decimal64 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64);

    friend constexpr auto operator-(decimal64 lhs, decimal64 rhs) noexcept -> decimal64;

    template <typename Integer>
    friend constexpr auto operator-(decimal64 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64);

    template <typename Integer>
    friend constexpr auto operator-(Integer lhs, decimal64 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64);

    friend constexpr auto operator*(decimal64 lhs, decimal64 rhs) noexcept -> decimal64;

    template <typename Integer>
    friend constexpr auto operator*(decimal64 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64);

    template <typename Integer>
    friend constexpr auto operator*(Integer lhs, decimal64 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64);

    friend constexpr auto operator/(decimal64 lhs, decimal64 rhs) noexcept -> decimal64;

    template <typename Integer>
    friend constexpr auto operator/(decimal64 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64);

    template <typename Integer>
    friend constexpr auto operator/(Integer lhs, decimal64 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64);

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
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64&);

    template <typename Decimal>
    constexpr auto operator+=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal64&);

    constexpr auto operator-=(decimal64 rhs) noexcept -> decimal64&;

    template <typename Integer>
    constexpr auto operator-=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64&);

    template <typename Decimal>
    constexpr auto operator-=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal64&);

    constexpr auto operator*=(decimal64 rhs) noexcept -> decimal64&;

    template <typename Integer>
    constexpr auto operator*=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64&);

    template <typename Decimal>
    constexpr auto operator*=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal64&);

    constexpr auto operator/=(decimal64 rhs) noexcept -> decimal64&;

    template <typename Integer>
    constexpr auto operator/=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64&);

    template <typename Decimal>
    constexpr auto operator/=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal64&);

    constexpr auto operator%=(decimal64 rhs) noexcept -> decimal64&;

    // 3.2.9 Comparison operators:
    // Equality
    friend constexpr auto operator==(decimal64 lhs, decimal64 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator==(decimal64 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator==(Integer lhs, decimal64 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Inequality
    friend constexpr auto operator!=(decimal64 lhs, decimal64 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator!=(decimal64 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator!=(Integer lhs, decimal64 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Less
    friend constexpr auto operator<(decimal64 lhs, decimal64 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator<(decimal64 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator<(Integer lhs, decimal64 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Less equal
    friend constexpr auto operator<=(decimal64 lhs, decimal64 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator<=(decimal64 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator<=(Integer lhs, decimal64 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Greater
    friend constexpr auto operator>(decimal64 lhs, decimal64 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator>(decimal64 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator>(Integer lhs, decimal64 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Greater equal
    friend constexpr auto operator>=(decimal64 lhs, decimal64 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator>=(decimal64 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator>=(Integer lhs, decimal64 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // C++20 spaceship
    #ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR
    friend constexpr auto operator<=>(decimal64 lhs, decimal64 rhs) noexcept -> std::partial_ordering;

    template <typename Integer>
    friend constexpr auto operator<=>(decimal64 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering);

    template <typename Integer>
    friend constexpr auto operator<=>(Integer lhs, decimal64 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering);
    #endif

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
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64);

    template <typename Integer>
    friend constexpr auto operator&(Integer lhs, decimal64 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64);

    friend constexpr auto operator|(decimal64 lhs, decimal64 rhs) noexcept -> decimal64;

    template <typename Integer>
    friend constexpr auto operator|(decimal64 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64);

    template <typename Integer>
    friend constexpr auto operator|(Integer lhs, decimal64 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64);

    friend constexpr auto operator^(decimal64 lhs, decimal64 rhs) noexcept -> decimal64;

    template <typename Integer>
    friend constexpr auto operator^(decimal64 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64);

    template <typename Integer>
    friend constexpr auto operator^(Integer lhs, decimal64 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64);

    friend constexpr auto operator<<(decimal64 lhs, decimal64 rhs) noexcept -> decimal64;

    template <typename Integer>
    friend constexpr auto operator<<(decimal64 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64);

    template <typename Integer>
    friend constexpr auto operator<<(Integer lhs, decimal64 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64);

    friend constexpr auto operator>>(decimal64 lhs, decimal64 rhs) noexcept -> decimal64;

    template <typename Integer>
    friend constexpr auto operator>>(decimal64 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64);

    template <typename Integer>
    friend constexpr auto operator>>(Integer lhs, decimal64 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64);

    friend constexpr auto operator~(decimal64 lhs) noexcept -> decimal64;

    // <cmath> functions that need to be friends
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
    friend constexpr auto frexp10(T num, int* expptr) noexcept -> typename T::significand_type;

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

#if defined(__GNUC__) && __GNUC__ >= 6
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wduplicated-branches"
#endif

// 3.2.5 initialization from coefficient and exponent:
#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
#else
template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1> && detail::is_integral_v<T2>, bool>>
#endif
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
    if (unsigned_coeff_digits > detail::precision_v<decimal64> + 1)
    {
        const auto digits_to_remove {unsigned_coeff_digits - (detail::precision_v<decimal64> + 1)};

        #if defined(__GNUC__) && !defined(__clang__)
        #  pragma GCC diagnostic push
        #  pragma GCC diagnostic ignored "-Wconversion"
        #endif

        unsigned_coeff /= detail::pow10(static_cast<Unsigned_Integer>(digits_to_remove));

        #if defined(__GNUC__) && !defined(__clang__)
        #  pragma GCC diagnostic pop
        #endif

        exp += digits_to_remove;
        unsigned_coeff_digits -= digits_to_remove;
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

#if defined(__GNUC__) && __GNUC__ >= 6
#  pragma GCC diagnostic pop
#endif

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wfloat-equal"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_REAL Float>
#else
template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool>>
#endif
BOOST_DECIMAL_CXX20_CONSTEXPR decimal64::decimal64(Float val) noexcept
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (val != val)
    {
        *this = from_bits(detail::d64_nan_mask);
    }
    else if (val == std::numeric_limits<Float>::infinity() || val == -std::numeric_limits<Float>::infinity())
    {
        *this = from_bits(detail::d64_inf_mask);
    }
    else
    #endif
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

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

template <typename Float>
BOOST_DECIMAL_CXX20_CONSTEXPR auto decimal64::operator=(const Float& val) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_floating_point_v, Float, decimal64&)
{
    *this = decimal64{val};
    return *this;
}

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal>
#else
template <typename Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool>>
#endif
constexpr decimal64::decimal64(Decimal val) noexcept
{
    *this = to_decimal<decimal64>(val);
}

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_INTEGRAL Integer>
#else
template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool>>
#endif
constexpr decimal64::decimal64(Integer val) noexcept // NOLINT : Incorrect parameter is never used
{
    using ConversionType = std::conditional_t<std::is_same<Integer, bool>::value, std::int32_t, Integer>;
    *this = decimal64{static_cast<ConversionType>(val), 0};
}

template <typename Integer>
constexpr auto decimal64::operator=(const Integer& val) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64&)
{
    using ConversionType = std::conditional_t<std::is_same<Integer, bool>::value, std::int32_t, Integer>;
    *this = decimal64{static_cast<ConversionType>(val), 0};
    return *this;
}

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_INTEGRAL T>
#else
template <typename T, std::enable_if_t<detail::is_integral_v<T>, bool>>
#endif
constexpr decimal64::decimal64(bool coeff, T exp, bool sign) noexcept
{
    *this = decimal64(static_cast<std::int32_t>(coeff), exp, sign);
}

constexpr decimal64::operator bool() const noexcept
{
    constexpr decimal64 zero {0, 0};
    return *this != zero;
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

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr decimal64::operator detail::int128_t() const noexcept
{
    return to_integral<decimal64, detail::int128_t>(*this);
}

constexpr decimal64::operator detail::uint128_t() const noexcept
{
    return to_integral<decimal64, detail::uint128_t>(*this);
}

#endif


template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool>>
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

constexpr auto decimal64::unbiased_exponent() const noexcept -> exponent_type
{
    exponent_type expval {};

    const auto exp_comb_bits {(bits_ & detail::d64_comb_11_mask)};

    switch (exp_comb_bits)
    {
        case detail::d64_comb_11_mask:
            expval = (bits_ & detail::d64_comb_11_exp_bits) >> (detail::d64_significand_bits + 1);
            break;
        case detail::d64_comb_10_mask:
            expval = UINT64_C(0b1000000000);
            break;
        case detail::d64_comb_01_mask:
            expval = UINT64_C(0b0100000000);
            break;
    }

    expval |= (bits_ & detail::d64_exponent_mask) >> detail::d64_significand_bits;

    return expval;
}

constexpr auto decimal64::biased_exponent() const noexcept -> biased_exponent_type
{
    return static_cast<std::int32_t>(unbiased_exponent()) - detail::bias_v<decimal64>;
}

constexpr auto decimal64::full_significand() const noexcept -> significand_type
{
    significand_type significand {};

    if ((bits_ & detail::d64_comb_11_mask) == detail::d64_comb_11_mask)
    {
        // Only need the one bit of T because the other 3 are implied
        significand = (bits_ & detail::d64_comb_11_significand_bits) == detail::d64_comb_11_significand_bits ?
            UINT64_C(0b1001'0000000000'0000000000'0000000000'0000000000'0000000000) :
            UINT64_C(0b1000'0000000000'0000000000'0000000000'0000000000'0000000000);
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

template <typename T>
constexpr auto decimal64::edit_exponent(T expval) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, T, void)
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

constexpr auto d64_div_impl(decimal64 lhs, decimal64 rhs, decimal64& q, decimal64& r) noexcept -> void
{
    #ifndef BOOST_DECIMAL_FAST_MATH
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
    #else
    static_cast<void>(r);
    #endif

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

    q = detail::d64_generic_div_impl<decimal64>(lhs_components, rhs_components);
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
    #ifndef BOOST_DECIMAL_FAST_MATH
    constexpr decimal64 zero {0, 0};

    const auto res {detail::check_non_finite(lhs, rhs)};
    if (res != zero)
    {
        return res;
    }
    #endif

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

    return detail::d64_add_impl<decimal64>(lhs_sig, lhs_exp, lhs.isneg(),
                                           rhs_sig, rhs_exp, rhs.isneg());
}

template <typename Integer>
constexpr auto operator+(decimal64 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64)
{
    using promoted_significand_type = detail::promote_significand_t<decimal64, Integer>;
    using exp_type = decimal64::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs) || isinf(lhs))
    {
        return lhs;
    }
    #endif

    bool lhs_bigger {lhs > rhs};
    if (lhs.isneg() && (rhs < 0))
    {
        lhs_bigger = !lhs_bigger;
    }

    auto sig_rhs {static_cast<promoted_significand_type>(detail::make_positive_unsigned(rhs))};
    bool abs_lhs_bigger {abs(lhs) > sig_rhs};

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize<decimal64>(sig_lhs, exp_lhs);
    auto lhs_components {detail::decimal64_components{sig_lhs, exp_lhs, lhs.isneg()}};

    exp_type exp_rhs {0};
    detail::normalize<decimal64>(sig_rhs, exp_rhs);
    const auto final_sig_rhs {static_cast<detail::decimal64_components::significand_type>(sig_rhs)};
    auto rhs_components {detail::decimal64_components{final_sig_rhs, exp_rhs, (rhs < 0)}};

    if (!lhs_bigger)
    {
        detail::swap(lhs_components, rhs_components);
        abs_lhs_bigger = !abs_lhs_bigger;
    }

    #ifdef BOOST_DECIMAL_DEBUG_ADD
    std::cerr << "Lhs sig: " << lhs_components.sig
              << "\nLhs exp: " << lhs_components.exp
              << "\nRhs sig: " << rhs_components.sig
              << "\nRhs exp: " << rhs_components.exp << std::endl;
    #endif

    if (!lhs_components.sign && rhs_components.sign)
    {
        return detail::d64_sub_impl<decimal64>(lhs_components.sig, lhs_components.exp, lhs_components.sign,
                                               rhs_components.sig, rhs_components.exp, rhs_components.sign,
                                               abs_lhs_bigger);
    }
    else
    {
        return detail::d64_add_impl<decimal64>(lhs_components.sig, lhs_components.exp, lhs_components.sign,
                                               rhs_components.sig, rhs_components.exp, rhs_components.sign);
    }
}

template <typename Integer>
constexpr auto operator+(Integer lhs, decimal64 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64)
{
    return rhs + lhs;
}

// NOLINTNEXTLINE : If subtraction is actually addition than use operator+ and vice versa
constexpr auto operator-(decimal64 lhs, decimal64 rhs) noexcept -> decimal64
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    constexpr decimal64 zero {0, 0};

    const auto res {detail::check_non_finite(lhs, rhs)};
    if (res != zero)
    {
        return res;
    }
    #endif

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

    return detail::d64_sub_impl<decimal64>(sig_lhs, exp_lhs, lhs.isneg(),
                                           sig_rhs, exp_rhs, rhs.isneg(),
                                           abs_lhs_bigger);
}

template <typename Integer>
constexpr auto operator-(decimal64 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64)
{
    using promoted_significand_type = detail::promote_significand_t<decimal64, Integer>;
    using exp_type = decimal64::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isinf(lhs) || isnan(lhs))
    {
        return lhs;
    }
    #endif

    if (!lhs.isneg() && (rhs < 0))
    {
        return lhs + detail::make_positive_unsigned(rhs);
    }

    auto sig_rhs {static_cast<promoted_significand_type>(detail::make_positive_unsigned(rhs))};
    const bool abs_lhs_bigger {abs(lhs) > sig_rhs};

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize<decimal64>(sig_lhs, exp_lhs);

    exp_type exp_rhs {0};
    detail::normalize<decimal64>(sig_rhs, exp_rhs);
    const auto final_sig_rhs {static_cast<decimal64::significand_type>(sig_rhs)};

    return detail::d64_sub_impl<decimal64>(sig_lhs, exp_lhs, lhs.isneg(),
                                           final_sig_rhs, exp_rhs, (rhs < 0),
                                           abs_lhs_bigger);
}

template <typename Integer>
constexpr auto operator-(Integer lhs, decimal64 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64)
{
    using promoted_significand_type = detail::promote_significand_t<decimal64, Integer>;
    using exp_type = decimal64::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isinf(rhs) || isnan(rhs))
    {
        return rhs;
    }
    #endif

    if (lhs >= 0 && rhs.isneg())
    {
        return lhs + (-rhs);
    }

    auto sig_lhs {static_cast<promoted_significand_type>(detail::make_positive_unsigned(lhs))};
    const bool abs_lhs_bigger {sig_lhs > abs(rhs)};

    exp_type exp_lhs {0};
    detail::normalize<decimal64>(sig_lhs, exp_lhs);
    const auto final_sig_lhs {static_cast<decimal64::significand_type>(detail::make_positive_unsigned(sig_lhs))};

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize<decimal64>(sig_rhs, exp_rhs);

    return detail::d64_sub_impl<decimal64>(final_sig_lhs, exp_lhs, (lhs < 0),
                                           sig_rhs, exp_rhs, rhs.isneg(),
                                           abs_lhs_bigger);
}

constexpr auto operator*(decimal64 lhs, decimal64 rhs) noexcept -> decimal64
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    constexpr decimal64 zero {0, 0};

    const auto non_finite {detail::check_non_finite(lhs, rhs)};
    if (non_finite != zero)
    {
        return non_finite;
    }
    #endif

    auto lhs_sig {lhs.full_significand()};
    auto lhs_exp {lhs.biased_exponent()};
    detail::normalize<decimal64>(lhs_sig, lhs_exp);

    auto rhs_sig {rhs.full_significand()};
    auto rhs_exp {rhs.biased_exponent()};
    detail::normalize<decimal64>(rhs_sig, rhs_exp);

    return detail::d64_mul_impl<decimal64>(lhs_sig, lhs_exp, lhs.isneg(),
                                           rhs_sig, rhs_exp, rhs.isneg());
}

template <typename Integer>
constexpr auto operator*(decimal64 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64)
{
    using promoted_significand_type = detail::promote_significand_t<decimal64, Integer>;
    using exp_type = decimal64::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs) || isinf(lhs))
    {
        return lhs;
    }
    #endif

    auto lhs_sig {lhs.full_significand()};
    auto lhs_exp {lhs.biased_exponent()};
    detail::normalize<decimal64>(lhs_sig, lhs_exp);

    auto rhs_sig {static_cast<promoted_significand_type>(detail::make_positive_unsigned(rhs))};
    exp_type rhs_exp {0};
    detail::normalize<decimal64>(rhs_sig, rhs_exp);
    const auto final_rhs_sig {static_cast<decimal64::significand_type>(rhs_sig)};

    return detail::d64_mul_impl<decimal64>(lhs_sig, lhs_exp, lhs.isneg(),
                                           final_rhs_sig, rhs_exp, (rhs < 0));
}

template <typename Integer>
constexpr auto operator*(Integer lhs, decimal64 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64)
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
constexpr auto operator/(decimal64 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64)
{
    using exp_type = decimal64::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
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
    #endif

    auto lhs_sig {lhs.full_significand()};
    auto lhs_exp {lhs.biased_exponent()};
    detail::normalize<decimal64>(lhs_sig, lhs_exp);

    detail::decimal64_components lhs_components {lhs_sig, lhs_exp, lhs.isneg()};

    auto rhs_sig {static_cast<std::uint64_t>(detail::make_positive_unsigned(rhs))};
    exp_type rhs_exp {};
    detail::decimal64_components rhs_components {detail::shrink_significand<std::uint64_t>(rhs_sig, rhs_exp), rhs_exp, rhs < 0};

    return detail::d64_generic_div_impl<decimal64>(lhs_components, rhs_components);
}

template <typename Integer>
constexpr auto operator/(Integer lhs, decimal64 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
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
    #endif

    auto rhs_sig {rhs.full_significand()};
    auto rhs_exp {rhs.biased_exponent()};
    detail::normalize<decimal64>(rhs_sig, rhs_exp);

    detail::decimal64_components lhs_components {detail::make_positive_unsigned(lhs), 0, lhs < 0};
    detail::decimal64_components rhs_components {rhs_sig, rhs_exp, rhs.isneg()};

    return detail::d64_generic_div_impl<decimal64>(lhs_components, rhs_components);
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
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64&)
{
    *this = *this + rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal64::operator+=(Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal64&)
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
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64&)
{
    *this = *this - rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal64::operator-=(Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal64&)
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
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64&)
{
    *this = *this * rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal64::operator*=(Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal64&)
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
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64&)
{
    *this = *this / rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal64::operator/=(Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal64&)
{
    *this = *this / rhs;
    return *this;
}

constexpr auto decimal64::operator%=(decimal64 rhs) noexcept -> decimal64&
{
    *this = *this % rhs;
    return *this;
}

constexpr auto operator==(decimal64 lhs, decimal64 rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check for IEEE requirement that nan != nan
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }
    #endif

    return equal_parts_impl<decimal64>(lhs.full_significand(), lhs.biased_exponent(), lhs.isneg(),
                                       rhs.full_significand(), rhs.biased_exponent(), rhs.isneg());
}

template <typename Integer>
constexpr auto operator==(decimal64 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return mixed_equality_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator==(Integer lhs, decimal64 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return mixed_equality_impl(rhs, lhs);
}

constexpr auto operator!=(decimal64 lhs, decimal64 rhs) noexcept -> bool
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator!=(decimal64 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator!=(Integer lhs, decimal64 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return !(lhs == rhs);
}

constexpr auto operator<(decimal64 lhs, decimal64 rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
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
        return !rhs.isneg();
    }
    #else
    if (!lhs.isneg() && rhs.isneg())
    {
        return false;
    }
    else if (lhs.isneg() && !rhs.isneg())
    {
        return true;
    }
    #endif

    return less_parts_impl<decimal64>(lhs.full_significand(), lhs.biased_exponent(), lhs.isneg(),
                                      rhs.full_significand(), rhs.biased_exponent(), rhs.isneg());
}

template <typename Integer>
constexpr auto operator<(decimal64 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return less_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator<(Integer lhs, decimal64 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(rhs))
    {
        return false;
    }
    #endif

    return !less_impl(rhs, lhs) && lhs != rhs;
}

constexpr auto operator<=(decimal64 lhs, decimal64 rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }
    #endif

    return !(rhs < lhs);
}

template <typename Integer>
constexpr auto operator<=(decimal64 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs))
    {
        return false;
    }
    #endif

    return !(rhs < lhs);
}

template <typename Integer>
constexpr auto operator<=(Integer lhs, decimal64 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(rhs))
    {
        return false;
    }
    #endif

    return !(rhs < lhs);
}

constexpr auto operator>(decimal64 lhs, decimal64 rhs) noexcept -> bool
{
    return rhs < lhs;
}

template <typename Integer>
constexpr auto operator>(decimal64 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return rhs < lhs;
}

template <typename Integer>
constexpr auto operator>(Integer lhs, decimal64 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return rhs < lhs;
}

constexpr auto operator>=(decimal64 lhs, decimal64 rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }
    #endif

    return !(lhs < rhs);
}

template <typename Integer>
constexpr auto operator>=(decimal64 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs))
    {
        return false;
    }
    #endif

    return !(lhs < rhs);
}

template <typename Integer>
constexpr auto operator>=(Integer lhs, decimal64 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(rhs))
    {
        return false;
    }
    #endif

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
constexpr auto operator<=>(decimal64 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering)
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
constexpr auto operator<=>(Integer lhs, decimal64 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering)
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
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64)
{
    return from_bits(lhs.bits_ & static_cast<std::uint64_t>(rhs));
}

template <typename Integer>
constexpr auto operator&(Integer lhs, decimal64 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64)
{
    return from_bits(static_cast<std::uint64_t>(lhs) & rhs.bits_);
}

constexpr auto operator|(decimal64 lhs, decimal64 rhs) noexcept -> decimal64
{
    return from_bits(lhs.bits_ | rhs.bits_);
}

template <typename Integer>
constexpr auto operator|(decimal64 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64)
{
    return from_bits(lhs.bits_ | static_cast<std::uint64_t>(rhs));
}

template <typename Integer>
constexpr auto operator|(Integer lhs, decimal64 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64)
{
    return from_bits(static_cast<std::uint64_t>(lhs) | rhs.bits_);
}

constexpr auto operator^(decimal64 lhs, decimal64 rhs) noexcept -> decimal64
{
    return from_bits(lhs.bits_ ^ rhs.bits_);
}

template <typename Integer>
constexpr auto operator^(decimal64 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64)
{
    return from_bits(lhs.bits_ ^ static_cast<std::uint64_t>(rhs));
}

template <typename Integer>
constexpr auto operator^(Integer lhs, decimal64 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64)
{
    return from_bits(static_cast<std::uint64_t>(lhs) ^ rhs.bits_);
}

constexpr auto operator<<(decimal64 lhs, decimal64 rhs) noexcept -> decimal64
{
    return from_bits(lhs.bits_ << rhs.bits_);
}

template <typename Integer>
constexpr auto operator<<(decimal64 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64)
{
    return from_bits(lhs.bits_ << static_cast<std::uint64_t>(rhs));
}

template <typename Integer>
constexpr auto operator<<(Integer lhs, decimal64 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64)
{
    return from_bits(static_cast<std::uint64_t>(lhs) << rhs.bits_);
}

constexpr auto operator>>(decimal64 lhs, decimal64 rhs) noexcept -> decimal64
{
    return from_bits(lhs.bits_ >> rhs.bits_);
}

template <typename Integer>
constexpr auto operator>>(decimal64 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64)
{
    return from_bits(lhs.bits_ >> static_cast<std::uint64_t>(rhs));
}

template <typename Integer>
constexpr auto operator>>(Integer lhs, decimal64 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64)
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
    #ifndef BOOST_DECIMAL_FAST_MATH
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
    #endif

    return lhs.unbiased_exponent() == rhs.unbiased_exponent();
}

// 3.6.5
// Effects: if x is finite, returns its quantum exponent.
// Otherwise, a domain error occurs and INT_MIN is returned.
constexpr auto quantexpd64(decimal64 x) noexcept -> int
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (!isfinite(x))
    {
        return INT_MIN;
    }
    #endif

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
    #ifndef BOOST_DECIMAL_FAST_MATH
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
    #endif

    return {lhs.full_significand(), rhs.biased_exponent(), lhs.isneg()};
}

constexpr auto scalblnd64(decimal64 num, long exp) noexcept -> decimal64
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    constexpr decimal64 zero {0, 0};

    if (num == zero || exp == 0 || isinf(num) || isnan(num))
    {
        return num;
    }
    #endif

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

} //namespace decimal
} //namespace boost

namespace std {

BOOST_DECIMAL_EXPORT template <>
#ifdef _MSC_VER
class numeric_limits<boost::decimal::decimal64>
#else
struct numeric_limits<boost::decimal::decimal64>
#endif
{

#ifdef _MSC_VER
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
