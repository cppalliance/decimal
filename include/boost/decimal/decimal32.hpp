// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DECIMAL32_HPP
#define BOOST_DECIMAL_DECIMAL32_HPP

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
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>

#if !defined(BOOST_DECIMAL_DISABLE_IOSTREAM)
#include <cwchar>
#include <iostream>
#endif

#endif // BOOST_DECIMAL_BUILD_MODULE

namespace boost { namespace decimal {

namespace detail {

// See IEEE 754 section 3.5.2
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d32_inf_mask      = UINT32_C(0b0'11110'000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d32_nan_mask      = UINT32_C(0b0'11111'000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d32_snan_mask     = UINT32_C(0b0'11111'100000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d32_comb_inf_mask = UINT32_C(0b0'11110'000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d32_comb_nan_mask = UINT32_C(0b0'11111'000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d32_exp_snan_mask = UINT32_C(0b0'00000'100000'0000000000'0000000000);

// Masks to update the significand based on the combination field
// In these first three 00, 01, or 10 are the leading 2 bits of the exp
// and the trailing 3 bits are to be concatenated onto the significand (23 bits total)
//
//    Comb.  Exponent          Significand
// s 00 TTT (00)eeeeee (0TTT)[tttttttttt][tttttttttt]
// s 01 TTT (01)eeeeee (0TTT)[tttttttttt][tttttttttt]
// s 10 TTT (10)eeeeee (0TTT)[tttttttttt][tttttttttt]
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_sign_mask = UINT32_C(0b1'00000'000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_combination_field_mask = UINT32_C(0b0'11111'000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_exponent_mask = UINT32_C(0b0'00000'111111'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_significand_mask = UINT32_C(0b0'00000'000000'1111111111'1111111111);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_significand_bits = UINT32_C(20);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_exponent_bits = UINT32_C(6);

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_comb_01_mask = UINT32_C(0b0'01000'000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_comb_10_mask = UINT32_C(0b0'10000'000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_comb_00_01_10_significand_bits = UINT32_C(0b0'00111'000000'0000000000'0000000000);

// This mask is used to determine if we use the masks above or below since 11 TTT is invalid
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_comb_11_mask = UINT32_C(0b0'11000'000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_comb_11_exp_bits = UINT32_C(0b0'00110'000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_comb_11_significand_bits = UINT32_C(0b0'00001'000000'0000000000'0000000000);

// For these masks the first two bits of the combination field imply 100 T as the
// leading bits of the significand and then bits 3 and 4 are the exp
//
//    Comb.  Exponent          Significand
// s 1100 T (00)eeeeee (100T)[tttttttttt][tttttttttt]
// s 1101 T (01)eeeeee (100T)[tttttttttt][tttttttttt]
// s 1110 T (10)eeeeee (100T)[tttttttttt][tttttttttt]
// BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t comb_1100_mask = 0b11000;
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_comb_1101_mask = UINT32_C(0b0'11010'000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_comb_1110_mask = UINT32_C(0b0'11100'000000'0000000000'0000000000);

// Powers of 2 used to determine the size of the significand
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_no_combination = UINT32_C(0b1111111111'1111111111);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_big_combination = UINT32_C(0b0111'1111111111'1111111111);

// Exponent fields
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_max_exp_no_combination = UINT32_C(0b111111);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_exp_combination_field_mask = d32_max_exp_no_combination;
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_exp_one_combination = UINT32_C(0b1'111111);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_max_biased_exp = UINT32_C(0b10'111111);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_small_combination_field_mask = UINT32_C(0b0'00000'000111'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_big_combination_field_mask = UINT32_C(0b0'00000'000001'0000000000'0000000000);

// Constexpr construction from an uint32_t without having to memcpy
//BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_construct_sign_mask = UINT32_C(0b1'00000'000000'0000000000'0000000000);
//BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_construct_combination_mask = UINT32_C(0b0'11111'000000'0000000000'0000000000);
//BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_construct_exp_mask = UINT32_C(0b0'00000'111111'0000000000'0000000000);
//BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_construct_significand_mask = d32_no_combination;

struct decimal32_components
{
    using significand_type = std::uint32_t;
    using biased_exponent_type = std::int32_t;

    significand_type sig;
    biased_exponent_type exp;
    bool sign;
};

} // namespace detail

#if defined(__GNUC__) && __GNUC__ >= 8
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif


// ISO/IEC DTR 24733
// 3.2.2 class decimal32
BOOST_DECIMAL_EXPORT class decimal32 final // NOLINT(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
{
public:
    using significand_type = std::uint32_t;
    using exponent_type = std::uint32_t;
    using biased_exponent_type = std::int32_t;

private:

    std::uint32_t bits_ {};

    // Returns the un-biased (quantum) exponent
    constexpr auto unbiased_exponent() const noexcept -> exponent_type ;

    // Returns the biased exponent
    constexpr auto biased_exponent() const noexcept -> biased_exponent_type;

    // Returns the significand complete with the bits implied from the combination field
    constexpr auto full_significand() const noexcept -> significand_type ;
    constexpr auto isneg() const noexcept -> bool;

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

    friend constexpr auto div_impl(decimal32 lhs, decimal32 rhs, decimal32& q, decimal32& r) noexcept -> void;
    friend constexpr auto mod_impl(decimal32 lhs, decimal32 rhs, const decimal32& q, decimal32& r) noexcept -> void;

    template <typename T>
    friend constexpr auto ilogb(T d) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, T, int);

    template <typename T>
    friend constexpr auto logb(T num) noexcept
        BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T);

    // Debug bit pattern
    friend constexpr auto from_bits(std::uint32_t bits) noexcept -> decimal32;
    friend BOOST_DECIMAL_CXX20_CONSTEXPR auto to_bits(decimal32 rhs) noexcept -> std::uint32_t;
    friend inline auto debug_pattern(decimal32 rhs) noexcept -> void;

    // Equality template between any integer type and decimal32
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, BOOST_DECIMAL_INTEGRAL Integer>
    friend constexpr auto mixed_equality_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
    friend constexpr auto mixed_decimal_equality_impl(Decimal1 lhs, Decimal2 rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                             detail::is_decimal_floating_point_v<Decimal2>), bool>;

    // Template to compare operator< for any integer type and decimal32
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, BOOST_DECIMAL_INTEGRAL Integer>
    friend constexpr auto less_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
    friend constexpr auto mixed_decimal_less_impl(Decimal1 lhs, Decimal2 rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                             detail::is_decimal_floating_point_v<Decimal2>), bool>;

public:
    // 3.2.2.1 construct/copy/destroy:
    constexpr decimal32() noexcept = default;

    // 3.2.2.2 Conversion from floating-point type
    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_REAL Float>
    #else
    template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool> = true>
    #endif
    #ifndef BOOST_DECIMAL_ALLOW_IMPLICIT_CONVERSIONS
    explicit
    #endif
    BOOST_DECIMAL_CXX20_CONSTEXPR decimal32(Float val) noexcept;

    template <typename Float>
    BOOST_DECIMAL_CXX20_CONSTEXPR auto operator=(const Float& val) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_floating_point_v, Float, decimal32&);

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal>
    #else
    template <typename Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool> = true>
    #endif
    explicit constexpr decimal32(Decimal val) noexcept;

    // 3.2.2.3 Conversion from integral type
    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_INTEGRAL Integer>
    #else
    template <BOOST_DECIMAL_INTEGRAL Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool> = true>
    #endif
    constexpr decimal32(Integer val) noexcept;

    template <typename Integer>
    constexpr auto operator=(const Integer& val) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32&);

    // 3.2.2.4 Conversion to integral type
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

    // 3.2.5 initialization from coefficient and exponent:
    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_INTEGRAL T, BOOST_DECIMAL_INTEGRAL T2>
    #else
    template <typename T, typename T2, std::enable_if_t<detail::is_integral_v<T> && detail::is_integral_v<T2>, bool> = true>
    #endif
    constexpr decimal32(T coeff, T2 exp, bool sign = false) noexcept;

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_INTEGRAL T>
    #else
    template <typename T, std::enable_if_t<detail::is_integral_v<T>, bool> = true>
    #endif
    constexpr decimal32(bool coeff, T exp, bool sign = false) noexcept;

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
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32);

    template <typename Integer>
    friend constexpr auto operator+(Integer lhs, decimal32 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32);

    friend constexpr auto operator-(decimal32 lhs, decimal32 rhs) noexcept -> decimal32;

    template <typename Integer>
    friend constexpr auto operator-(decimal32 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32);

    template <typename Integer>
    friend constexpr auto operator-(Integer lhs, decimal32 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32);

    friend constexpr auto operator*(decimal32 lhs, decimal32 rhs) noexcept -> decimal32;

    template <typename Integer>
    friend constexpr auto operator*(decimal32 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32);

    template <typename Integer>
    friend constexpr auto operator*(Integer lhs, decimal32 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32);

    friend constexpr auto operator/(decimal32 lhs, decimal32 rhs) noexcept -> decimal32;

    template <typename Integer>
    friend constexpr auto operator/(decimal32 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32);

    template <typename Integer>
    friend constexpr auto operator/(Integer lhs, decimal32 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32);

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
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32&);

    template <typename Decimal>
    constexpr auto operator+=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal32&);

    constexpr auto operator-=(decimal32 rhs) noexcept -> decimal32&;

    template <typename Integer>
    constexpr auto operator-=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32&);

    template <typename Decimal>
    constexpr auto operator-=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal32&);

    constexpr auto operator*=(decimal32 rhs) noexcept -> decimal32&;

    template <typename Integer>
    constexpr auto operator*=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32&);

    template <typename Decimal>
    constexpr auto operator*=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal32&);

    constexpr auto operator/=(decimal32 rhs) noexcept -> decimal32&;

    template <typename Integer>
    constexpr auto operator/=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32&);

    template <typename Decimal>
    constexpr auto operator/=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal32&);

    constexpr auto operator%=(decimal32 rhs) noexcept -> decimal32&;

    // 3.2.9 comparison operators:
    // Equality
    friend constexpr auto operator==(decimal32 lhs, decimal32 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator==(decimal32 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator==(Integer lhs, decimal32 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Inequality
    friend constexpr auto operator!=(decimal32 lhs, decimal32 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator!=(decimal32 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator!=(Integer lhs, decimal32 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Less
    friend constexpr auto operator<(decimal32 lhs, decimal32 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator<(decimal32 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator<(Integer lhs, decimal32 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Less equal
    friend constexpr auto operator<=(decimal32 lhs, decimal32 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator<=(decimal32 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator<=(Integer lhs, decimal32 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Greater
    friend constexpr auto operator>(decimal32 lhs, decimal32 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator>(decimal32 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator>(Integer lhs, decimal32 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Greater equal
    friend constexpr auto operator>=(decimal32 lhs, decimal32 rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator>=(decimal32 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator>=(Integer lhs, decimal32 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    #ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR
    friend constexpr auto operator<=>(decimal32 lhs, decimal32 rhs) noexcept -> std::partial_ordering;

    template <typename Integer>
    friend constexpr auto operator<=>(decimal32 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering);

    template <typename Integer>
    friend constexpr auto operator<=>(Integer lhs, decimal32 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering);
    #endif

    // Bitwise operators
    friend constexpr auto operator&(decimal32 lhs, decimal32 rhs) noexcept -> decimal32;

    template <typename Integer>
    friend constexpr auto operator&(decimal32 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32);

    template <typename Integer>
    friend constexpr auto operator&(Integer lhs, decimal32 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32);

    friend constexpr auto operator|(decimal32 lhs, decimal32 rhs) noexcept -> decimal32;

    template <typename Integer>
    friend constexpr auto operator|(decimal32 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32);

    template <typename Integer>
    friend constexpr auto operator|(Integer lhs, decimal32 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32);

    friend constexpr auto operator^(decimal32 lhs, decimal32 rhs) noexcept -> decimal32;

    template <typename Integer>
    friend constexpr auto operator^(decimal32 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32);

    template <typename Integer>
    friend constexpr auto operator^(Integer lhs, decimal32 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32);

    friend constexpr auto operator<<(decimal32 lhs, decimal32 rhs) noexcept -> decimal32;

    template <typename Integer>
    friend constexpr auto operator<<(decimal32 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32);

    template <typename Integer>
    friend constexpr auto operator<<(Integer lhs, decimal32 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32);

    friend constexpr auto operator>>(decimal32 lhs, decimal32 rhs) noexcept -> decimal32;

    template <typename Integer>
    friend constexpr auto operator>>(decimal32 lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32);

    template <typename Integer>
    friend constexpr auto operator>>(Integer lhs, decimal32 rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32);

    friend constexpr auto operator~(decimal32 lhs) noexcept -> decimal32;

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
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
    friend constexpr auto frexp10(T num, int* expptr) noexcept -> typename T::significand_type;

    friend constexpr auto scalbnd32(decimal32 num, int exp) noexcept -> decimal32;
    friend constexpr auto scalblnd32(decimal32 num, long exp) noexcept -> decimal32;

    // These can be made public only for debugging matters
#ifndef BOOST_DECIMAL_DEBUG_MEMBERS
private:
#endif
    // Replaces the biased exponent with the value of exp
    template <typename T>
    constexpr auto edit_exponent(T exp) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, T, void);

    // Replaces the value of the significand with sig
    template <typename T>
    constexpr auto edit_significand(T sig) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, T, void);

    // Replaces the current sign with the one provided
    constexpr auto edit_sign(bool sign) noexcept -> void;
};

#if defined(__GNUC__) && __GNUC__ >= 8
#  pragma GCC diagnostic pop
#endif

#if defined(__GNUC__) && __GNUC__ >= 6
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wduplicated-branches"
#endif

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_INTEGRAL T, BOOST_DECIMAL_INTEGRAL T2>
#else
template <typename T, typename T2, std::enable_if_t<detail::is_integral_v<T> && detail::is_integral_v<T2>, bool>>
#endif
constexpr decimal32::decimal32(T coeff, T2 exp, bool sign) noexcept // NOLINT(readability-function-cognitive-complexity,misc-no-recursion)
{
    using Unsigned_Integer = detail::make_unsigned_t<T>;

    static_assert(detail::is_integral_v<T>, "Coefficient must be an integer");
    static_assert(detail::is_integral_v<T2>, "Exponent must be an integer");

    bits_ = UINT32_C(0);
    bool isneg {false};
    Unsigned_Integer unsigned_coeff {detail::make_positive_unsigned(coeff)};
    BOOST_DECIMAL_IF_CONSTEXPR (detail::is_signed_v<T>)
    {
        if (coeff < 0 || sign)
        {
            bits_ |= detail::d32_sign_mask;
            isneg = true;
        }
    }
    else
    {
        if (sign)
        {
            bits_ |= detail::d32_sign_mask;
            isneg = true;
        }
    }

    // If the coeff is not in range make it so
    auto unsigned_coeff_digits {detail::num_digits(unsigned_coeff)};
    const bool reduced {unsigned_coeff_digits > detail::precision};
    if (unsigned_coeff_digits > detail::precision + 1)
    {
        const auto digits_to_remove {unsigned_coeff_digits - (detail::precision + 1)};

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
        exp += static_cast<T2>(detail::fenv_round(unsigned_coeff, isneg));
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
        bits_ |= (reduced_coeff & detail::d32_significand_mask);
    }
    else if (reduced_coeff <= detail::d32_big_combination)
    {
        // Break the number into 3 bits for the combination field and 20 bits for the significand field

        // Use the least significant 20 bits to set the significand
        bits_ |= (reduced_coeff & detail::d32_significand_mask);

        // Now set the combination field (maximum of 3 bits)
        std::uint32_t remaining_bits {reduced_coeff & detail::d32_small_combination_field_mask};
        remaining_bits <<= detail::d32_exponent_bits;
        bits_ |= remaining_bits;
    }
    else
    {
        // Have to use the full combination field
        bits_ |= detail::d32_comb_11_mask;
        big_combination = true;

        bits_ |= (reduced_coeff & detail::d32_significand_mask);
        const auto remaining_bit {reduced_coeff & detail::d32_big_combination_field_mask};

        if (remaining_bit)
        {
            bits_ |= detail::d32_comb_11_significand_bits;
        }
    }

    // If the exponent fits we do not need to use the combination field
    auto biased_exp {static_cast<std::uint32_t>(exp + detail::bias)};
    const std::uint32_t biased_exp_low_six_bits {(biased_exp & detail::d32_max_exp_no_combination) << detail::d32_significand_bits};

    #ifdef BOOST_DECIMAL_DEBUG_D32_CONSTRUCTOR
    if (biased_exp_low_six_bits > 0b111111'0000000000'0000000000)
    {
        return;
    }
    #endif

    if (biased_exp <= detail::d32_max_exp_no_combination)
    {
        bits_ |= biased_exp_low_six_bits;
    }
    else if (biased_exp <= detail::d32_exp_one_combination)
    {
        if (big_combination)
        {
            bits_ |= (detail::d32_comb_1101_mask | biased_exp_low_six_bits);
        }
        else
        {
            bits_ |= (detail::d32_comb_01_mask | biased_exp_low_six_bits);
        }
    }
    else if (biased_exp <= detail::d32_max_biased_exp)
    {
        if (big_combination)
        {
            bits_ |= (detail::d32_comb_1110_mask | biased_exp_low_six_bits);
        }
        else
        {
            bits_ |= (detail::d32_comb_10_mask | biased_exp_low_six_bits);
        }
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
                if (exp < 0)
                {
                    *this = decimal32(0, 0, isneg);
                }
                else
                {
                    bits_ = detail::d32_comb_inf_mask;
                }
            }
        }
        else
        {
            bits_ = detail::d32_comb_inf_mask;
        }
    }
}

#if defined(__GNUC__) && __GNUC__ >= 6
#  pragma GCC diagnostic pop
#endif

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
    return (rhs.bits_ & detail::d32_nan_mask) == detail::d32_nan_mask;
}

constexpr auto issignaling BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool
{
    return (rhs.bits_ & detail::d32_snan_mask) == detail::d32_snan_mask;
}

constexpr auto isinf BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32 rhs) noexcept -> bool
{
    return ((rhs.bits_ & detail::d32_nan_mask) == detail::d32_inf_mask);
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
    rhs.bits_ ^= detail::d32_sign_mask;
    return rhs;
}

// We use kahan summation here where applicable
// https://en.wikipedia.org/wiki/Kahan_summation_algorithm
// NOLINTNEXTLINE : If addition is actually subtraction than change operator and vice versa
constexpr auto operator+(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    constexpr decimal32 zero {0, 0};

    const auto res {detail::check_non_finite(lhs, rhs)};
    if (res != zero)
    {
        return res;
    }
    #endif

    const bool abs_lhs_bigger {abs(lhs) > abs(rhs)};

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize(sig_lhs, exp_lhs);

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize(sig_rhs, exp_rhs);

    return detail::d32_add_impl<decimal32>(sig_lhs, exp_lhs, lhs.isneg(),
                                           sig_rhs, exp_rhs, rhs.isneg(),
                                           abs_lhs_bigger);
}

template <typename Integer>
constexpr auto operator+(decimal32 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32)
{
    using promoted_significand_type = detail::promote_significand_t<decimal32, Integer>;
    using exp_type = decimal32::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs) || isinf(lhs))
    {
        return lhs;
    }
    #endif

    // Make the significand type wide enough that it won't overflow during normalization
    auto sig_rhs {static_cast<promoted_significand_type>(detail::make_positive_unsigned(rhs))};
    const bool abs_lhs_bigger {abs(lhs) > sig_rhs};

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize(sig_lhs, exp_lhs);

    exp_type exp_rhs {0};
    detail::normalize(sig_rhs, exp_rhs);

    // Now that the rhs has been normalized it is guaranteed to fit into the decimal32 significand type
    const auto final_sig_rhs {static_cast<typename detail::decimal32_components::significand_type>(detail::make_positive_unsigned(sig_rhs))};

    return detail::d32_add_impl<decimal32>(sig_lhs, exp_lhs, lhs.isneg(),
                                           final_sig_rhs, exp_rhs, (rhs < 0),
                                           abs_lhs_bigger);
}

template <typename Integer>
constexpr auto operator+(Integer lhs, decimal32 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32)
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
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32&)
{
    *this = *this + rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal32::operator+=(Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal32&)
{
    *this = *this + rhs;
    return *this;
}

// NOLINTNEXTLINE : If subtraction is actually addition than use operator+ and vice versa
constexpr auto operator-(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    constexpr decimal32 zero {0, 0};

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
    detail::normalize(sig_lhs, exp_lhs);

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize(sig_rhs, exp_rhs);

    return detail::sub_impl<decimal32>(sig_lhs, exp_lhs, lhs.isneg(),
                                       sig_rhs, exp_rhs, rhs.isneg(),
                                       abs_lhs_bigger);
}

template <typename Integer>
constexpr auto operator-(decimal32 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32)
{
    using promoted_significand_type = detail::promote_significand_t<decimal32, Integer>;
    using exp_type = decimal32::biased_exponent_type;

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
    detail::normalize(sig_lhs, exp_lhs);

    exp_type exp_rhs {0};
    detail::normalize(sig_rhs, exp_rhs);
    auto final_sig_rhs {static_cast<decimal32::significand_type>(sig_rhs)};

    return detail::sub_impl<decimal32>(sig_lhs, exp_lhs, lhs.isneg(),
                                       final_sig_rhs, exp_rhs, (rhs < 0),
                                       abs_lhs_bigger);
}

template <typename Integer>
constexpr auto operator-(Integer lhs, decimal32 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32)
{
    using promoted_significand_type = detail::promote_significand_t<decimal32, Integer>;
    using exp_type = decimal32::biased_exponent_type;

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
    detail::normalize(sig_lhs, exp_lhs);
    auto final_sig_lhs {static_cast<decimal32::significand_type>(sig_lhs)};

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize(sig_rhs, exp_rhs);

    return detail::sub_impl<decimal32>(final_sig_lhs, exp_lhs, (lhs < 0),
                                       sig_rhs, exp_rhs, rhs.isneg(),
                                       abs_lhs_bigger);
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
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal32&)
{
    *this = *this - rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal32::operator-=(Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32&)
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
constexpr auto operator==(decimal32 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return mixed_equality_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator==(Integer lhs, decimal32 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return mixed_equality_impl(rhs, lhs);
}

constexpr auto operator!=(decimal32 lhs, decimal32 rhs) noexcept -> bool
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator!=(decimal32 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator!=(Integer lhs, decimal32 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return !(lhs == rhs);
}

constexpr auto operator<(decimal32 lhs, decimal32 rhs) noexcept -> bool
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

    return less_parts_impl(lhs.full_significand(), lhs.biased_exponent(), lhs.isneg(),
                           rhs.full_significand(), rhs.biased_exponent(), rhs.isneg());
}

template <typename Integer>
constexpr auto operator<(decimal32 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return less_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator<(Integer lhs, decimal32 rhs) noexcept
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

constexpr auto operator<=(decimal32 lhs, decimal32 rhs) noexcept -> bool
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
constexpr auto operator<=(decimal32 lhs, Integer rhs) noexcept
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
constexpr auto operator<=(Integer lhs, decimal32 rhs) noexcept
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

constexpr auto operator>(decimal32 lhs, decimal32 rhs) noexcept -> bool
{
    return rhs < lhs;
}

template <typename Integer>
constexpr auto operator>(decimal32 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs))
    {
        return false;
    }
    #endif

    return rhs < lhs;
}

template <typename Integer>
constexpr auto operator>(Integer lhs, decimal32 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(rhs))
    {
        return false;
    }
    #endif

    return rhs < lhs;
}

constexpr auto operator>=(decimal32 lhs, decimal32 rhs) noexcept -> bool
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
constexpr auto operator>=(decimal32 lhs, Integer rhs) noexcept
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
constexpr auto operator>=(Integer lhs, decimal32 rhs) noexcept
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
constexpr auto operator<=>(decimal32 lhs, Integer rhs) noexcept
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
constexpr auto operator<=>(Integer lhs, decimal32 rhs) noexcept
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

constexpr auto decimal32::unbiased_exponent() const noexcept -> exponent_type
{
    exponent_type expval {};

    const auto exp_comb_bits {(bits_ & detail::d32_comb_11_mask)};

    switch (exp_comb_bits)
    {
        case detail::d32_comb_11_mask:
            // bits 2 and 3 are the exp part of the combination field
            expval = (bits_ & detail::d32_comb_11_exp_bits) >> (detail::d32_significand_bits + 1);
            break;
        case detail::d32_comb_10_mask:
            expval = UINT32_C(0b10000000);
            break;
        case detail::d32_comb_01_mask:
            expval = UINT32_C(0b01000000);
            break;
    }

    expval |= (bits_ & detail::d32_exponent_mask) >> detail::d32_significand_bits;

    return expval;
}

constexpr auto decimal32::biased_exponent() const noexcept -> biased_exponent_type
{
    return static_cast<biased_exponent_type>(unbiased_exponent()) - detail::bias;
}

template <typename T>
constexpr auto decimal32::edit_exponent(T expval) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, T, void)
{
    *this = decimal32(this->full_significand(), expval, this->isneg());
}

constexpr auto decimal32::full_significand() const noexcept -> significand_type
{
    significand_type significand {};

    if ((bits_ & detail::d32_comb_11_mask) == detail::d32_comb_11_mask)
    {
        // Only need the one bit of T because the other 3 are implied
        significand = (bits_ & detail::d32_comb_11_significand_bits) == detail::d32_comb_11_significand_bits ?
                      UINT32_C(0b1001'0000000000'0000000000) :
                      UINT32_C(0b1000'0000000000'0000000000);
    }
    else
    {
        // Last three bits in the combination field, so we need to shift past the exp field
        // which is next
        significand |= (bits_ & detail::d32_comb_00_01_10_significand_bits) >> detail::d32_exponent_bits;
    }

    significand |= (bits_ & detail::d32_significand_mask);

    return significand;
}

template <typename T>
constexpr auto decimal32::edit_significand(T sig) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, T, void)
{
    *this = decimal32(sig, this->biased_exponent(), this->isneg());
}

constexpr auto decimal32::isneg() const noexcept -> bool
{
    return static_cast<bool>(bits_ & detail::d32_sign_mask);
}

// Allows changing the sign even on nans and infs
constexpr auto decimal32::edit_sign(bool sign) noexcept -> void
{
    if (sign)
    {
        bits_ |= detail::d32_sign_mask;
    }
    else
    {
        bits_ &= ~detail::d32_sign_mask;
    }
}

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
BOOST_DECIMAL_CXX20_CONSTEXPR decimal32::decimal32(Float val) noexcept
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (val != val)
    {
        *this = boost::decimal::from_bits(boost::decimal::detail::d32_nan_mask);
    }
    else if (val == std::numeric_limits<Float>::infinity() || val == -std::numeric_limits<Float>::infinity())
    {
        *this = boost::decimal::from_bits(boost::decimal::detail::d32_inf_mask);
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

        #ifndef BOOST_DECIMAL_FAST_MATH
        if (components.exponent > detail::emax)
        {
            *this = boost::decimal::from_bits(boost::decimal::detail::d32_inf_mask);
        }
        else
        #endif
        {
            *this = decimal32 {components.mantissa, components.exponent, components.sign};
        }
    }
}

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

template <typename Float>
BOOST_DECIMAL_CXX20_CONSTEXPR auto decimal32::operator=(const Float& val) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_floating_point_v, Float, decimal32&)
{
    *this = decimal32{val};
    return *this;
}

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal>
#else
template <typename Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool>>
#endif
constexpr decimal32::decimal32(Decimal val) noexcept
{
    *this = to_decimal<decimal32, Decimal>(val);
}

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_INTEGRAL Integer>
#else
template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool>>
#endif
constexpr decimal32::decimal32(Integer val) noexcept // NOLINT : Incorrect parameter is never used
{
    using ConversionType = std::conditional_t<std::is_same<Integer, bool>::value, std::int32_t, Integer>;
    *this = decimal32{static_cast<ConversionType>(val), 0};
}

template <typename Integer>
constexpr auto decimal32::operator=(const Integer& val) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32&)
{
    using ConversionType = std::conditional_t<std::is_same<Integer, bool>::value, std::int32_t, Integer>;
    *this = decimal32{static_cast<ConversionType>(val), 0};
    return *this;
}

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_INTEGRAL T>
#else
template <typename T, std::enable_if_t<detail::is_integral_v<T>, bool>>
#endif
constexpr decimal32::decimal32(bool coeff, T exp, bool sign) noexcept
{
    *this = decimal32(static_cast<std::int32_t>(coeff), exp, sign);
}

constexpr decimal32::operator bool() const noexcept
{
    constexpr decimal32 zero {0, 0};
    return *this != zero;
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

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr decimal32::operator detail::int128_t() const noexcept
{
    return to_integral<decimal32, detail::int128_t>(*this);
}

constexpr decimal32::operator detail::uint128_t() const noexcept
{
    return to_integral<decimal32, detail::uint128_t>(*this);
}

#endif

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool>>
constexpr decimal32::operator Decimal() const noexcept
{
    return to_decimal<Decimal>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR auto to_bits(decimal32 rhs) noexcept -> std::uint32_t
{
    const auto bits {detail::bit_cast<std::uint32_t>(rhs.bits_)};
    return bits;
}

constexpr auto operator*(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    constexpr decimal32 zero {0, 0};

    const auto res {detail::check_non_finite(lhs, rhs)};
    if (res != zero)
    {
        return res;
    }
    #endif

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize(sig_lhs, exp_lhs);

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize(sig_rhs, exp_rhs);

    return detail::mul_impl<decimal32>(sig_lhs, exp_lhs, lhs.isneg(),
                                       sig_rhs, exp_rhs, rhs.isneg());
}

template <typename Integer>
constexpr auto operator*(decimal32 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32)
{
    using promoted_significand_type = detail::promote_significand_t<decimal32, Integer>;
    using exp_type = decimal32::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs) || isinf(lhs))
    {
        return lhs;
    }
    #endif

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize(sig_lhs, exp_lhs);

    auto sig_rhs {static_cast<promoted_significand_type>(detail::make_positive_unsigned(rhs))};
    exp_type exp_rhs {0};
    detail::normalize(sig_rhs, exp_rhs);
    auto final_sig_rhs {static_cast<decimal32::significand_type>(sig_rhs)};

    return detail::mul_impl<decimal32>(sig_lhs, exp_lhs, lhs.isneg(),
                                       final_sig_rhs, exp_rhs, (rhs < 0));
}

template <typename Integer>
constexpr auto operator*(Integer lhs, decimal32 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32)
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
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32&)
{
    *this = *this * rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal32::operator*=(Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal32&)
{
    *this = *this * rhs;
    return *this;
}

constexpr auto div_impl(decimal32 lhs, decimal32 rhs, decimal32& q, decimal32& r) noexcept -> void
{
    #ifndef BOOST_DECIMAL_FAST_MATH
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
    #else
    static_cast<void>(r);
    #endif

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

    q = detail::generic_div_impl<decimal32>(lhs_components, rhs_components);
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
constexpr auto operator/(decimal32 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32)
{
    using exp_type = decimal32::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
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
    #endif

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize(sig_lhs, exp_lhs);

    detail::decimal32_components lhs_components {sig_lhs, exp_lhs, lhs.isneg()};
    exp_type exp_rhs {};
    detail::decimal32_components rhs_components {detail::shrink_significand(detail::make_positive_unsigned(rhs), exp_rhs), exp_rhs, rhs < 0};

    return detail::generic_div_impl<decimal32>(lhs_components, rhs_components);
}

template <typename Integer>
constexpr auto operator/(Integer lhs, decimal32 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32)
{
    using exp_type = decimal32::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
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
    #endif

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize(sig_rhs, exp_rhs);

    exp_type lhs_exp {};
    auto lhs_sig {detail::make_positive_unsigned(detail::shrink_significand(lhs, lhs_exp))};
    detail::decimal32_components lhs_components {lhs_sig, lhs_exp, lhs < 0};
    detail::decimal32_components rhs_components {sig_rhs, exp_rhs, rhs.isneg()};

    return detail::generic_div_impl<decimal32>(lhs_components, rhs_components);
}

constexpr auto decimal32::operator/=(decimal32 rhs) noexcept -> decimal32&
{
    *this = *this / rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal32::operator/=(Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32&)
{
    *this = *this / rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal32::operator/=(Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal32&)
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
inline auto debug_pattern(decimal32 rhs) noexcept -> void
{
    #if !defined(BOOST_DECIMAL_DISABLE_IOSTREAM)
    std::cerr << "Sig: " << rhs.full_significand()
              << "\nExp: " << rhs.biased_exponent()
              << "\nNeg: " << rhs.isneg() << std::endl;
    #else
    static_cast<void>(rhs);
    #endif
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

constexpr auto operator&(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
{
    return from_bits(lhs.bits_ & rhs.bits_);
}

template <typename Integer>
constexpr auto operator&(decimal32 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32)
{
    return from_bits(lhs.bits_ & static_cast<std::uint32_t>(rhs));
}

template <typename Integer>
constexpr auto operator&(Integer lhs, decimal32 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32)
{
    return from_bits(static_cast<std::uint32_t>(lhs) & rhs.bits_);
}

constexpr auto operator|(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
{
    return from_bits(lhs.bits_ | rhs.bits_);
}

template <typename Integer>
constexpr auto operator|(decimal32 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32)
{
    return from_bits(lhs.bits_ | static_cast<std::uint32_t>(rhs));
}

template <typename Integer>
constexpr auto operator|(Integer lhs, decimal32 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32)
{
    return from_bits(static_cast<std::uint32_t>(lhs) | rhs.bits_);
}

constexpr auto operator^(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
{
    return from_bits(lhs.bits_ ^ rhs.bits_);
}

template <typename Integer>
constexpr auto operator^(decimal32 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32)
{
    return from_bits(lhs.bits_ ^ static_cast<std::uint32_t>(rhs));
}

template <typename Integer>
constexpr auto operator^(Integer lhs, decimal32 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32)
{
    return from_bits(static_cast<std::uint32_t>(lhs) ^ rhs.bits_);
}

constexpr auto operator<<(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
{
    return from_bits(lhs.bits_ << rhs.bits_);
}

template <typename Integer>
constexpr auto operator<<(decimal32 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32)
{
    return from_bits(lhs.bits_ << static_cast<std::uint32_t>(rhs));
}

template <typename Integer>
constexpr auto operator<<(Integer lhs, decimal32 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32)
{
    return from_bits(static_cast<std::uint32_t>(lhs) << rhs.bits_);
}

constexpr auto operator>>(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
{
    return from_bits(lhs.bits_ >> rhs.bits_);
}

template <typename Integer>
constexpr auto operator>>(decimal32 lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32)
{
    return from_bits(lhs.bits_ >> static_cast<std::uint32_t>(rhs));
}

template <typename Integer>
constexpr auto operator>>(Integer lhs, decimal32 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32)
{
    return from_bits(static_cast<std::uint32_t>(lhs) >> rhs.bits_);
}

constexpr auto operator~(decimal32 lhs) noexcept -> decimal32
{
    return from_bits(~lhs.bits_);
}

// 3.6.4
// Effects: determines if the quantum exponents of x and y are the same.
// If both x and y are NaN, or infinity, they have the same quantum exponents;
// if exactly one operand is infinity or exactly one operand is NaN, they do not have the same quantum exponents.
// The samequantum functions raise no exception.
constexpr auto samequantumd32(decimal32 lhs, decimal32 rhs) noexcept -> bool
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
constexpr auto quantexpd32(decimal32 x) noexcept -> int
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
constexpr auto quantized32(decimal32 lhs, decimal32 rhs) noexcept -> decimal32
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
        return boost::decimal::from_bits(boost::decimal::detail::d32_snan_mask);
    }
    else if (isinf(lhs) && isinf(rhs))
    {
        return lhs;
    }
    #endif

    return {lhs.full_significand(), rhs.biased_exponent(), lhs.isneg()};
}

constexpr auto scalblnd32(decimal32 num, long exp) noexcept -> decimal32
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    constexpr decimal32 zero {0, 0};

    if (num == zero || exp == 0 || isinf(num) || isnan(num))
    {
        return num;
    }
    #endif

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

} // namespace decimal
} // namespace boost

namespace std {

BOOST_DECIMAL_EXPORT template <>
#ifdef _MSC_VER
class numeric_limits<boost::decimal::decimal32>
#else
struct numeric_limits<boost::decimal::decimal32>
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
