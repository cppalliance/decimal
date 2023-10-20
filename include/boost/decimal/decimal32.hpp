// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DECIMAL32_HPP
#define BOOST_DECIMAL_DECIMAL32_HPP

#include <cassert>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <iostream>
#include <limits>
#include <type_traits>
#include <sstream>

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/bit_cast.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/emulated128.hpp>
#include <boost/decimal/detail/fenv_rounding.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <boost/decimal/detail/parser.hpp>
#include <boost/decimal/detail/power_tables.hpp>
#include <boost/decimal/detail/ryu/ryu_generic_128.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/utilities.hpp>
#include <boost/decimal/detail/normalize.hpp>
#include <boost/decimal/detail/comparison.hpp>
#include <boost/decimal/detail/mixed_decimal_arithmetic.hpp>
#include <boost/decimal/detail/to_integral.hpp>
#include <boost/decimal/detail/to_float.hpp>
#include <boost/decimal/detail/to_decimal.hpp>
#include <boost/decimal/detail/promotion.hpp>
#include <boost/decimal/detail/io.hpp>
#include <boost/decimal/detail/check_non_finite.hpp>
#include <boost/decimal/detail/shrink_significand.hpp>
#include <boost/decimal/detail/cmath/isfinite.hpp>
#include <boost/decimal/detail/cmath/fpclassify.hpp>
#include <boost/decimal/detail/cmath/abs.hpp>
#include <boost/decimal/detail/cmath/floor.hpp>
#include <boost/decimal/detail/cmath/ceil.hpp>

namespace boost { namespace decimal {

namespace detail {

// See IEEE 754 section 3.5.2
static constexpr auto d32_inf_mask      = UINT32_C(0b0'11110'000000'0000000000'0000000000);
static constexpr auto d32_nan_mask      = UINT32_C(0b0'11111'000000'0000000000'0000000000);
static constexpr auto d32_snan_mask     = UINT32_C(0b0'11111'100000'0000000000'0000000000);
static constexpr auto d32_comb_inf_mask = UINT32_C(0b0'11110'000000'0000000000'0000000000);
static constexpr auto d32_comb_nan_mask = UINT32_C(0b0'11111'000000'0000000000'0000000000);
static constexpr auto d32_exp_snan_mask = UINT32_C(0b0'10000'000000'0000000000'0000000000);

// Masks to update the significand based on the combination field
// In these first three 00, 01, or 10 are the leading 2 bits of the exp
// and the trailing 3 bits are to be concatenated onto the significand (23 bits total)
//
//    Comb.  Exponent          Significand
// s 00 TTT (00)eeeeee (0TTT)[tttttttttt][tttttttttt]
// s 01 TTT (01)eeeeee (0TTT)[tttttttttt][tttttttttt]
// s 10 TTT (10)eeeeee (0TTT)[tttttttttt][tttttttttt]
static constexpr std::uint32_t d32_sign_mask = UINT32_C(0b1'00000'000000'0000000000'0000000000);

static constexpr std::uint32_t d32_comb_01_mask = UINT32_C(0b0'01000'000000'0000000000'0000000000);
static constexpr std::uint32_t d32_comb_10_mask = UINT32_C(0b0'10000'000000'0000000000'0000000000);

// This mask is used to determine if we use the masks above or below since 11 TTT is invalid
static constexpr std::uint32_t d32_comb_11_mask = UINT32_C(0b0'11000'000000'0000000000'0000000000);
static constexpr std::uint32_t d32_comb_11_exp_bits = UINT32_C(0b0'00110'000000'0000000000'0000000000);
static constexpr std::uint32_t d32_comb_11_significand_bits = UINT32_C(0b0'00001'000000'0000000000'0000000000);

// For these masks the first two bits of the combination field imply 100 T as the
// leading bits of the significand and then bits 3 and 4 are the exp
//
//    Comb.  Exponent          Significand
// s 1100 T (00)eeeeee (100T)[tttttttttt][tttttttttt]
// s 1101 T (01)eeeeee (100T)[tttttttttt][tttttttttt]
// s 1110 T (10)eeeeee (100T)[tttttttttt][tttttttttt]
// static constexpr std::uint32_t comb_1100_mask = 0b11000;
static constexpr std::uint32_t d32_comb_1101_mask = UINT32_C(0b0'11010'000000'0000000000'0000000000);
static constexpr std::uint32_t d32_comb_1110_mask = UINT32_C(0b0'11100'000000'0000000000'0000000000);

// Powers of 2 used to determine the size of the significand
static constexpr std::uint32_t d32_no_combination = UINT32_C(0b1111111111'1111111111);
static constexpr std::uint32_t d32_big_combination = UINT32_C(0b0111'1111111111'1111111111);

// Exponent fields
static constexpr std::uint32_t d32_max_exp_no_combination = UINT32_C(0b111111);
static constexpr std::uint32_t d32_exp_combination_field_mask = d32_max_exp_no_combination;
static constexpr std::uint32_t d32_exp_one_combination = UINT32_C(0b1'111111);
static constexpr std::uint32_t d32_max_biased_exp = UINT32_C(0b10'111111);
static constexpr std::uint32_t d32_small_combination_field_mask = UINT32_C(0b0'00000'000111'0000000000'0000000000);
static constexpr std::uint32_t d32_big_combination_field_mask = UINT32_C(0b0'00000'000001'0000000000'0000000000);

// Constexpr construction from an uint32_t without having to memcpy
//static constexpr std::uint32_t d32_construct_sign_mask = UINT32_C(0b1'00000'000000'0000000000'0000000000);
//static constexpr std::uint32_t d32_construct_combination_mask = UINT32_C(0b0'11111'000000'0000000000'0000000000);
//static constexpr std::uint32_t d32_construct_exp_mask = UINT32_C(0b0'00000'111111'0000000000'0000000000);
//static constexpr std::uint32_t d32_construct_significand_mask = d32_no_combination;

struct decimal32_components
{
    std::uint32_t sig;
    std::int32_t exp;
    bool sign;
};

} // namespace detail

// ISO/IEC DTR 24733
// 3.2.2 class decimal32
class decimal32 final // NOLINT(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
{
private:

    std::uint32_t bits_ {};

    // Returns the un-biased (quantum) exponent
    constexpr auto unbiased_exponent() const noexcept -> std::uint32_t;

    // Returns the biased exponent
    constexpr auto biased_exponent() const noexcept -> std::int32_t;

    // Returns the significand complete with the bits implied from the combination field
    constexpr auto full_significand() const noexcept -> std::uint32_t;
    constexpr auto isneg() const noexcept -> bool;

    // Attempts conversion to integral type:
    // If this is nan sets errno to EINVAL and returns 0
    // If this is not representable sets errno to ERANGE and returns 0
    template <typename Decimal, typename TargetType>
    friend constexpr auto to_integral(Decimal val) noexcept -> TargetType;

    template <typename Decimal, typename TargetType>
    friend BOOST_DECIMAL_CXX20_CONSTEXPR auto to_float(Decimal val) noexcept -> TargetType;

    template <typename TargetType, typename Decimal>
    friend constexpr auto to_decimal(Decimal val) noexcept -> TargetType;

    friend constexpr auto generic_div_impl(detail::decimal32_components lhs, detail::decimal32_components rhs,
                                           detail::decimal32_components& q) noexcept -> void;
    friend constexpr auto div_impl(decimal32 lhs, decimal32 rhs, decimal32& q, decimal32& r) noexcept -> void;
    friend constexpr auto mod_impl(decimal32 lhs, decimal32 rhs, const decimal32& q, decimal32& r) noexcept -> void;

    template <typename T>
    friend constexpr auto ilogb(T d) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, int>;

    // Debug bit pattern
    friend constexpr auto from_bits(std::uint32_t bits) noexcept -> decimal32;
    friend BOOST_DECIMAL_CXX20_CONSTEXPR auto to_bits(decimal32 rhs) noexcept -> std::uint32_t;
    friend auto debug_pattern(decimal32 rhs) noexcept -> void;

    // Equality template between any integer type and decimal32
    template <typename Decimal, typename Integer>
    friend constexpr auto mixed_equality_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    template <typename Decimal1, typename Decimal2>
    friend constexpr auto mixed_decimal_equality_impl(Decimal1 lhs, Decimal2 rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                             detail::is_decimal_floating_point_v<Decimal2>), bool>;

    // Template to compare operator< for any integer type and decimal32
    template <typename Decimal, typename Integer>
    friend constexpr auto less_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    template <typename Decimal1, typename Decimal2>
    friend constexpr auto mixed_decimal_less_impl(Decimal1 lhs, Decimal2 rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                             detail::is_decimal_floating_point_v<Decimal2>), bool>;

    template <typename T, typename T2>
    friend constexpr auto add_impl(T lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                                   T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> detail::decimal32_components;

    template <typename T, typename T2>
    friend constexpr auto sub_impl(T lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                                   T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign,
                                   bool abs_lhs_bigger) noexcept -> detail::decimal32_components;

    template <typename T, typename T2>
    friend constexpr auto mul_impl(T lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                                   T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> detail::decimal32_components;

public:
    // 3.2.2.1 construct/copy/destroy:
    constexpr decimal32() noexcept = default;

    // 3.2.2.2 Conversion from floating-point type
    template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool> = true>
    explicit BOOST_DECIMAL_CXX20_CONSTEXPR decimal32(Float val) noexcept;

    template <typename Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool> = true>
    explicit constexpr decimal32(Decimal val) noexcept;

    // 3.2.2.3 Conversion from integral type
    template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool> = true>
    explicit constexpr decimal32(Integer val) noexcept;

    // 3.2.2.4 Conversion to integral type
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

    // 3.2.5 initialization from coefficient and exponent:
    template <typename T, typename T2, std::enable_if_t<detail::is_integral_v<T>, bool> = true>
    constexpr decimal32(T coeff, T2 exp, bool sign = false) noexcept;

    constexpr decimal32(const decimal32& val) noexcept = default;
    constexpr decimal32(decimal32&& val) noexcept = default;
    constexpr auto operator=(const decimal32& val) noexcept -> decimal32& = default;
    constexpr auto operator=(decimal32&& val) noexcept -> decimal32& = default;

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

    // cmath functions that are easier as friends
    friend constexpr auto signbit     BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool;
    friend constexpr auto isinf       BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool;
    friend constexpr auto isnan       BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool;
    friend constexpr auto issignaling BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool;
    friend constexpr auto isnormal    BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool;

    // 3.2.7 unary arithmetic operators:
    friend constexpr auto operator+(decimal32 rhs) noexcept -> decimal32;
    friend constexpr auto operator-(decimal32 rhs) noexcept -> decimal32;

    // 3.2.8 binary arithmetic operators:
    friend constexpr auto operator+(decimal32 lhs, decimal32 rhs) noexcept -> decimal32;

    template <typename Integer>
    friend constexpr auto operator+(decimal32 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>;

    template <typename Integer>
    friend constexpr auto operator+(Integer lhs, decimal32 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>;

    friend constexpr auto operator-(decimal32 lhs, decimal32 rhs) noexcept -> decimal32;

    template <typename Integer>
    friend constexpr auto operator-(decimal32 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>;

    template <typename Integer>
    friend constexpr auto operator-(Integer lhs, decimal32 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>;

    friend constexpr auto operator*(decimal32 lhs, decimal32 rhs) noexcept -> decimal32;

    template <typename Integer>
    friend constexpr auto operator*(decimal32 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>;

    template <typename Integer>
    friend constexpr auto operator*(Integer lhs, decimal32 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>;

    friend constexpr auto operator/(decimal32 lhs, decimal32 rhs) noexcept -> decimal32;

    template <typename Integer>
    friend constexpr auto operator/(decimal32 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>;

    template <typename Integer>
    friend constexpr auto operator/(Integer lhs, decimal32 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>;

    friend constexpr auto operator%(decimal32 lhs, decimal32 rhs) noexcept -> decimal32;

    // 3.2.2.5 Increment and Decrement
    constexpr auto operator++()    noexcept -> decimal32&;
    constexpr auto operator++(int) noexcept -> decimal32;  // NOLINT : C++14 so constexpr implies const
    constexpr auto operator--()    noexcept -> decimal32&;
    constexpr auto operator--(int) noexcept -> decimal32;  // NOLINT : C++14 so constexpr implies const

    // 3.2.2.6 Compound assignment
    constexpr auto operator+=(decimal32 rhs) noexcept -> decimal32&;

    template <typename Integer>
    constexpr auto operator+=(Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32&>;

    template <typename Decimal>
    constexpr auto operator+=(Decimal rhs) noexcept
        -> std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, decimal32&>;

    constexpr auto operator-=(decimal32 rhs) noexcept -> decimal32&;

    template <typename Integer>
    constexpr auto operator-=(Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32&>;

    template <typename Decimal>
    constexpr auto operator-=(Decimal rhs) noexcept
        -> std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, decimal32&>;

    constexpr auto operator*=(decimal32 rhs) noexcept -> decimal32&;

    template <typename Integer>
    constexpr auto operator*=(Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32&>;

    template <typename Decimal>
    constexpr auto operator*=(Decimal rhs) noexcept
        -> std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, decimal32&>;

    constexpr auto operator/=(decimal32 rhs) noexcept -> decimal32&;

    template <typename Integer>
    constexpr auto operator/=(Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32&>;

    template <typename Decimal>
    constexpr auto operator/=(Decimal rhs) noexcept
        -> std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, decimal32&>;

    constexpr auto operator%=(decimal32 rhs) noexcept -> decimal32&;

    // 3.2.9 comparison operators:
    // Equality
    friend constexpr auto operator==(decimal32 lhs, decimal32 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator==(decimal32 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator==(Integer lhs, decimal32 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    // Inequality
    friend constexpr auto operator!=(decimal32 lhs, decimal32 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator!=(decimal32 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator!=(Integer lhs, decimal32 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    // Less
    friend constexpr auto operator<(decimal32 lhs, decimal32 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator<(decimal32 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator<(Integer lhs, decimal32 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    // Less equal
    friend constexpr auto operator<=(decimal32 lhs, decimal32 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator<=(decimal32 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator<=(Integer lhs, decimal32 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    // Greater
    friend constexpr auto operator>(decimal32 lhs, decimal32 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator>(decimal32 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator>(Integer lhs, decimal32 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    // Greater equal
    friend constexpr auto operator>=(decimal32 lhs, decimal32 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator>=(decimal32 lhs, Integer rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    template <typename Integer>
    friend constexpr auto operator>=(Integer lhs, decimal32 rhs) noexcept
        -> std::enable_if_t<detail::is_integral_v<Integer>, bool>;

    #ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR
    friend constexpr auto operator<=>(decimal32 lhs, decimal32 rhs) noexcept -> std::partial_ordering;

    template <typename Integer>
    friend constexpr auto operator<=>(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, std::partial_ordering>;

    template <typename Integer>
    friend constexpr auto operator<=>(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, std::partial_ordering>;
    #endif

    // 3.2.10 Formatted input:
    template <typename charT, typename traits, typename DecimalType>
    friend auto operator>>(std::basic_istream<charT, traits>& is, DecimalType& d)
        -> std::enable_if_t<detail::is_decimal_floating_point_v<DecimalType>, std::basic_istream<charT, traits>&>;

    // 3.2.11 Formatted output:
    template <typename charT, typename traits, typename DecimalType>
    friend auto operator<<(std::basic_ostream<charT, traits>& os, const DecimalType& d)
        -> std::enable_if_t<detail::is_decimal_floating_point_v<DecimalType>, std::basic_ostream<charT, traits>&>;

    // <cmath> extensions
    // 3.6.4 Same Quantum
    friend constexpr auto samequantumd32(decimal32 lhs, decimal32 rhs) noexcept -> bool;

    // 3.6.5 Quantum exponent
    friend constexpr auto quantexpd32(decimal32 x) noexcept -> int;

    // 3.6.6 Quantize
    friend constexpr auto quantized32(decimal32 lhs, decimal32 rhs) noexcept -> decimal32;

    // <cmath> functions that need to be friends
    friend constexpr auto copysignd32(decimal32 mag, decimal32 sgn) noexcept -> decimal32;
    friend constexpr auto fmad32(decimal32 x, decimal32 y, decimal32 z) noexcept -> decimal32;

    // Related to <cmath>
    template <typename T>
    friend constexpr auto frexp10(T num, int* expptr) noexcept
    -> std::enable_if_t<detail::is_decimal_floating_point_v<T>,
            std::conditional_t<std::is_same<T, decimal32>::value, std::uint32_t, std::uint64_t>>;

    friend constexpr auto scalbnd32(decimal32 num, int exp) noexcept -> decimal32;
    friend constexpr auto scalblnd32(decimal32 num, long exp) noexcept -> decimal32;

    // These can be made public only for debugging matters
#ifndef BOOST_DECIMAL_DEBUG_MEMBERS
private:
#endif
    // Replaces the biased exponent with the value of exp
    template <typename T, std::enable_if_t<detail::is_integral_v<T>, bool> = true>
    constexpr auto edit_exponent(T exp) noexcept -> void;

    // Replaces the value of the significand with sig
    template <typename T, std::enable_if_t<detail::is_integral_v<T>, bool> = true>
    constexpr auto edit_significand(T sig) noexcept -> void;

    // Replaces the current sign with the one provided
    constexpr auto edit_sign(bool sign) noexcept -> void;
};

template <typename T, typename T2, std::enable_if_t<detail::is_integral_v<T>, bool>>
constexpr decimal32::decimal32(T coeff, T2 exp, bool sign) noexcept // NOLINT(readability-function-cognitive-complexity,misc-no-recursion)
{
    using Unsigned_Integer = detail::make_unsigned_t<T>;

    static_assert(detail::is_integral_v<T>, "Coefficient must be an integer");
    static_assert(detail::is_integral_v<T2>, "Exponent must be an integer");

    bits_ = UINT32_C(0);
    bool isneg {false};
    Unsigned_Integer unsigned_coeff {};
    BOOST_DECIMAL_IF_CONSTEXPR (detail::is_signed_v<T>)
    {
        if (coeff < 0 || sign)
        {
            bits_ |= detail::d32_sign_mask;
            isneg = true;
        }
        unsigned_coeff = coeff < static_cast<T>(0) ? static_cast<Unsigned_Integer>(detail::apply_sign(coeff)) :
                                                     static_cast<Unsigned_Integer>(coeff);
    }
    else
    {
        if (sign)
        {
            bits_ |= detail::d32_sign_mask;
            isneg = true;
        }
        unsigned_coeff = static_cast<Unsigned_Integer>(coeff);
    }

    // If the coeff is not in range make it so
    auto unsigned_coeff_digits {detail::num_digits(unsigned_coeff)};
    const bool reduced {unsigned_coeff_digits > detail::precision};
    while (unsigned_coeff_digits > detail::precision + 1)
    {
        unsigned_coeff /= 10;
        ++exp;
        --unsigned_coeff_digits;
    }

    // Round as required
    if (reduced)
    {
        exp += detail::fenv_round(unsigned_coeff, isneg);
    }

    auto reduced_coeff {static_cast<std::uint32_t>(unsigned_coeff)};
    bool big_combination {false};

    if (reduced_coeff == 0)
    {
        exp = 0;
    }
    else if (reduced_coeff <= detail::d32_no_combination)
    {
        // If the coefficient fits directly we don't need to use the combination field
        // bits_.significand = reduced_coeff;
        bits_ |= (reduced_coeff & detail::d32_no_combination);
    }
    else if (reduced_coeff <= detail::d32_big_combination)
    {
        // Break the number into 3 bits for the combination field and 20 bits for the significand field

        // Use the least significant 20 bits to set the significand
        bits_ |= (reduced_coeff & detail::d32_no_combination);

        // Now set the combination field (maximum of 3 bits)
        std::uint32_t remaining_bits {reduced_coeff & detail::d32_small_combination_field_mask};
        remaining_bits <<= UINT32_C(6);
        bits_ |= remaining_bits;
    }
    else
    {
        // Have to use the full combination field
        bits_ |= detail::d32_comb_11_mask;
        big_combination = true;

        bits_ |= reduced_coeff & detail::d32_no_combination;
        const auto remaining_bit {reduced_coeff & detail::d32_big_combination_field_mask};

        if (remaining_bit)
        {
            bits_ |= detail::d32_comb_11_significand_bits;
        }
    }

    // If the exponent fits we do not need to use the combination field
    std::uint32_t biased_exp {static_cast<std::uint32_t>(exp + detail::bias)};
    const std::uint32_t biased_exp_low_six {biased_exp & detail::d32_exp_combination_field_mask};
    const std::uint32_t biased_exp_low_six_bits {biased_exp_low_six << UINT32_C(20)};
    if (biased_exp <= detail::d32_max_exp_no_combination)
    {
        std::uint32_t biased_exp_bits {biased_exp << UINT32_C(20)};
        bits_ |= biased_exp_bits;
    }
    else if (biased_exp <= detail::d32_exp_one_combination)
    {
        if (big_combination)
        {
            bits_ |= detail::d32_comb_1101_mask;
        }
        else
        {
            bits_ |= detail::d32_comb_01_mask;
        }

        bits_ |= biased_exp_low_six_bits;
    }
    else if (biased_exp <= detail::d32_max_biased_exp)
    {
        if (big_combination)
        {
            bits_ |= detail::d32_comb_1110_mask;
        }
        else
        {
            bits_ |= detail::d32_comb_10_mask;
        }

        bits_ |= biased_exp_low_six_bits;
    }
    else
    {
        // The value is probably infinity

        // If we can offset some extra power in the coefficient try to do so
        const auto coeff_dig {detail::num_digits(reduced_coeff)};
        if (coeff_dig < detail::precision)
        {
            for (auto i {coeff_dig}; i <= detail::precision; ++i)
            {
                reduced_coeff *= 10;
                --biased_exp;
                --exp;
                if (biased_exp == detail::d32_max_biased_exp)
                {
                    break;
                }
            }

            if (detail::num_digits(reduced_coeff) <= detail::precision)
            {
                *this = decimal32(reduced_coeff, exp, isneg);
            }
            else
            {
                bits_ |= detail::d32_comb_inf_mask;
            }
        }
        else
        {
            bits_ |= detail::d32_comb_inf_mask;
        }
    }
}

constexpr auto from_bits(std::uint32_t bits) noexcept -> decimal32
{
    decimal32 result;
    result.bits_ = bits;

    return result;
}

constexpr auto signbit BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool
{
    return rhs.bits_ & detail::d32_sign_mask;
}

constexpr auto isnan BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool
{
    return (rhs.bits_ & detail::d32_comb_nan_mask) == detail::d32_comb_nan_mask;
}

constexpr auto issignaling BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool
{
    return isnan(rhs) && (rhs.bits_.exponent & detail::d32_exp_snan_mask) == detail::d32_exp_snan_mask;
}

constexpr auto isinf BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool
{
    return ((rhs.bits_.combination_field & detail::d32_comb_inf_mask) == detail::d32_comb_inf_mask) && (!isnan(rhs));
}

constexpr auto isnormal BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool
{
    // Check for de-normals
    const auto sig {rhs.full_significand()};
    const auto exp {rhs.unbiased_exponent()};

    if (exp <= detail::precision - 1)
    {
        return false;
    }

    return (sig != 0) && isfinite(rhs);
}

constexpr auto operator+(decimal32 rhs) noexcept -> decimal32
{
    return rhs;
}

constexpr auto operator-(decimal32 rhs) noexcept-> decimal32
{
    rhs.bits_.sign ^= 1U;
    return rhs;
}

template <typename T, typename T2>
constexpr auto add_impl(T lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                        T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> detail::decimal32_components
{
    const bool sign {lhs_sign};

    auto delta_exp {lhs_exp > rhs_exp ? lhs_exp - rhs_exp : rhs_exp - lhs_exp};

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "Starting sig lhs: " << sig_lhs
              << "\nStarting sig rhs: " << sig_rhs << std::endl;
    #endif

    if (delta_exp > detail::precision + 1)
    {
        // If the difference in exponents is more than the digits of accuracy
        // we return the larger of the two
        //
        // e.g. 1e20 + 1e-20 = 1e20

        return {lhs_sig, lhs_exp, lhs_sign};
    }
    else if (delta_exp == detail::precision + 1)
    {
        // Only need to see if we need to add one to the
        // significand of the bigger value
        //
        // e.g. 1.234567e5 + 9.876543e-2 = 1.234568e5

        if (rhs_sig >= UINT32_C(5'000'000))
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
    // 32-bit signed int can have 9 digits and our normalized significand has 7
    if (delta_exp <= 2)
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
        lhs_sig *= 100;
        delta_exp -= 2;
        lhs_exp -=2;
    }

    while (delta_exp > 1)
    {
        rhs_sig /= 10;
        --delta_exp;
    }


    if (delta_exp == 1)
    {
        detail::fenv_round(rhs_sig, rhs_sign);
    }

    // Cast the results to signed types so that we can apply a sign at the end if necessary
    // Both of the significands are maximally 24 bits, so they fit into a 32-bit signed type just fine
    const auto new_sig {static_cast<std::int32_t>(lhs_sig + rhs_sig)};
    const auto new_exp {lhs_exp};
    const auto res_sig {detail::make_positive_unsigned(new_sig)};

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "Final sig lhs: " << sig_lhs
              << "\nFinal sig rhs: " << sig_rhs
              << "\nResult sig: " << new_sig << std::endl;
    #endif

    return {res_sig, new_exp, sign};
}

template <typename T, typename T2>
constexpr auto sub_impl(T lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                        T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign,
                        bool abs_lhs_bigger) noexcept -> detail::decimal32_components
{
    auto delta_exp {lhs_exp > rhs_exp ? lhs_exp - rhs_exp : rhs_exp - lhs_exp};
    auto signed_sig_lhs {detail::make_signed_value(lhs_sig, lhs_sign)};
    auto signed_sig_rhs {detail::make_signed_value(rhs_sig, rhs_sign)};

    if (delta_exp > detail::precision + 1)
    {
        // If the difference in exponents is more than the digits of accuracy
        // we return the larger of the two
        //
        // e.g. 1e20 - 1e-20 = 1e20
        return abs_lhs_bigger ? detail::decimal32_components{detail::shrink_significand(lhs_sig, lhs_exp), lhs_exp, false} :
                                detail::decimal32_components{detail::shrink_significand(rhs_sig, rhs_exp), rhs_exp, true};
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
        detail::fenv_round(sig_smaller, smaller_sign);
    }

    // Both of the significands are less than 9'999'999, so we can safely
    // cast them to signed 32-bit ints to calculate the new significand
    std::int32_t new_sig {}; // NOLINT : Value is never used but can't leave uninitialized in constexpr function

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

// We use kahan summation here where applicable
// https://en.wikipedia.org/wiki/Kahan_summation_algorithm
// NOLINTNEXTLINE : If addition is actually subtraction than change operator and vice versa
constexpr auto operator+(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
{
    constexpr decimal32 zero {0, 0};

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

    // Ensure that lhs is always the larger for ease of implementation
    if (!lhs_bigger)
    {
        detail::swap(lhs, rhs);
    }

    if (!lhs.isneg() && rhs.isneg())
    {
        return lhs - abs(rhs);
    }

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize(sig_lhs, exp_lhs);

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize(sig_rhs, exp_rhs);

    const auto result {add_impl(sig_lhs, exp_lhs, lhs.isneg(), sig_rhs, exp_rhs, rhs.isneg())};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator+(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>
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
    detail::normalize(sig_lhs, exp_lhs);

    auto lhs_components {detail::decimal32_components{sig_lhs, exp_lhs, lhs.isneg()}};
    auto sig_rhs {rhs};
    std::int32_t exp_rhs {0};
    detail::normalize(sig_rhs, exp_rhs);
    auto unsigned_sig_rhs = detail::shrink_significand(detail::make_positive_unsigned(sig_rhs), exp_rhs);
    auto rhs_components {detail::decimal32_components{unsigned_sig_rhs, exp_rhs, (rhs < 0)}};

    if (!lhs_bigger)
    {
        detail::swap(lhs_components, rhs_components);
        lhs_bigger = !lhs_bigger;
        abs_lhs_bigger = !abs_lhs_bigger;
    }

    detail::decimal32_components result {};

    if (!lhs_components.sign && rhs_components.sign)
    {
        result = sub_impl(lhs_components.sig, lhs_components.exp, lhs_components.sign,
                          rhs_components.sig, rhs_components.exp, rhs_components.sign,
                          abs_lhs_bigger);
    }
    else
    {
        result = add_impl(lhs_components.sig, lhs_components.exp, lhs_components.sign,
                          rhs_components.sig, rhs_components.exp, rhs_components.sign);
    }

    return decimal32(result.sig, result.exp, result.sign);
}

template <typename Integer>
constexpr auto operator+(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>
{
    return rhs + lhs;
}

constexpr auto decimal32::operator++() noexcept -> decimal32&
{
    constexpr decimal32 one(1, 0);
    *this = *this + one;
    return *this;
}

constexpr auto decimal32::operator++(int) noexcept -> decimal32
{
    return ++(*this);
}

constexpr auto decimal32::operator+=(decimal32 rhs) noexcept -> decimal32&
{
    *this = *this + rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal32::operator+=(Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32&>
{
    *this = *this + rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal32::operator+=(Decimal rhs) noexcept
    -> std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, decimal32&>
{
    *this = *this + rhs;
    return *this;
}

// NOLINTNEXTLINE : If subtraction is actually addition than use operator+ and vice versa
constexpr auto operator-(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
{
    constexpr decimal32 zero {0, 0};

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
    detail::normalize(sig_lhs, exp_lhs);

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize(sig_rhs, exp_rhs);

    const auto result {sub_impl(sig_lhs, exp_lhs, lhs.isneg(),
                                sig_rhs, exp_rhs, rhs.isneg(),
                                abs_lhs_bigger)};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator-(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>
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
    detail::normalize(sig_lhs, exp_lhs);
    auto lhs_components {detail::decimal32_components{sig_lhs, exp_lhs, lhs.isneg()}};

    auto sig_rhs {rhs};
    std::int32_t exp_rhs {0};
    detail::normalize(sig_rhs, exp_rhs);
    auto unsigned_sig_rhs = detail::shrink_significand(detail::make_positive_unsigned(sig_rhs), exp_rhs);
    auto rhs_components {detail::decimal32_components{unsigned_sig_rhs, exp_rhs, (rhs < 0)}};

    const auto result {sub_impl(lhs_components.sig, lhs_components.exp, lhs_components.sign,
                                rhs_components.sig, rhs_components.exp, rhs_components.sign,
                                abs_lhs_bigger)};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator-(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>
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

    auto sig_lhs {detail::make_positive_unsigned(lhs)};
    std::int32_t exp_lhs {0};
    detail::normalize(sig_lhs, exp_lhs);
    auto unsigned_sig_lhs = detail::shrink_significand(detail::make_positive_unsigned(sig_lhs), exp_lhs);
    auto lhs_components {detail::decimal32_components{unsigned_sig_lhs, exp_lhs, (lhs < 0)}};

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize(sig_rhs, exp_rhs);
    auto rhs_components {detail::decimal32_components{sig_rhs, exp_rhs, rhs.isneg()}};

    const auto result {sub_impl(lhs_components.sig, lhs_components.exp, lhs_components.sign,
                                rhs_components.sig, rhs_components.exp, rhs_components.sign,
                                abs_lhs_bigger)};

    return {result.sig, result.exp, result.sign};
}

constexpr auto decimal32::operator--() noexcept -> decimal32&
{
    constexpr decimal32 one(1, 0);
    *this = *this - one;
    return *this;
}

constexpr auto decimal32::operator--(int) noexcept -> decimal32
{
    return --(*this);
}

constexpr auto decimal32::operator-=(decimal32 rhs) noexcept -> decimal32&
{
    *this = *this - rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal32::operator-=(Decimal rhs) noexcept
    -> std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, decimal32&>
{
    *this = *this - rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal32::operator-=(Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32&>
{
    *this = *this - rhs;
    return *this;
}

constexpr auto operator==(decimal32 lhs, decimal32 rhs) noexcept -> bool
{
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }

    return equal_parts_impl(lhs.full_significand(), lhs.biased_exponent(), lhs.isneg(),
                            rhs.full_significand(), rhs.biased_exponent(), rhs.isneg());
}

template <typename Integer>
constexpr auto operator==(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return mixed_equality_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator==(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return mixed_equality_impl(rhs, lhs);
}

constexpr auto operator!=(decimal32 lhs, decimal32 rhs) noexcept -> bool
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator!=(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator!=(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return !(lhs == rhs);
}

constexpr auto operator<(decimal32 lhs, decimal32 rhs) noexcept -> bool
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
        if (!signbit(rhs))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return less_parts_impl(lhs.full_significand(), lhs.biased_exponent(), lhs.isneg(),
                           rhs.full_significand(), rhs.biased_exponent(), rhs.isneg());
}

template <typename Integer>
constexpr auto operator<(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    return less_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator<(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    if (isnan(rhs))
    {
        return false;
    }

    return !less_impl(rhs, lhs) && lhs != rhs;
}

constexpr auto operator<=(decimal32 lhs, decimal32 rhs) noexcept -> bool
{
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }

    return !(rhs < lhs);
}

template <typename Integer>
constexpr auto operator<=(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    if (isnan(lhs))
    {
        return false;
    }

    return !(rhs < lhs);
}

template <typename Integer>
constexpr auto operator<=(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    if (isnan(rhs))
    {
        return false;
    }

    return !(rhs < lhs);
}

constexpr auto operator>(decimal32 lhs, decimal32 rhs) noexcept -> bool
{
    return rhs < lhs;
}

template <typename Integer>
constexpr auto operator>(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    if (isnan(lhs))
    {
        return false;
    }

    return rhs < lhs;
}

template <typename Integer>
constexpr auto operator>(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    if (isnan(rhs))
    {
        return false;
    }

    return rhs < lhs;
}

constexpr auto operator>=(decimal32 lhs, decimal32 rhs) noexcept -> bool
{
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }

    return !(lhs < rhs);
}

template <typename Integer>
constexpr auto operator>=(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    if (isnan(lhs))
    {
        return false;
    }

    return !(lhs < rhs);
}

template <typename Integer>
constexpr auto operator>=(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, bool>
{
    if (isnan(rhs))
    {
        return false;
    }

    return !(lhs < rhs);
}

#ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR

constexpr auto operator<=>(decimal32 lhs, decimal32 rhs) noexcept -> std::partial_ordering
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
constexpr auto operator<=>(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, std::partial_ordering>
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
constexpr auto operator<=>(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, std::partial_ordering>
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

constexpr auto decimal32::unbiased_exponent() const noexcept -> std::uint32_t
{
    std::uint32_t expval {};

    if ((bits_.combination_field & detail::d32_comb_11_mask) == 0b11000)
    {
        // bits 2 and 3 are the exp part of the combination field
        expval |= (bits_.combination_field & detail::d32_comb_11_exp_bits) << 5;
    }
    else
    {
        // bits 0 and 1 are the exp part of the combination field
        expval |= (bits_.combination_field & detail::d32_comb_11_mask) << 3;
    }

    expval |= bits_.exponent;

    return expval;
}

constexpr auto decimal32::biased_exponent() const noexcept -> std::int32_t
{
    return static_cast<std::int32_t>(unbiased_exponent()) - detail::bias;
}

template <typename T, std::enable_if_t<detail::is_integral_v<T>, bool>>
constexpr auto decimal32::edit_exponent(T expval) noexcept -> void
{
    *this = decimal32(this->full_significand(), expval, this->isneg());
}

constexpr auto decimal32::full_significand() const noexcept -> std::uint32_t
{
    std::uint32_t significand {};

    if ((bits_.combination_field & detail::d32_comb_11_mask) == 0b11000)
    {
        // Only need the one bit of T because the other 3 are implied
        if (bits_.combination_field & detail::d32_comb_11_significand_bits)
        {
            significand = 0b1001'0000000000'0000000000;
        }
        else
        {
            significand = 0b1000'0000000000'0000000000;
        }
    }
    else
    {
        significand |= ((bits_.combination_field & 0b00111) << 20);
    }

    significand |= bits_.significand;

    return significand;
}

template <typename T, std::enable_if_t<detail::is_integral_v<T>, bool>>
constexpr auto decimal32::edit_significand(T sig) noexcept -> void
{
    *this = decimal32(sig, this->biased_exponent(), this->isneg());
}

constexpr auto decimal32::isneg() const noexcept -> bool
{
    return static_cast<bool>(bits_.sign);
}

// Allows changing the sign even on nans and infs
constexpr auto decimal32::edit_sign(bool sign) noexcept -> void
{
    this->bits_.sign = static_cast<std::uint32_t>(sign);
}

template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool>>
BOOST_DECIMAL_CXX20_CONSTEXPR decimal32::decimal32(Float val) noexcept
{
    if (val != val)
    {
        *this = boost::decimal::from_bits(boost::decimal::detail::d32_nan_mask);
    }
    else if (val == std::numeric_limits<Float>::infinity() || val == -std::numeric_limits<Float>::infinity())
    {
        *this = boost::decimal::from_bits(boost::decimal::detail::d32_inf_mask);
    }
    else
    {
        const auto components {detail::ryu::floating_point_to_fd128(val)};

        #ifdef BOOST_DECIMAL_DEBUG
        std::cerr << "Mant: " << components.mantissa
                  << "\nExp: " << components.exponent
                  << "\nSign: " << components.sign << std::endl;
        #endif

        if (components.exponent > detail::emax)
        {
            *this = boost::decimal::from_bits(boost::decimal::detail::d32_inf_mask);
        }
        else
        {
            *this = decimal32 {components.mantissa, components.exponent, components.sign};
        }
    }
}

template <typename Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool>>
constexpr decimal32::decimal32(Decimal val) noexcept
{
    *this = to_decimal<decimal32>(val);
}

template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool>>
constexpr decimal32::decimal32(Integer val) noexcept // NOLINT : Incorrect parameter is never used
{
    *this = decimal32{val, 0};
}

constexpr decimal32::operator int() const noexcept
{
    return to_integral<decimal32, int>(*this);
}

constexpr decimal32::operator unsigned() const noexcept
{
    return to_integral<decimal32, unsigned>(*this);
}

constexpr decimal32::operator long() const noexcept
{
    return to_integral<decimal32, long>(*this);
}

constexpr decimal32::operator unsigned long() const noexcept
{
    return to_integral<decimal32, unsigned long>(*this);
}

constexpr decimal32::operator long long() const noexcept
{
    return to_integral<decimal32, long long>(*this);
}

constexpr decimal32::operator unsigned long long() const noexcept
{
    return to_integral<decimal32, unsigned long long>(*this);
}

constexpr decimal32::operator std::int8_t() const noexcept
{
    return to_integral<decimal32, std::int8_t>(*this);
}

constexpr decimal32::operator std::uint8_t() const noexcept
{
    return to_integral<decimal32, std::uint8_t>(*this);
}

constexpr decimal32::operator std::int16_t() const noexcept
{
    return to_integral<decimal32, std::int16_t>(*this);
}

constexpr decimal32::operator std::uint16_t() const noexcept
{
    return to_integral<decimal32, std::uint16_t>(*this);
}

template <typename Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool>>
constexpr decimal32::operator Decimal() const noexcept
{
    return to_decimal<Decimal>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR auto to_bits(decimal32 rhs) noexcept -> std::uint32_t
{
    const auto bits {detail::bit_cast<std::uint32_t>(rhs.bits_)};
    return bits;
}

template <typename T, typename T2>
constexpr auto mul_impl(T lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                        T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> detail::decimal32_components
{
    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "sig lhs: " << sig_lhs
              << "\nexp lhs: " << exp_lhs
              << "\nsig rhs: " << sig_rhs
              << "\nexp rhs: " << exp_rhs;
    #endif

    bool sign {lhs_sign != rhs_sign};

    // Once we have the normalized significands and exponents all we have to do is
    // multiply the significands and add the exponents
    //
    // We use a 64 bit resultant significand because the two 23-bit unsigned significands will always fit

    auto res_sig {static_cast<std::uint64_t>(lhs_sig) * static_cast<std::uint64_t>(rhs_sig)};
    auto res_exp {lhs_exp + rhs_exp};

    const auto sig_dig {detail::num_digits(res_sig)};

    if (sig_dig > 9)
    {
        res_sig /= detail::powers_of_10[sig_dig - 9];
        res_exp += sig_dig - 9;
    }

    const auto res_sig_32 {static_cast<std::uint32_t>(res_sig)};

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "\nres sig: " << res_sig_32
              << "\nres exp: " << res_exp << std::endl;
    #endif

    if (res_sig_32 == 0)
    {
        sign = false;
    }

    return {res_sig_32, res_exp, sign};
}

constexpr auto operator*(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
{
    constexpr decimal32 zero {0, 0};

    const auto res {detail::check_non_finite(lhs, rhs)};
    if (res != zero)
    {
        return res;
    }

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize(sig_lhs, exp_lhs);

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize(sig_rhs, exp_rhs);

    const auto result {mul_impl(sig_lhs, exp_lhs, lhs.isneg(), sig_rhs, exp_rhs, rhs.isneg())};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator*(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>
{
    if (isnan(lhs) || isinf(lhs))
    {
        return lhs;
    }

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize(sig_lhs, exp_lhs);
    auto lhs_components {detail::decimal32_components{sig_lhs, exp_lhs, lhs.isneg()}};

    auto sig_rhs {rhs};
    std::int32_t exp_rhs {0};
    detail::normalize(sig_rhs, exp_rhs);
    auto unsigned_sig_rhs {detail::shrink_significand(detail::make_positive_unsigned(sig_rhs), exp_rhs)};
    auto rhs_components {detail::decimal32_components{unsigned_sig_rhs, exp_rhs, (rhs < 0)}};

    const auto result {mul_impl(lhs_components.sig, lhs_components.exp, lhs_components.sign,
                                rhs_components.sig, rhs_components.exp, rhs_components.sign)};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator*(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>
{
    return rhs * lhs;
}

constexpr auto decimal32::operator*=(decimal32 rhs) noexcept -> decimal32&
{
    *this = *this * rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal32::operator*=(Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32&>
{
    *this = *this * rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal32::operator*=(Decimal rhs) noexcept
    -> std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, decimal32&>
{
    *this = *this * rhs;
    return *this;
}

constexpr auto generic_div_impl(detail::decimal32_components lhs, detail::decimal32_components rhs,
                                detail::decimal32_components& q) noexcept -> void
{
    bool sign {lhs.sign != rhs.sign};

    // If rhs is greater than we need to offset the significands to get the correct values
    // e.g. 4/8 is 0 but 40/8 yields 5 in integer maths
    const auto big_sig_lhs {static_cast<std::uint64_t>(lhs.sig) * detail::powers_of_10[detail::precision]};
    lhs.exp -= detail::precision;

    auto res_sig {big_sig_lhs / static_cast<std::uint64_t>(rhs.sig)};
    auto res_exp {lhs.exp - rhs.exp};

    const auto sig_dig {detail::num_digits(res_sig)};

    if (sig_dig > std::numeric_limits<std::uint32_t>::digits10)
    {
        res_sig /= detail::powers_of_10[sig_dig - std::numeric_limits<std::uint32_t>::digits10];
        res_exp += sig_dig - std::numeric_limits<std::uint32_t>::digits10;
    }

    const auto res_sig_32 {static_cast<std::uint32_t>(res_sig)};

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "\nres sig: " << res_sig_32
              << "\nres exp: " << res_exp << std::endl;
    #endif

    if (res_sig_32 == 0)
    {
        sign = false;
    }

    // Let the constructor handle shrinking it back down and rounding correctly
    q = detail::decimal32_components{res_sig_32, res_exp, sign};
}

constexpr auto div_impl(decimal32 lhs, decimal32 rhs, decimal32& q, decimal32& r) noexcept -> void
{
    // Check pre-conditions
    constexpr decimal32 zero {0, 0};
    constexpr decimal32 nan {boost::decimal::from_bits(boost::decimal::detail::d32_snan_mask)};
    constexpr decimal32 inf {boost::decimal::from_bits(boost::decimal::detail::d32_inf_mask)};

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
    detail::normalize(sig_lhs, exp_lhs);

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize(sig_rhs, exp_rhs);

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "sig lhs: " << sig_lhs
              << "\nexp lhs: " << exp_lhs
              << "\nsig rhs: " << sig_rhs
              << "\nexp rhs: " << exp_rhs << std::endl;
    #endif

    detail::decimal32_components lhs_components {sig_lhs, exp_lhs, lhs.isneg()};
    detail::decimal32_components rhs_components {sig_rhs, exp_rhs, rhs.isneg()};
    detail::decimal32_components q_components {};

    generic_div_impl(lhs_components, rhs_components, q_components);

    q = decimal32(q_components.sig, q_components.exp, q_components.sign);
}

constexpr auto mod_impl(decimal32 lhs, decimal32 rhs, const decimal32& q, decimal32& r) noexcept -> void
{
    constexpr decimal32 zero {0, 0};

    // https://en.cppreference.com/w/cpp/numeric/math/fmod
    auto q_trunc {q > zero ? floor(q) : ceil(q)};
    r = lhs - (decimal32(q_trunc) * rhs);
}

constexpr auto operator/(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
{
    decimal32 q {};
    decimal32 r {};
    div_impl(lhs, rhs, q, r);

    return q;
}

template <typename Integer>
constexpr auto operator/(decimal32 lhs, Integer rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>
{
    // Check pre-conditions
    constexpr decimal32 zero {0, 0};
    constexpr decimal32 nan {boost::decimal::from_bits(boost::decimal::detail::d32_snan_mask)};
    constexpr decimal32 inf {boost::decimal::from_bits(boost::decimal::detail::d32_inf_mask)};

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

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize(sig_lhs, exp_lhs);

    detail::decimal32_components lhs_components {sig_lhs, exp_lhs, lhs.isneg()};
    std::int32_t exp_rhs {};
    detail::decimal32_components rhs_components {detail::shrink_significand(detail::make_positive_unsigned(rhs), exp_rhs), exp_rhs, rhs < 0};
    detail::decimal32_components q_components {};

    generic_div_impl(lhs_components, rhs_components, q_components);

    return decimal32(q_components.sig, q_components.exp, q_components.sign);
}

template <typename Integer>
constexpr auto operator/(Integer lhs, decimal32 rhs) noexcept -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32>
{
    // Check pre-conditions
    constexpr decimal32 zero {0, 0};
    constexpr decimal32 nan {boost::decimal::from_bits(boost::decimal::detail::d32_snan_mask)};
    constexpr decimal32 inf {boost::decimal::from_bits(boost::decimal::detail::d32_inf_mask)};

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

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize(sig_rhs, exp_rhs);

    std::int32_t lhs_exp {};
    auto lhs_sig {detail::make_positive_unsigned(detail::shrink_significand(lhs, lhs_exp))};
    detail::decimal32_components lhs_components {lhs_sig, lhs_exp, lhs < 0};
    detail::decimal32_components rhs_components {sig_rhs, exp_rhs, rhs.isneg()};
    detail::decimal32_components q_components {};

    generic_div_impl(lhs_components, rhs_components, q_components);

    return decimal32(q_components.sig, q_components.exp, q_components.sign);
}

constexpr auto decimal32::operator/=(decimal32 rhs) noexcept -> decimal32&
{
    *this = *this / rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal32::operator/=(Integer rhs) noexcept
    -> std::enable_if_t<detail::is_integral_v<Integer>, decimal32&>
{
    *this = *this / rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal32::operator/=(Decimal rhs) noexcept
    -> std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, decimal32&>
{
    *this = *this / rhs;
    return *this;
}

constexpr auto operator%(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
{
    decimal32 q {};
    decimal32 r {};
    div_impl(lhs, rhs, q, r);
    mod_impl(lhs, rhs, q, r);

    return r;
}

constexpr auto decimal32::operator%=(boost::decimal::decimal32 rhs) noexcept -> decimal32&
{
    *this = *this % rhs;
    return *this;
}

// LCOV_EXCL_START
auto debug_pattern(decimal32 rhs) noexcept -> void
{
    std::cerr << "Sig: " << rhs.full_significand()
              << "\nExp: " << rhs.biased_exponent()
              << "\nNeg: " << rhs.isneg() << std::endl;
}
// LCOV_EXCL_STOP

BOOST_DECIMAL_CXX20_CONSTEXPR decimal32::operator float() const noexcept
{
    return to_float<decimal32, float>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal32::operator double() const noexcept
{
    return to_float<decimal32, double>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal32::operator long double() const noexcept
{
    // Double already has more range and precision than a decimal32 will ever be able to provide
    return static_cast<long double>(to_float<decimal32, double>(*this));
}

#ifdef BOOST_DECIMAL_HAS_FLOAT16
constexpr decimal32::operator std::float16_t() const noexcept
{
    return static_cast<std::float16_t>(to_float<decimal32, float>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_FLOAT32
constexpr decimal32::operator std::float32_t() const noexcept
{
    return static_cast<std::float32_t>(to_float<decimal32, float>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_FLOAT64
constexpr decimal32::operator std::float64_t() const noexcept
{
    return static_cast<std::float64_t>(to_float<decimal32, double>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_BRAINFLOAT16
constexpr decimal32::operator std::bfloat16_t() const noexcept
{
    return static_cast<std::bfloat16_t>(to_float<decimal32, float>(*this));
}
#endif

// 3.6.4
// Effects: determines if the quantum exponents of x and y are the same.
// If both x and y are NaN, or infinity, they have the same quantum exponents;
// if exactly one operand is infinity or exactly one operand is NaN, they do not have the same quantum exponents.
// The samequantum functions raise no exception.
constexpr auto samequantumd32(decimal32 lhs, decimal32 rhs) noexcept -> bool
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
constexpr auto quantexpd32(decimal32 x) noexcept -> int
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
constexpr auto quantized32(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
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
        return boost::decimal::from_bits(boost::decimal::detail::d32_snan_mask);
    }
    else if (isinf(lhs) && isinf(rhs))
    {
        return lhs;
    }

    return {lhs.full_significand(), rhs.biased_exponent(), lhs.isneg()};
}

constexpr auto scalblnd32(decimal32 num, long exp) noexcept -> decimal32
{
    constexpr decimal32 zero {0, 0};

    if (num == zero || exp == 0 || isinf(num) || isnan(num))
    {
        return num;
    }

    num.edit_exponent(num.biased_exponent() + exp);

    return num;
}

constexpr auto scalbnd32(decimal32 num, int expval) noexcept -> decimal32
{
    return scalblnd32(num, static_cast<long>(expval));
}

constexpr auto copysignd32(decimal32 mag, decimal32 sgn) noexcept -> decimal32
{
    mag.edit_sign(sgn.isneg());
    return mag;
}

constexpr auto fmad32(decimal32 x, decimal32 y, decimal32 z) noexcept -> decimal32
{
    // First calculate x * y without rounding
    constexpr decimal32 zero {0, 0};

    const auto res {detail::check_non_finite(x, y)};
    if (res != zero)
    {
        return res;
    }

    auto sig_lhs {x.full_significand()};
    auto exp_lhs {x.biased_exponent()};
    detail::normalize(sig_lhs, exp_lhs);

    auto sig_rhs {y.full_significand()};
    auto exp_rhs {y.biased_exponent()};
    detail::normalize(sig_rhs, exp_rhs);

    auto mul_result {mul_impl(sig_lhs, exp_lhs, x.isneg(), sig_rhs, exp_rhs, y.isneg())};
    const decimal32 dec_result {mul_result.sig, mul_result.exp, mul_result.sign};

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

    detail::normalize(mul_result.sig, mul_result.exp);

    auto sig_z {z.full_significand()};
    auto exp_z {z.biased_exponent()};
    detail::normalize(sig_z, exp_z);
    detail::decimal32_components z_components {sig_z, exp_z, z.isneg()};

    if (!lhs_bigger)
    {
        detail::swap(mul_result, z_components);
        abs_lhs_bigger = !abs_lhs_bigger;
    }

    detail::decimal32_components result {};

    if (!mul_result.sign && z_components.sign)
    {
        result = sub_impl(mul_result.sig, mul_result.exp, mul_result.sign,
                          z_components.sig, z_components.exp, z_components.sign,
                          abs_lhs_bigger);
    }
    else
    {
        result = add_impl(mul_result.sig, mul_result.exp, mul_result.sign,
                          z_components.sig, z_components.exp, z_components.sign);
    }

    return {result.sig, result.exp, result.sign};
}

} // namespace decimal
} // namespace boost

namespace std {

template <>
#ifdef BOOST_MSVC
class numeric_limits<boost::decimal::decimal32>
#else
struct numeric_limits<boost::decimal::decimal32>
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
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int digits = 7;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int digits10 = digits;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int max_digits10 = digits;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int radix = 10;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int min_exponent = -95;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int min_exponent10 = min_exponent;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int max_exponent = 96;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int max_exponent10 = max_exponent;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool traps = numeric_limits<std::uint32_t>::traps;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool tinyness_before = true;

    // Member functions
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto (min)        () -> boost::decimal::decimal32 { return {1, min_exponent}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto (max)        () -> boost::decimal::decimal32 { return {9'999'999, max_exponent}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto lowest       () -> boost::decimal::decimal32 { return {-9'999'999, max_exponent}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto epsilon      () -> boost::decimal::decimal32 { return {1, -7}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto round_error  () -> boost::decimal::decimal32 { return epsilon(); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto infinity     () -> boost::decimal::decimal32 { return boost::decimal::from_bits(boost::decimal::detail::d32_inf_mask); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto quiet_NaN    () -> boost::decimal::decimal32 { return boost::decimal::from_bits(boost::decimal::detail::d32_nan_mask); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto signaling_NaN() -> boost::decimal::decimal32 { return boost::decimal::from_bits(boost::decimal::detail::d32_snan_mask); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto denorm_min   () -> boost::decimal::decimal32 { return {1, boost::decimal::detail::etiny}; }
};

} // Namespace std

#endif // BOOST_DECIMAL_DECIMAL32_HPP
