// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_decimal32_t_HPP
#define BOOST_DECIMAL_decimal32_t_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/bit_cast.hpp>
#include <boost/decimal/detail/config.hpp>
#include "detail/int128.hpp"
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
#include <boost/decimal/detail/components.hpp>
#include <boost/decimal/detail/cmath/next.hpp>
#include <boost/decimal/detail/to_chars_result.hpp>
#include <boost/decimal/detail/chars_format.hpp>

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

namespace boost {
namespace decimal {

namespace detail {

// See IEEE 754 section 3.5.2
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_inf_mask  = UINT32_C(0x78000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_nan_mask  = UINT32_C(0x7C000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_snan_mask = UINT32_C(0x7E000000);

//    Comb.  Exponent          Significand
// s         eeeeeeee     ttttttttttttttttttttttt - sign + 2 steering bits concatenate to 6 bits of exponent (8 total) + 23 bits of significand like float
// s   11    eeeeeeee    [100] + ttttttttttttttttttttt - sign + 2 steering bits + 8 bits of exponent + 21 bits of significand (0b100 + 21 bits)
//
// Only is the type different in steering 11 which yields significand 100 + 21 bits giving us our 24 total bits of precision

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_sign_mask = UINT32_C(0b10000000000000000000000000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_combination_field_mask = UINT32_C(0b01100000000000000000000000000000);

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_comb_11_mask = UINT32_C(0b0'11000'000000'0000000000'0000000000);

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_not_11_exp_mask = UINT32_C(0b01111111100000000000000000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_not_11_exp_shift = UINT32_C(23);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_11_exp_mask = UINT32_C(0b00011111111000000000000000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_11_exp_shift = UINT32_C(21);

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_not_11_significand_mask = UINT32_C(0b00000000011111111111111111111111);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_11_significand_mask = UINT32_C(0b00000000000111111111111111111111);

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_biggest_no_combination_significand = UINT32_C(0b11111111111111111111111); // 23 bits

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_max_biased_exponent = UINT32_C(191);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint32_t d32_max_significand_value = UINT32_C(9'999'999);

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
constexpr auto to_chars_scientific_impl(char* first, char* last, const TargetDecimalType& value, chars_format fmt) noexcept -> to_chars_result;

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
constexpr auto to_chars_fixed_impl(char* first, char* last, const TargetDecimalType& value, const chars_format fmt) noexcept -> to_chars_result;

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
constexpr auto to_chars_hex_impl(char* first, char* last, const TargetDecimalType& value) noexcept -> to_chars_result;

} // namespace detail

#if defined(__GNUC__) && __GNUC__ >= 8
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif


// ISO/IEC DTR 24733
// 3.2.2 class decimal32_t
BOOST_DECIMAL_EXPORT class decimal32_t final // NOLINT(cppcoreguidelines-special-member-functions,hicpp-special-member-functions)
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

    // Returns a complete struct so we don't have to decode the number multiple times if we need everything
    constexpr auto to_components() const noexcept -> detail::decimal32_t_components;

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

    friend constexpr auto div_impl(decimal32_t lhs, decimal32_t rhs, decimal32_t& q, decimal32_t& r) noexcept -> void;
    friend constexpr auto mod_impl(decimal32_t lhs, decimal32_t rhs, const decimal32_t& q, decimal32_t& r) noexcept -> void;

    template <typename T>
    friend constexpr auto ilogb(T d) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, T, int);

    template <typename T>
    friend constexpr auto logb(T num) noexcept
        BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T);

    // Debug bit pattern
    friend constexpr auto from_bits(std::uint32_t bits) noexcept -> decimal32_t;
    friend BOOST_DECIMAL_CXX20_CONSTEXPR auto to_bits(decimal32_t rhs) noexcept -> std::uint32_t;
    friend inline auto debug_pattern(decimal32_t rhs) noexcept -> void;

    // Equality template between any integer type and decimal32_t
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, BOOST_DECIMAL_INTEGRAL Integer>
    friend constexpr auto mixed_equality_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
    friend constexpr auto mixed_decimal_equality_impl(Decimal1 lhs, Decimal2 rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                             detail::is_decimal_floating_point_v<Decimal2>), bool>;

    // Template to compare operator< for any integer type and decimal32_t
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, BOOST_DECIMAL_INTEGRAL Integer>
    friend constexpr auto less_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
    friend constexpr auto mixed_decimal_less_impl(Decimal1 lhs, Decimal2 rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                             detail::is_decimal_floating_point_v<Decimal2>), bool>;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
    friend constexpr auto equality_impl(DecimalType lhs, DecimalType rhs) noexcept -> bool;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
    friend constexpr auto sequential_less_impl(DecimalType lhs, DecimalType rhs) noexcept -> bool;

    friend constexpr auto to_bid_d32(decimal32_t val) noexcept -> std::uint32_t;

    friend constexpr auto from_bid_d32(std::uint32_t bits) noexcept -> decimal32_t;

    template <typename DecimalType>
    friend constexpr auto to_dpd_d32(DecimalType val) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, DecimalType, std::uint32_t);

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
    friend constexpr auto detail::nextafter_impl(DecimalType val, bool direction) noexcept -> DecimalType;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
    friend constexpr auto detail::to_chars_scientific_impl(char* first, char* last, const TargetDecimalType& value, chars_format fmt) noexcept -> to_chars_result;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
    friend constexpr auto detail::to_chars_fixed_impl(char* first, char* last, const TargetDecimalType& value, const chars_format fmt) noexcept -> to_chars_result;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
    friend constexpr auto detail::to_chars_hex_impl(char* first, char* last, const TargetDecimalType& value) noexcept -> to_chars_result;

public:
    // 3.2.2.1 construct/copy/destroy:
    constexpr decimal32_t() noexcept = default;

    // 3.2.2.2 Conversion from a floating-point type
    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_REAL Float>
    #else
    template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool> = true>
    #endif
    #ifndef BOOST_DECIMAL_ALLOW_IMPLICIT_CONVERSIONS
    explicit
    #endif
    BOOST_DECIMAL_CXX20_CONSTEXPR decimal32_t(Float val) noexcept;

    template <typename Float>
    BOOST_DECIMAL_CXX20_CONSTEXPR auto operator=(const Float& val) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_floating_point_v, Float, decimal32_t&);

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal>
    #else
    template <typename Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool> = true>
    #endif
    explicit constexpr decimal32_t(Decimal val) noexcept;

    // 3.2.2.3 Conversion from integral type
    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_INTEGRAL Integer>
    #else
    template <BOOST_DECIMAL_INTEGRAL Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool> = true>
    #endif
    constexpr decimal32_t(Integer val) noexcept;

    template <typename Integer>
    constexpr auto operator=(const Integer& val) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t&);

    // 3.2.2.4 Conversion to integral type
    explicit constexpr operator bool() const noexcept;
    explicit constexpr operator int() const noexcept;
    explicit constexpr operator unsigned() const noexcept;
    explicit constexpr operator long() const noexcept;
    explicit constexpr operator unsigned long() const noexcept;
    explicit constexpr operator long long() const noexcept;
    explicit constexpr operator unsigned long long() const noexcept;

    #ifdef BOOST_DECIMAL_HAS_INT128
    explicit constexpr operator detail::builtin_int128_t() const noexcept;
    explicit constexpr operator detail::builtin_uint128_t() const noexcept;
    #endif

    // We allow implict promotions to and decimal type with greater or equal precision (e.g. decimal_fast32_t)
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool> = true>
    constexpr operator Decimal() const noexcept;

    // 3.2.5 initialization from coefficient and exponent:
    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_UNSIGNED_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
    #else
    template <typename T1, typename T2, std::enable_if_t<detail::is_unsigned_v<T1> && detail::is_integral_v<T2>, bool> = true>
    #endif
    constexpr decimal32_t(T1 coeff, T2 exp, bool sign = false) noexcept;

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_SIGNED_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
    #else
    template <typename T1, typename T2, std::enable_if_t<!detail::is_unsigned_v<T1> && detail::is_integral_v<T2>, bool> = true>
    #endif
    constexpr decimal32_t(T1 coeff, T2 exp) noexcept;

    explicit constexpr decimal32_t(bool value) noexcept;

    constexpr decimal32_t(const decimal32_t& val) noexcept = default;
    constexpr decimal32_t(decimal32_t&& val) noexcept = default;
    constexpr auto operator=(const decimal32_t& val) noexcept -> decimal32_t& = default;
    constexpr auto operator=(decimal32_t&& val) noexcept -> decimal32_t& = default;

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
    friend constexpr auto signbit     BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32_t rhs) noexcept -> bool;
    friend constexpr auto isinf       BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32_t rhs) noexcept -> bool;
    friend constexpr auto isnan       BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32_t rhs) noexcept -> bool;
    friend constexpr auto issignaling BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32_t rhs) noexcept -> bool;
    friend constexpr auto isnormal    BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32_t rhs) noexcept -> bool;
    friend constexpr auto isfinite    BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal32_t rhs) noexcept -> bool;

    // 3.2.7 unary arithmetic operators:
    friend constexpr auto operator+(decimal32_t rhs) noexcept -> decimal32_t;
    friend constexpr auto operator-(decimal32_t rhs) noexcept -> decimal32_t;

    // 3.2.8 binary arithmetic operators:
    friend constexpr auto operator+(decimal32_t lhs, decimal32_t rhs) noexcept -> decimal32_t;

    template <typename Integer>
    friend constexpr auto operator+(decimal32_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t);

    template <typename Integer>
    friend constexpr auto operator+(Integer lhs, decimal32_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t);

    friend constexpr auto operator-(decimal32_t lhs, decimal32_t rhs) noexcept -> decimal32_t;

    template <typename Integer>
    friend constexpr auto operator-(decimal32_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t);

    template <typename Integer>
    friend constexpr auto operator-(Integer lhs, decimal32_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t);

    friend constexpr auto operator*(decimal32_t lhs, decimal32_t rhs) noexcept -> decimal32_t;

    template <typename Integer>
    friend constexpr auto operator*(decimal32_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t);

    template <typename Integer>
    friend constexpr auto operator*(Integer lhs, decimal32_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t);

    friend constexpr auto operator/(decimal32_t lhs, decimal32_t rhs) noexcept -> decimal32_t;

    template <typename Integer>
    friend constexpr auto operator/(decimal32_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t);

    template <typename Integer>
    friend constexpr auto operator/(Integer lhs, decimal32_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t);

    friend constexpr auto operator%(decimal32_t lhs, decimal32_t rhs) noexcept -> decimal32_t;

    // 3.2.2.5 Increment and Decrement
    constexpr auto operator++()    noexcept -> decimal32_t&;
    constexpr auto operator++(int) noexcept -> decimal32_t;  // NOLINT : C++14 so constexpr implies const
    constexpr auto operator--()    noexcept -> decimal32_t&;
    constexpr auto operator--(int) noexcept -> decimal32_t;  // NOLINT : C++14 so constexpr implies const

    // 3.2.2.6 Compound assignment
    constexpr auto operator+=(decimal32_t rhs) noexcept -> decimal32_t&;

    template <typename Integer>
    constexpr auto operator+=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t&);

    template <typename Decimal>
    constexpr auto operator+=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal32_t&);

    constexpr auto operator-=(decimal32_t rhs) noexcept -> decimal32_t&;

    template <typename Integer>
    constexpr auto operator-=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t&);

    template <typename Decimal>
    constexpr auto operator-=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal32_t&);

    constexpr auto operator*=(decimal32_t rhs) noexcept -> decimal32_t&;

    template <typename Integer>
    constexpr auto operator*=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t&);

    template <typename Decimal>
    constexpr auto operator*=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal32_t&);

    constexpr auto operator/=(decimal32_t rhs) noexcept -> decimal32_t&;

    template <typename Integer>
    constexpr auto operator/=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t&);

    template <typename Decimal>
    constexpr auto operator/=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal32_t&);

    constexpr auto operator%=(decimal32_t rhs) noexcept -> decimal32_t&;

    // 3.2.9 comparison operators:
    // Equality
    friend constexpr auto operator==(decimal32_t lhs, decimal32_t rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator==(decimal32_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator==(Integer lhs, decimal32_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Inequality
    friend constexpr auto operator!=(decimal32_t lhs, decimal32_t rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator!=(decimal32_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator!=(Integer lhs, decimal32_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Less
    friend constexpr auto operator<(decimal32_t lhs, decimal32_t rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator<(decimal32_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator<(Integer lhs, decimal32_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Less equal
    friend constexpr auto operator<=(decimal32_t lhs, decimal32_t rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator<=(decimal32_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator<=(Integer lhs, decimal32_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Greater
    friend constexpr auto operator>(decimal32_t lhs, decimal32_t rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator>(decimal32_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator>(Integer lhs, decimal32_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Greater equal
    friend constexpr auto operator>=(decimal32_t lhs, decimal32_t rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator>=(decimal32_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator>=(Integer lhs, decimal32_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    #ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR
    friend constexpr auto operator<=>(decimal32_t lhs, decimal32_t rhs) noexcept -> std::partial_ordering;

    template <typename Integer>
    friend constexpr auto operator<=>(decimal32_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering);

    template <typename Integer>
    friend constexpr auto operator<=>(Integer lhs, decimal32_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering);
    #endif

    // Bitwise operators
    friend constexpr auto operator&(decimal32_t lhs, decimal32_t rhs) noexcept -> decimal32_t;

    template <typename Integer>
    friend constexpr auto operator&(decimal32_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t);

    template <typename Integer>
    friend constexpr auto operator&(Integer lhs, decimal32_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t);

    friend constexpr auto operator|(decimal32_t lhs, decimal32_t rhs) noexcept -> decimal32_t;

    template <typename Integer>
    friend constexpr auto operator|(decimal32_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t);

    template <typename Integer>
    friend constexpr auto operator|(Integer lhs, decimal32_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t);

    friend constexpr auto operator^(decimal32_t lhs, decimal32_t rhs) noexcept -> decimal32_t;

    template <typename Integer>
    friend constexpr auto operator^(decimal32_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t);

    template <typename Integer>
    friend constexpr auto operator^(Integer lhs, decimal32_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t);

    friend constexpr auto operator<<(decimal32_t lhs, decimal32_t rhs) noexcept -> decimal32_t;

    template <typename Integer>
    friend constexpr auto operator<<(decimal32_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t);

    template <typename Integer>
    friend constexpr auto operator<<(Integer lhs, decimal32_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t);

    friend constexpr auto operator>>(decimal32_t lhs, decimal32_t rhs) noexcept -> decimal32_t;

    template <typename Integer>
    friend constexpr auto operator>>(decimal32_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t);

    template <typename Integer>
    friend constexpr auto operator>>(Integer lhs, decimal32_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t);

    friend constexpr auto operator~(decimal32_t lhs) noexcept -> decimal32_t;

    // <cmath> extensions
    // 3.6.4 Same Quantum
    friend constexpr auto samequantumd32(decimal32_t lhs, decimal32_t rhs) noexcept -> bool;

    // 3.6.5 Quantum exponent
    friend constexpr auto quantexpd32(decimal32_t x) noexcept -> int;

    // 3.6.6 Quantize
    friend constexpr auto quantized32(decimal32_t lhs, decimal32_t rhs) noexcept -> decimal32_t;

    // <cmath> functions that need to be friends
    friend constexpr auto copysignd32(decimal32_t mag, decimal32_t sgn) noexcept -> decimal32_t;
    friend constexpr auto fmad32(decimal32_t x, decimal32_t y, decimal32_t z) noexcept -> decimal32_t;

    // Related to <cmath>
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
    friend constexpr auto frexp10(T num, int* expptr) noexcept -> typename T::significand_type;

    friend constexpr auto scalbnd32(decimal32_t num, int exp) noexcept -> decimal32_t;
    friend constexpr auto scalblnd32(decimal32_t num, long exp) noexcept -> decimal32_t;

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

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4127)
#endif

#if defined(__GNUC__) && __GNUC__ >= 8
#  pragma GCC diagnostic pop
#endif

#if defined(__GNUC__) && __GNUC__ >= 6
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wduplicated-branches"
#  pragma GCC diagnostic ignored "-Wbool-compare"
#  pragma GCC diagnostic ignored "-Wconversion"
#endif

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_UNSIGNED_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
#else
template <typename T1, typename T2, std::enable_if_t<detail::is_unsigned_v<T1> && detail::is_integral_v<T2>, bool>>
#endif
constexpr decimal32_t::decimal32_t(T1 coeff, T2 exp, bool sign) noexcept // NOLINT(readability-function-cognitive-complexity,misc-no-recursion)
{
    static_assert(detail::is_integral_v<T1>, "Coefficient must be an integer");
    static_assert(detail::is_integral_v<T2>, "Exponent must be an integer");

    bits_ = sign ? detail::d32_sign_mask : UINT32_C(0);

    // If the coeff is not in range, make it so
    // Only count the number of digits if we absolutely have to
    int coeff_digits {-1};
    if (coeff > detail::d32_max_significand_value)
    {
        bool sticky_bit {detail::find_sticky_bit(coeff, exp, detail::bias_v<decimal32_t>)};

        if (!sticky_bit)
        {
            coeff_digits = detail::num_digits(coeff);
            if (coeff_digits > detail::precision + 1)
            {
                const auto digits_to_remove {coeff_digits - (detail::precision + 1)};

                #if defined(__GNUC__) && !defined(__clang__)
                #  pragma GCC diagnostic push
                #  pragma GCC diagnostic ignored "-Wconversion"
                #endif

                if (coeff % detail::pow10(static_cast<T1>(digits_to_remove)) != 0u)
                {
                    sticky_bit = true;
                }
                coeff /= detail::pow10(static_cast<T1>(digits_to_remove));

                #if defined(__GNUC__) && !defined(__clang__)
                #  pragma GCC diagnostic pop
                #endif

                coeff_digits -= digits_to_remove;
                exp += static_cast<T2>(detail::fenv_round(coeff, sign, sticky_bit)) + digits_to_remove;
            }
            else
            {
                exp += static_cast<T2>(detail::fenv_round(coeff, sign, sticky_bit));
            }
        }
        else
        {
            // This should already be handled in find_sticky_bit
            BOOST_DECIMAL_ASSERT((coeff >= 1'000'000U && coeff <= 9'999'999U) || coeff == 0U);
            exp += static_cast<T2>(detail::fenv_round(coeff, sign, sticky_bit));
        }
    }

    auto reduced_coeff {static_cast<significand_type>(coeff)};
    bool big_combination {false};

    if (reduced_coeff == 0U)
    {
        // Normalize our handling of zeros
        return;
    }

    if (reduced_coeff <= detail::d32_biggest_no_combination_significand)
    {
        // If the coefficient fits directly, we don't need to use the combination field
        // bits_.significand = reduced_coeff;
        bits_ |= (reduced_coeff & detail::d32_not_11_significand_mask);
    }
    else
    {
        // Have to use the full combination field
        bits_ |= (detail::d32_comb_11_mask | (reduced_coeff & detail::d32_11_significand_mask));
        big_combination = true;
    }

    // If the exponent fits we do not need to use the combination field
    const auto biased_exp {static_cast<std::int32_t>(exp + detail::bias)};
    if (BOOST_DECIMAL_LIKELY(biased_exp >= 0 && biased_exp <= detail::d32_max_biased_exponent))
    {
        if (big_combination)
        {
            bits_ |= (biased_exp << detail::d32_11_exp_shift) & detail::d32_11_exp_mask;
        }
        else
        {
            bits_ |= (biased_exp << detail::d32_not_11_exp_shift) & detail::d32_not_11_exp_mask;
        }
    }
    else
    {
        // If we can fit the extra exponent in the significand, then we can construct the value
        // If we can't, the value is either 0 or infinity depending on the sign of exp

        if (coeff_digits == -1)
        {
            coeff_digits = detail::num_digits(reduced_coeff);
        }

        const auto exp_delta {biased_exp - detail::d32_max_biased_exponent};
        const auto digit_delta {coeff_digits - static_cast<int>(exp_delta)};
        if (digit_delta > 0 && coeff_digits + digit_delta <= detail::precision)
        {
            exp -= digit_delta;
            reduced_coeff *= detail::pow10(static_cast<significand_type>(digit_delta));
            *this = decimal32_t(reduced_coeff, exp, sign);
        }
        else
        {
            bits_ = exp < 0 ? UINT32_C(0) : detail::d32_inf_mask;
        }
    }
}

#ifdef _MSC_VER
#  pragma warning(pop)
#endif

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_SIGNED_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
#else
template <typename T1, typename T2, std::enable_if_t<!detail::is_unsigned_v<T1> && detail::is_integral_v<T2>, bool>>
#endif
constexpr decimal32_t::decimal32_t(const T1 coeff, const T2 exp) noexcept : decimal32_t(detail::make_positive_unsigned(coeff), exp, coeff < 0) {}

#if defined(__GNUC__) && __GNUC__ >= 6
#  pragma GCC diagnostic pop
#endif

constexpr decimal32_t::decimal32_t(const bool value) noexcept : decimal32_t(static_cast<significand_type>(value), 0, false) {}

constexpr auto from_bits(const std::uint32_t bits) noexcept -> decimal32_t
{
    decimal32_t result;
    result.bits_ = bits;

    return result;
}

constexpr auto signbit BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (const decimal32_t rhs) noexcept -> bool
{
    return rhs.bits_ & detail::d32_sign_mask;
}

constexpr auto isnan BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (const decimal32_t rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return (rhs.bits_ & detail::d32_nan_mask) == detail::d32_nan_mask;
    #else
    static_cast<void>(rhs);
    return false;
    #endif
}

constexpr auto issignaling BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (const decimal32_t rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return (rhs.bits_ & detail::d32_snan_mask) == detail::d32_snan_mask;
    #else
    static_cast<void>(rhs);
    return false;
    #endif
}

constexpr auto isinf BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (const decimal32_t rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return ((rhs.bits_ & detail::d32_nan_mask) == detail::d32_inf_mask);
    #else
    static_cast<void>(rhs);
    return false;
    #endif
}

constexpr auto isfinite BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (const decimal32_t rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return ((rhs.bits_ & detail::d32_inf_mask) != detail::d32_inf_mask);
    #else
    static_cast<void>(rhs);
    return false;
    #endif
}

constexpr auto isnormal BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (const decimal32_t rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check for de-normals
    const auto sig {rhs.full_significand()};
    const auto exp {rhs.unbiased_exponent()};

    if (exp <= detail::precision - 1)
    {
        return false;
    }

    return (sig != 0) && isfinite(rhs);
    #else
    return rhs.full_significand() != 0;
    #endif
}

constexpr auto operator+(const decimal32_t rhs) noexcept -> decimal32_t
{
    return rhs;
}

constexpr auto operator-(decimal32_t rhs) noexcept-> decimal32_t
{
    rhs.bits_ ^= detail::d32_sign_mask;
    return rhs;
}

// We use kahan summation here where applicable
// https://en.wikipedia.org/wiki/Kahan_summation_algorithm
// NOLINTNEXTLINE: If addition is actually subtraction than change operator and vice versa
constexpr auto operator+(const decimal32_t lhs, const decimal32_t rhs) noexcept -> decimal32_t
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (!isfinite(lhs) || !isfinite(rhs))
    {
        return detail::check_non_finite(lhs, rhs);
    }
    #endif

    auto lhs_components {lhs.to_components()};
    detail::normalize(lhs_components.sig, lhs_components.exp);
    auto rhs_components {rhs.to_components()};
    detail::normalize(rhs_components.sig, rhs_components.exp);

    return detail::d32_add_impl<decimal32_t>(lhs_components, rhs_components);
}

template <typename Integer>
constexpr auto operator+(const decimal32_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t)
{
    using promoted_significand_type = detail::promote_significand_t<decimal32_t, Integer>;
    using exp_type = decimal32_t::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (!isfinite(lhs))
    {
        return lhs;
    }
    #endif

    // Make the significand type wide enough that it won't overflow during normalization
    auto sig_rhs {static_cast<promoted_significand_type>(detail::make_positive_unsigned(rhs))};

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize(sig_lhs, exp_lhs);

    exp_type exp_rhs {0};
    detail::normalize(sig_rhs, exp_rhs);

    // Now that the rhs has been normalized, it is guaranteed to fit into the decimal32_t significand type
    const auto final_sig_rhs {static_cast<typename detail::decimal32_t_components::significand_type>(detail::make_positive_unsigned(sig_rhs))};

    return detail::d32_add_impl<decimal32_t>(sig_lhs, exp_lhs, lhs.isneg(),
                                           final_sig_rhs, exp_rhs, (rhs < 0));
}

template <typename Integer>
constexpr auto operator+(const Integer lhs, const decimal32_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t)
{
    return rhs + lhs;
}

constexpr auto decimal32_t::operator++() noexcept -> decimal32_t&
{
    constexpr decimal32_t one(1, 0);
    *this = *this + one;
    return *this;
}

constexpr auto decimal32_t::operator++(int) noexcept -> decimal32_t
{
    return ++(*this);
}

constexpr auto decimal32_t::operator+=(const decimal32_t rhs) noexcept -> decimal32_t&
{
    *this = *this + rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal32_t::operator+=(const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t&)
{
    *this = *this + rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal32_t::operator+=(const Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal32_t&)
{
    *this = *this + rhs;
    return *this;
}

// NOLINTNEXTLINE: If subtraction is actually addition than use operator+ and vice versa
constexpr auto operator-(const decimal32_t lhs, const decimal32_t rhs) noexcept -> decimal32_t
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (!isfinite(lhs) || !isfinite(rhs))
    {
        return detail::check_non_finite(lhs, rhs);
    }
    #endif

    auto lhs_components {lhs.to_components()};
    detail::normalize(lhs_components.sig, lhs_components.exp);
    auto rhs_components {rhs.to_components()};
    detail::normalize(rhs_components.sig, rhs_components.exp);

    // a - b = a + (-b)
    rhs_components.sign = !rhs_components.sign;
    return detail::d32_add_impl<decimal32_t>(lhs_components, rhs_components);
}

template <typename Integer>
constexpr auto operator-(const decimal32_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t)
{
    using promoted_significand_type = detail::promote_significand_t<decimal32_t, Integer>;
    using exp_type = decimal32_t::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (!isfinite(lhs))
    {
        return lhs;
    }
    #endif

    auto sig_rhs {static_cast<promoted_significand_type>(detail::make_positive_unsigned(rhs))};

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize(sig_lhs, exp_lhs);

    exp_type exp_rhs {0};
    detail::normalize(sig_rhs, exp_rhs);
    auto final_sig_rhs {static_cast<decimal32_t::significand_type>(sig_rhs)};

    return detail::d32_add_impl<decimal32_t>(sig_lhs, exp_lhs, lhs.isneg(),
                                           final_sig_rhs, exp_rhs, !(rhs < 0));
}

template <typename Integer>
constexpr auto operator-(const Integer lhs, const decimal32_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t)
{
    using promoted_significand_type = detail::promote_significand_t<decimal32_t, Integer>;
    using exp_type = decimal32_t::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (!isfinite(rhs))
    {
        return rhs;
    }
    #endif
    
    auto sig_lhs {static_cast<promoted_significand_type>(detail::make_positive_unsigned(lhs))};

    exp_type exp_lhs {0};
    detail::normalize(sig_lhs, exp_lhs);
    const auto final_sig_lhs {static_cast<decimal32_t::significand_type>(sig_lhs)};

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize(sig_rhs, exp_rhs);

    return detail::d32_add_impl<decimal32_t>(final_sig_lhs, exp_lhs, (lhs < 0),
                                           sig_rhs, exp_rhs, !rhs.isneg());
}

constexpr auto decimal32_t::operator--() noexcept -> decimal32_t&
{
    constexpr decimal32_t one(1, 0);
    *this = *this - one;
    return *this;
}

constexpr auto decimal32_t::operator--(int) noexcept -> decimal32_t
{
    return --(*this);
}

constexpr auto decimal32_t::operator-=(const decimal32_t rhs) noexcept -> decimal32_t&
{
    *this = *this - rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal32_t::operator-=(const Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal32_t&)
{
    *this = *this - rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal32_t::operator-=(const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t&)
{
    *this = *this - rhs;
    return *this;
}

constexpr auto operator==(const decimal32_t lhs, const decimal32_t rhs) noexcept -> bool
{
    return equality_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator==(const decimal32_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return mixed_equality_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator==(const Integer lhs, const decimal32_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return mixed_equality_impl(rhs, lhs);
}

constexpr auto operator!=(const decimal32_t lhs, const decimal32_t rhs) noexcept -> bool
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator!=(const decimal32_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator!=(const Integer lhs, const decimal32_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return !(lhs == rhs);
}

constexpr auto operator<(const decimal32_t lhs, const decimal32_t rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (!isfinite(lhs) || !isfinite(rhs))
    {
        if (isnan(lhs) || isnan(rhs))
        {
            return false;
        }
        else if (isfinite(lhs) && isinf(rhs))
        {
            return !rhs.isneg();
        }
    }
    #endif

    return sequential_less_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator<(const decimal32_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return less_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator<(const Integer lhs, const decimal32_t rhs) noexcept
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

constexpr auto operator<=(const decimal32_t lhs, const decimal32_t rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (!isfinite(lhs) || !isfinite(rhs))
    {
        if (isnan(lhs) || isnan(rhs))
        {
            return false;
        }
        if (isinf(lhs))
        {
            return signbit(lhs);
        }
        if (isinf(rhs))
        {
            return !signbit(rhs);
        }
    }
    #endif

    return !sequential_less_impl(rhs, lhs);
}

template <typename Integer>
constexpr auto operator<=(const decimal32_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (!isfinite(lhs))
    {
        if (isnan(lhs))
        {
            return false;
        }
        if (isinf(lhs))
        {
            return signbit(lhs);
        }
    }
    #endif

    return !(rhs < lhs);
}

template <typename Integer>
constexpr auto operator<=(const Integer lhs, const decimal32_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (!isfinite(rhs))
    {
        if (isnan(rhs))
        {
            return false;
        }
        if (isinf(rhs))
        {
            return !signbit(rhs);
        }
    }
    #endif

    return !(rhs < lhs);
}

constexpr auto operator>(const decimal32_t lhs, const decimal32_t rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (!isfinite(lhs) || !isfinite(rhs))
    {
        if (isnan(lhs) || isnan(rhs))
        {
            return false;
        }
        if (isinf(lhs))
        {
            return !signbit(lhs);
        }
        if (isinf(rhs))
        {
            return signbit(rhs);
        }
    }
    #endif

    return sequential_less_impl(rhs, lhs);
}

template <typename Integer>
constexpr auto operator>(const decimal32_t lhs, const Integer rhs) noexcept
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
constexpr auto operator>(const Integer lhs, const decimal32_t rhs) noexcept
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

constexpr auto operator>=(const decimal32_t lhs, const decimal32_t rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }
    #endif

    return !sequential_less_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator>=(const decimal32_t lhs, const Integer rhs) noexcept
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
constexpr auto operator>=(const Integer lhs, const decimal32_t rhs) noexcept
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

constexpr auto operator<=>(const decimal32_t lhs, const decimal32_t rhs) noexcept -> std::partial_ordering
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
constexpr auto operator<=>(const decimal32_t lhs, const Integer rhs) noexcept
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
constexpr auto operator<=>(const Integer lhs, const decimal32_t rhs) noexcept
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

constexpr auto decimal32_t::unbiased_exponent() const noexcept -> exponent_type
{
    exponent_type expval {};

    if ((bits_ & detail::d32_comb_11_mask) == detail::d32_comb_11_mask)
    {
        expval = (bits_ & detail::d32_11_exp_mask) >> detail::d32_11_exp_shift;
    }
    else
    {
        expval = (bits_ & detail::d32_not_11_exp_mask) >> detail::d32_not_11_exp_shift;
    }

    return expval;
}

constexpr auto decimal32_t::biased_exponent() const noexcept -> biased_exponent_type
{
    return static_cast<biased_exponent_type>(unbiased_exponent()) - detail::bias;
}

template <typename T>
constexpr auto decimal32_t::edit_exponent(T expval) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, T, void)
{
    *this = decimal32_t(this->full_significand(), expval, this->isneg());
}

constexpr auto decimal32_t::full_significand() const noexcept -> significand_type
{
    significand_type significand {};

    if ((bits_ & detail::d32_comb_11_mask) == detail::d32_comb_11_mask)
    {
        constexpr std::uint32_t implied_bit {UINT32_C(0b100000000000000000000000)};
        significand = implied_bit | (bits_ & detail::d32_11_significand_mask);
    }
    else
    {
        significand = bits_ & detail::d32_not_11_significand_mask;
    }

    return significand;
}

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4127)
#endif

template <typename T>
constexpr auto decimal32_t::edit_significand(const T sig) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, T, void)
{
    const auto unsigned_sig {detail::make_positive_unsigned(sig)};
    BOOST_DECIMAL_IF_CONSTEXPR (detail::is_signed_v<T>)
    {
        *this = decimal32_t(unsigned_sig, this->biased_exponent(), this->isneg() || sig < 0);
    }
    else
    {
        *this = decimal32_t(unsigned_sig, this->biased_exponent(), this->isneg());
    }
}

#ifdef _MSC_VER
#  pragma warning(pop)
#endif

constexpr auto decimal32_t::isneg() const noexcept -> bool
{
    return static_cast<bool>(bits_ & detail::d32_sign_mask);
}

// Allows changing the sign even on nans and infs
constexpr auto decimal32_t::edit_sign(const bool sign) noexcept -> void
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

constexpr auto decimal32_t::to_components() const noexcept -> detail::decimal32_t_components
{
    detail::decimal32_t_components components {};

    exponent_type expval {};
    significand_type significand {};

    if ((bits_ & detail::d32_comb_11_mask) == detail::d32_comb_11_mask)
    {
        constexpr std::uint32_t implied_bit {UINT32_C(0b100000000000000000000000)};
        significand = implied_bit | (bits_ & detail::d32_11_significand_mask);
        expval = (bits_ & detail::d32_11_exp_mask) >> detail::d32_11_exp_shift;
    }
    else
    {
        significand = bits_ & detail::d32_not_11_significand_mask;
        expval = (bits_ & detail::d32_not_11_exp_mask) >> detail::d32_not_11_exp_shift;
    }

    components.sig = significand;
    components.exp = static_cast<decimal32_t::biased_exponent_type>(expval) - detail::bias_v<decimal32_t>;
    components.sign = bits_ & detail::d32_sign_mask;

    return components;
}


#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_REAL Float>
#else
template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool>>
#endif
BOOST_DECIMAL_CXX20_CONSTEXPR decimal32_t::decimal32_t(const Float val) noexcept
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
            *this = decimal32_t {components.mantissa, components.exponent, components.sign};
        }
    }
}

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

template <typename Float>
BOOST_DECIMAL_CXX20_CONSTEXPR auto decimal32_t::operator=(const Float& val) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_floating_point_v, Float, decimal32_t&)
{
    *this = decimal32_t{val};
    return *this;
}

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal>
#else
template <typename Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool>>
#endif
constexpr decimal32_t::decimal32_t(const Decimal val) noexcept
{
    *this = to_decimal<decimal32_t, Decimal>(val);
}

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_INTEGRAL Integer>
#else
template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool>>
#endif
constexpr decimal32_t::decimal32_t(const Integer val) noexcept : decimal32_t{val, 0}// NOLINT : Incorrect parameter is never used
{
}

template <typename Integer>
constexpr auto decimal32_t::operator=(const Integer& val) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t&)
{
    using ConversionType = std::conditional_t<std::is_same<Integer, bool>::value, std::int32_t, Integer>;
    *this = decimal32_t{static_cast<ConversionType>(val), 0};
    return *this;
}

constexpr decimal32_t::operator bool() const noexcept
{
    constexpr decimal32_t zero {0, 0};
    return *this != zero;
}

constexpr decimal32_t::operator int() const noexcept
{
    return to_integral<decimal32_t, int>(*this);
}

constexpr decimal32_t::operator unsigned() const noexcept
{
    return to_integral<decimal32_t, unsigned>(*this);
}

constexpr decimal32_t::operator long() const noexcept
{
    return to_integral<decimal32_t, long>(*this);
}

constexpr decimal32_t::operator unsigned long() const noexcept
{
    return to_integral<decimal32_t, unsigned long>(*this);
}

constexpr decimal32_t::operator long long() const noexcept
{
    return to_integral<decimal32_t, long long>(*this);
}

constexpr decimal32_t::operator unsigned long long() const noexcept
{
    return to_integral<decimal32_t, unsigned long long>(*this);
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr decimal32_t::operator detail::builtin_int128_t() const noexcept
{
    return to_integral<decimal32_t, detail::builtin_int128_t>(*this);
}

constexpr decimal32_t::operator detail::builtin_uint128_t() const noexcept
{
    return to_integral<decimal32_t, detail::builtin_uint128_t>(*this);
}

#endif

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool>>
constexpr decimal32_t::operator Decimal() const noexcept
{
    return to_decimal<Decimal>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR auto to_bits(const decimal32_t rhs) noexcept -> std::uint32_t
{
    const auto bits {detail::bit_cast<std::uint32_t>(rhs.bits_)};
    return bits;
}

constexpr auto operator*(const decimal32_t lhs, const decimal32_t rhs) noexcept -> decimal32_t
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (!isfinite(lhs) || !isfinite(rhs))
    {
        return detail::check_non_finite(lhs, rhs);
    }
    #endif

    const auto lhs_components {lhs.to_components()};
    const auto rhs_components {rhs.to_components()};

    return detail::mul_impl<decimal32_t>(lhs_components, rhs_components);
}

template <typename Integer>
constexpr auto operator*(const decimal32_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t)
{
    using promoted_significand_type = detail::promote_significand_t<decimal32_t, Integer>;
    using exp_type = decimal32_t::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (!isfinite(lhs))
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
    const auto final_sig_rhs {static_cast<decimal32_t::significand_type>(sig_rhs)};

    return detail::mul_impl<decimal32_t>(sig_lhs, exp_lhs, lhs.isneg(),
                                       final_sig_rhs, exp_rhs, (rhs < 0));
}

template <typename Integer>
constexpr auto operator*(const Integer lhs, const decimal32_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t)
{
    return rhs * lhs;
}

constexpr auto decimal32_t::operator*=(const decimal32_t rhs) noexcept -> decimal32_t&
{
    *this = *this * rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal32_t::operator*=(const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t&)
{
    *this = *this * rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal32_t::operator*=(const Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal32_t&)
{
    *this = *this * rhs;
    return *this;
}

constexpr auto div_impl(const decimal32_t lhs, const decimal32_t rhs, decimal32_t& q, decimal32_t& r) noexcept -> void
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check pre-conditions
    constexpr decimal32_t zero {0, 0};
    constexpr decimal32_t nan {boost::decimal::from_bits(boost::decimal::detail::d32_snan_mask)};
    constexpr decimal32_t inf {boost::decimal::from_bits(boost::decimal::detail::d32_inf_mask)};

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

    auto lhs_components {lhs.to_components()};
    detail::normalize(lhs_components.sig, lhs_components.exp);

    auto rhs_components {rhs.to_components()};
    detail::normalize(rhs_components.sig, rhs_components.exp);

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "sig lhs: " << sig_lhs
              << "\nexp lhs: " << exp_lhs
              << "\nsig rhs: " << sig_rhs
              << "\nexp rhs: " << exp_rhs << std::endl;
    #endif

    q = detail::generic_div_impl<decimal32_t>(lhs_components, rhs_components);
}

constexpr auto mod_impl(const decimal32_t lhs, const decimal32_t rhs, const decimal32_t& q, decimal32_t& r) noexcept -> void
{
    constexpr decimal32_t zero {0, 0};

    // https://en.cppreference.com/w/cpp/numeric/math/fmod
    auto q_trunc {q > zero ? floor(q) : ceil(q)};
    r = lhs - (decimal32_t(q_trunc) * rhs);
}

constexpr auto operator/(const decimal32_t lhs, const decimal32_t rhs) noexcept -> decimal32_t
{
    decimal32_t q {};
    decimal32_t r {};
    div_impl(lhs, rhs, q, r);

    return q;
}

template <typename Integer>
constexpr auto operator/(const decimal32_t lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t)
{
    using exp_type = decimal32_t::biased_exponent_type;
    using sig_type = decimal32_t::significand_type;
    using integer_type = std::conditional_t<(std::numeric_limits<Integer>::digits10 > std::numeric_limits<sig_type>::digits10), detail::make_unsigned_t<Integer>, sig_type>;

    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check pre-conditions
    constexpr decimal32_t zero {0, 0};
    constexpr decimal32_t nan {boost::decimal::from_bits(boost::decimal::detail::d32_snan_mask)};
    constexpr decimal32_t inf {boost::decimal::from_bits(boost::decimal::detail::d32_inf_mask)};

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
    detail::decimal32_t_components lhs_components {sig_lhs, exp_lhs, lhs.isneg()};

    exp_type exp_rhs {};
    auto unsigned_rhs {static_cast<integer_type>(detail::make_positive_unsigned(rhs))};
    detail::normalize(unsigned_rhs, exp_rhs);
    detail::decimal32_t_components rhs_components {static_cast<sig_type>(unsigned_rhs), exp_rhs, rhs < 0};

    return detail::generic_div_impl<decimal32_t>(lhs_components, rhs_components);
}

template <typename Integer>
constexpr auto operator/(Integer lhs, const decimal32_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t)
{
    using exp_type = decimal32_t::biased_exponent_type;
    using sig_type = decimal32_t::significand_type;
    using integer_type = std::conditional_t<(std::numeric_limits<Integer>::digits10 > std::numeric_limits<sig_type>::digits10), detail::make_unsigned_t<Integer>, sig_type>;

    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check pre-conditions
    constexpr decimal32_t zero {0, 0};
    constexpr decimal32_t nan {boost::decimal::from_bits(boost::decimal::detail::d32_snan_mask)};
    constexpr decimal32_t inf {boost::decimal::from_bits(boost::decimal::detail::d32_inf_mask)};

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
    auto unsigned_lhs {static_cast<integer_type>(detail::make_positive_unsigned(lhs))};
    detail::normalize(unsigned_lhs, lhs_exp);
    detail::decimal32_t_components lhs_components {static_cast<sig_type>(unsigned_lhs), lhs_exp, lhs < 0};
    detail::decimal32_t_components rhs_components {sig_rhs, exp_rhs, rhs.isneg()};

    return detail::generic_div_impl<decimal32_t>(lhs_components, rhs_components);
}

constexpr auto decimal32_t::operator/=(const decimal32_t rhs) noexcept -> decimal32_t&
{
    *this = *this / rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal32_t::operator/=(const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t&)
{
    *this = *this / rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal32_t::operator/=(const Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal32_t&)
{
    *this = *this / rhs;
    return *this;
}

constexpr auto operator%(const decimal32_t lhs, const decimal32_t rhs) noexcept -> decimal32_t
{
    decimal32_t q {};
    decimal32_t r {};
    div_impl(lhs, rhs, q, r);
    mod_impl(lhs, rhs, q, r);

    return r;
}

constexpr auto decimal32_t::operator%=(const decimal32_t rhs) noexcept -> decimal32_t&
{
    *this = *this % rhs;
    return *this;
}

// LCOV_EXCL_START
inline auto debug_pattern(const decimal32_t rhs) noexcept -> void
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

BOOST_DECIMAL_CXX20_CONSTEXPR decimal32_t::operator float() const noexcept
{
    return to_float<decimal32_t, float>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal32_t::operator double() const noexcept
{
    return to_float<decimal32_t, double>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal32_t::operator long double() const noexcept
{
    // Double already has more range and precision than a decimal32_t will ever be able to provide
    return static_cast<long double>(to_float<decimal32_t, double>(*this));
}

#ifdef BOOST_DECIMAL_HAS_FLOAT16
constexpr decimal32_t::operator std::float16_t() const noexcept
{
    return static_cast<std::float16_t>(to_float<decimal32_t, float>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_FLOAT32
constexpr decimal32_t::operator std::float32_t() const noexcept
{
    return static_cast<std::float32_t>(to_float<decimal32_t, float>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_FLOAT64
constexpr decimal32_t::operator std::float64_t() const noexcept
{
    return static_cast<std::float64_t>(to_float<decimal32_t, double>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_BRAINFLOAT16
constexpr decimal32_t::operator std::bfloat16_t() const noexcept
{
    return static_cast<std::bfloat16_t>(to_float<decimal32_t, float>(*this));
}
#endif

constexpr auto operator&(const decimal32_t lhs, const decimal32_t rhs) noexcept -> decimal32_t
{
    return from_bits(lhs.bits_ & rhs.bits_);
}

template <typename Integer>
constexpr auto operator&(const decimal32_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t)
{
    return from_bits(lhs.bits_ & static_cast<std::uint32_t>(rhs));
}

template <typename Integer>
constexpr auto operator&(const Integer lhs, const decimal32_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t)
{
    return from_bits(static_cast<std::uint32_t>(lhs) & rhs.bits_);
}

constexpr auto operator|(const decimal32_t lhs, const decimal32_t rhs) noexcept -> decimal32_t
{
    return from_bits(lhs.bits_ | rhs.bits_);
}

template <typename Integer>
constexpr auto operator|(const decimal32_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t)
{
    return from_bits(lhs.bits_ | static_cast<std::uint32_t>(rhs));
}

template <typename Integer>
constexpr auto operator|(const Integer lhs, const decimal32_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t)
{
    return from_bits(static_cast<std::uint32_t>(lhs) | rhs.bits_);
}

constexpr auto operator^(const decimal32_t lhs, const decimal32_t rhs) noexcept -> decimal32_t
{
    return from_bits(lhs.bits_ ^ rhs.bits_);
}

template <typename Integer>
constexpr auto operator^(const decimal32_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t)
{
    return from_bits(lhs.bits_ ^ static_cast<std::uint32_t>(rhs));
}

template <typename Integer>
constexpr auto operator^(const Integer lhs, const decimal32_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t)
{
    return from_bits(static_cast<std::uint32_t>(lhs) ^ rhs.bits_);
}

constexpr auto operator<<(const decimal32_t lhs, const decimal32_t rhs) noexcept -> decimal32_t
{
    return from_bits(lhs.bits_ << rhs.bits_);
}

template <typename Integer>
constexpr auto operator<<(const decimal32_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t)
{
    return from_bits(lhs.bits_ << static_cast<std::uint32_t>(rhs));
}

template <typename Integer>
constexpr auto operator<<(const Integer lhs, const decimal32_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t)
{
    return from_bits(static_cast<std::uint32_t>(lhs) << rhs.bits_);
}

constexpr auto operator>>(const decimal32_t lhs, const decimal32_t rhs) noexcept -> decimal32_t
{
    return from_bits(lhs.bits_ >> rhs.bits_);
}

template <typename Integer>
constexpr auto operator>>(const decimal32_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t)
{
    return from_bits(lhs.bits_ >> static_cast<std::uint32_t>(rhs));
}

template <typename Integer>
constexpr auto operator>>(const Integer lhs, const decimal32_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_t)
{
    return from_bits(static_cast<std::uint32_t>(lhs) >> rhs.bits_);
}

constexpr auto operator~(const decimal32_t lhs) noexcept -> decimal32_t
{
    return from_bits(~lhs.bits_);
}

// 3.6.4
// Effects: determines if the quantum exponents of x and y are the same.
// If both x and y are NaN, or infinity, they have the same quantum exponents;
// if exactly one operand is infinity or exactly one operand is NaN, they do not have the same quantum exponents.
// The samequantum functions raise no exception.
constexpr auto samequantumd32(const decimal32_t lhs, const decimal32_t rhs) noexcept -> bool
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
constexpr auto quantexpd32(const decimal32_t x) noexcept -> int
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
constexpr auto quantized32(const decimal32_t lhs, const decimal32_t rhs) noexcept -> decimal32_t
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    // Return the correct type of nan
    if (isnan(lhs))
    {
        return lhs;
    }
    if (isnan(rhs))
    {
        return rhs;
    }

    // If one is infinity then return a signaling NAN
    if (isinf(lhs) != isinf(rhs))
    {
        return from_bits(detail::d32_snan_mask);
    }
    if (isinf(lhs) && isinf(rhs))
    {
        return lhs;
    }
    #endif

    return {lhs.full_significand(), rhs.biased_exponent(), lhs.isneg()};
}

constexpr auto scalblnd32(decimal32_t num, const long exp) noexcept -> decimal32_t
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    constexpr decimal32_t zero {0, 0};

    if (num == zero || exp == 0 || !isfinite(num))
    {
        return num;
    }
    #endif

    num.edit_exponent(num.biased_exponent() + exp);

    return num;
}

constexpr auto scalbnd32(const decimal32_t num, const int expval) noexcept -> decimal32_t
{
    return scalblnd32(num, static_cast<long>(expval));
}

constexpr auto copysignd32(decimal32_t mag, const decimal32_t sgn) noexcept -> decimal32_t
{
    mag.edit_sign(sgn.isneg());
    return mag;
}

} // namespace decimal
} // namespace boost

namespace std {

template <>
#ifdef _MSC_VER
class numeric_limits<boost::decimal::decimal32_t>
#else
struct numeric_limits<boost::decimal::decimal32_t>
#endif
{

#ifdef _MSC_VER
public:
#endif

    static constexpr bool is_specialized = true;
    static constexpr bool is_signed = true;
    static constexpr bool is_integer = false;
    static constexpr bool is_exact = false;
    static constexpr bool has_infinity = true;
    static constexpr bool has_quiet_NaN = true;
    static constexpr bool has_signaling_NaN = true;

    // These members were deprecated in C++23
    #if ((!defined(_MSC_VER) && (__cplusplus <= 202002L)) || (defined(_MSC_VER) && (_MSVC_LANG <= 202002L)))
    static constexpr std::float_denorm_style has_denorm = std::denorm_present;
    static constexpr bool has_denorm_loss = true;
    #endif

    static constexpr std::float_round_style round_style = std::round_indeterminate;
    static constexpr bool is_iec559 = true;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo = false;
    static constexpr int digits = 7;
    static constexpr int digits10 = digits;
    static constexpr int max_digits10 = digits;
    static constexpr int radix = 10;
    static constexpr int min_exponent = -95;
    static constexpr int min_exponent10 = min_exponent;
    static constexpr int max_exponent = 96;
    static constexpr int max_exponent10 = max_exponent;
    static constexpr bool traps = numeric_limits<std::uint32_t>::traps;
    static constexpr bool tinyness_before = true;

    // Member functions
    static constexpr auto (min)        () -> boost::decimal::decimal32_t { return {UINT32_C(1), min_exponent}; }
    static constexpr auto (max)        () -> boost::decimal::decimal32_t { return {boost::decimal::detail::d32_max_significand_value, max_exponent - digits + 1}; }
    static constexpr auto lowest       () -> boost::decimal::decimal32_t { return {boost::decimal::detail::d32_max_significand_value, max_exponent - digits + 1, true}; }
    static constexpr auto epsilon      () -> boost::decimal::decimal32_t { return {UINT32_C(1), -digits + 1}; }
    static constexpr auto round_error  () -> boost::decimal::decimal32_t { return epsilon(); }
    static constexpr auto infinity     () -> boost::decimal::decimal32_t { return boost::decimal::from_bits(boost::decimal::detail::d32_inf_mask); }
    static constexpr auto quiet_NaN    () -> boost::decimal::decimal32_t { return boost::decimal::from_bits(boost::decimal::detail::d32_nan_mask); }
    static constexpr auto signaling_NaN() -> boost::decimal::decimal32_t { return boost::decimal::from_bits(boost::decimal::detail::d32_snan_mask); }
    static constexpr auto denorm_min   () -> boost::decimal::decimal32_t { return {1, boost::decimal::detail::etiny}; }
};

} // Namespace std

#endif // BOOST_DECIMAL_decimal32_t_HPP
