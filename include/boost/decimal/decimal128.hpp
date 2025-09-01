// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_decimal128_t_HPP
#define BOOST_DECIMAL_decimal128_t_HPP

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
#include <boost/decimal/detail/cmath/next.hpp>
#include "detail/int128.hpp"
#include <boost/decimal/detail/to_chars_result.hpp>
#include <boost/decimal/detail/chars_format.hpp>
#include <boost/decimal/detail/components.hpp>

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
#include <sstream>
#endif

#endif //BOOST_DECIMAL_BUILD_MODULE

namespace boost {
namespace decimal {

namespace detail {

// See IEEE 754 section 3.5.2
BOOST_DECIMAL_CONSTEXPR_VARIABLE int128::uint128_t d128_inf_mask {UINT64_C(0x7800000000000000), UINT64_C(0)};
BOOST_DECIMAL_CONSTEXPR_VARIABLE int128::uint128_t d128_nan_mask {UINT64_C(0x7C00000000000000), UINT64_C(0)};
BOOST_DECIMAL_CONSTEXPR_VARIABLE int128::uint128_t d128_snan_mask {UINT64_C(0x7E00000000000000), UINT64_C(0)};

//    Comb.  Exponent          Significand
// s        eeeeeeeeeeeeee   (0TTT) 110-bits
// s   11   eeeeeeeeeeeeee   (100T) 110-bits

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t     d128_sign_mask {UINT64_C(0b1'00000'00000000'0000000000'0000000000'0000000000'0000000000'0000000000)};
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t     d128_combination_field_mask = UINT64_C(0b0'11'00000000'000'0000000000'0000000000'0000000000'0000000000'0000000000);

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t     d128_not_11_exp_mask = UINT64_C(0b0'11111111111111'000000000'0000000000'0000000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t     d128_not_11_exp_high_word_shift {49U};
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t     d128_11_exp_mask {UINT64_C(0b0'00'11111111111111'0000000'0000000000'0000000000'0000000000'0000000000)};
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t     d128_11_exp_high_word_shift {47U};

BOOST_DECIMAL_CONSTEXPR_VARIABLE int128::uint128_t d128_not_11_significand_mask {UINT64_C(0b0'00000000000000'111111111'1111111111'1111111111'1111111111'1111111111), UINT64_MAX};
BOOST_DECIMAL_CONSTEXPR_VARIABLE int128::uint128_t d128_11_significand_mask {UINT64_C(0b0'00'00000000000000'1111111'1111111111'1111111111'1111111111'1111111111), UINT64_MAX};

BOOST_DECIMAL_CONSTEXPR_VARIABLE int128::uint128_t d128_biggest_no_combination_significand {d128_not_11_significand_mask};

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t     d128_max_biased_exponent {UINT64_C(12287)};
BOOST_DECIMAL_CONSTEXPR_VARIABLE int128::uint128_t d128_max_significand_value {UINT64_C(0b1111011010000100110111110101011011000011111000000), UINT64_C(0b0011011110001101100011100110001111111111111111111111111111111111)};

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
constexpr auto to_chars_scientific_impl(char* first, char* last, const TargetDecimalType& value, chars_format fmt) noexcept -> to_chars_result;

} //namespace detail

BOOST_DECIMAL_EXPORT class decimal128_t final
{
public:
    using significand_type = boost::int128::uint128_t;
    using exponent_type = std::uint64_t;
    using biased_exponent_type = std::int32_t;

private:
    int128::uint128_t bits_ {};

    #ifdef BOOST_DECIMAL_HAS_INT128

    friend constexpr auto from_bits(detail::builtin_uint128_t rhs) noexcept -> decimal128_t;
    friend constexpr auto to_bits(decimal128_t rhs) noexcept -> detail::builtin_uint128_t;

    #endif

    friend constexpr auto from_bits(int128::uint128_t rhs) noexcept -> decimal128_t;

    constexpr auto unbiased_exponent() const noexcept -> std::uint64_t;
    constexpr auto biased_exponent() const noexcept -> std::int32_t;
    constexpr auto full_significand() const noexcept -> int128::uint128_t;
    constexpr auto isneg() const noexcept -> bool;
    constexpr auto to_components() const noexcept -> detail::decimal128_t_components;

    // Allows direct editing of the exp
    template <typename T, std::enable_if_t<detail::is_integral_v<T>, bool> = true>
    constexpr auto edit_exponent(T exp) noexcept -> void;
    constexpr auto edit_sign(bool sign) noexcept -> void;

    // Attempts conversion to integral type:
    // If this is nan sets errno to EINVAL and returns 0
    // If this is not representable sets errno to ERANGE and returns 0
    template <typename Decimal, typename TargetType>
    friend constexpr auto to_integral_128(Decimal val) noexcept
        BOOST_DECIMAL_REQUIRES_TWO_RETURN(detail::is_decimal_floating_point_v, Decimal, detail::is_integral_v, TargetType, TargetType);

    template <typename Decimal, typename TargetType>
    friend BOOST_DECIMAL_CXX20_CONSTEXPR auto to_float(Decimal val) noexcept
        BOOST_DECIMAL_REQUIRES_TWO_RETURN(detail::is_decimal_floating_point_v, Decimal, detail::is_floating_point_v, TargetType, TargetType);

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetType, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal>
    friend constexpr auto to_decimal(Decimal val) noexcept -> TargetType;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
    friend BOOST_DECIMAL_FORCE_INLINE constexpr auto equality_impl(DecimalType lhs, DecimalType rhs) noexcept -> bool;

    // Equality template between any integer type and decimal128_t
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, BOOST_DECIMAL_INTEGRAL Integer>
    friend constexpr auto mixed_equality_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
    friend constexpr auto mixed_decimal_equality_impl(Decimal1 lhs, Decimal2 rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                             detail::is_decimal_floating_point_v<Decimal2>), bool>;

    // Template to compare operator< for any integer type and decimal128_t
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, BOOST_DECIMAL_INTEGRAL Integer>
    friend constexpr auto less_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
    friend constexpr auto mixed_decimal_less_impl(Decimal1 lhs, Decimal2 rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                             detail::is_decimal_floating_point_v<Decimal2>), bool>;

    friend constexpr auto d128_div_impl(const decimal128_t& lhs, const decimal128_t& rhs, decimal128_t& q, decimal128_t& r) noexcept -> void;

    friend constexpr auto d128_mod_impl(const decimal128_t& lhs, const decimal128_t& rhs, const decimal128_t& q, decimal128_t& r) noexcept -> void;

    template <typename T>
    friend constexpr auto ilogb(T d) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, T, int);

    template <typename T>
    friend constexpr auto logb(T num) noexcept
        BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T);

    friend constexpr auto not_finite(decimal128_t rhs) noexcept -> bool;

    friend constexpr auto to_bid_d128(decimal128_t val) noexcept -> int128::uint128_t;

    friend constexpr auto from_bid_d128(int128::uint128_t bits) noexcept -> decimal128_t;

    #ifdef BOOST_DECIMAL_HAS_INT128
    friend constexpr auto from_bid_d128(detail::builtin_uint128_t bits) noexcept -> decimal128_t;
    #endif

    template <typename DecimalType>
    friend constexpr auto to_dpd_d128(DecimalType val) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, DecimalType, int128::uint128_t);

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
    friend constexpr auto detail::nextafter_impl(DecimalType val, bool direction) noexcept -> DecimalType;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
    friend constexpr auto detail::to_chars_scientific_impl(char* first, char* last, const TargetDecimalType& value, chars_format fmt) noexcept -> to_chars_result;

public:
    // 3.2.4.1 construct/copy/destroy
    constexpr decimal128_t() noexcept = default;
    constexpr decimal128_t& operator=(const decimal128_t& rhs) noexcept = default;
    constexpr decimal128_t& operator=(decimal128_t&& rhs) noexcept = default;
    constexpr decimal128_t(const decimal128_t& rhs) noexcept = default;
    constexpr decimal128_t(decimal128_t&& rhs) noexcept = default;

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_REAL Float>
    #else
    template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool> = true>
    #endif
    #ifndef BOOST_DECIMAL_ALLOW_IMPLICIT_CONVERSIONS
    explicit
    #endif
    BOOST_DECIMAL_CXX20_CONSTEXPR decimal128_t(Float val) noexcept;

    template <typename Float>
    BOOST_DECIMAL_CXX20_CONSTEXPR auto operator=(const Float& val) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_floating_point_v, Float, decimal128_t&);

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal>
    #else
    template <typename Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool> = true>
    #endif
    explicit constexpr decimal128_t(Decimal val) noexcept;

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_INTEGRAL Integer>
    #else
    template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool> = true>
    #endif
    constexpr decimal128_t(Integer val) noexcept;

    template <typename Integer>
    constexpr auto operator=(const Integer& val) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t&);

    // 3.2.5 initialization from coefficient and exponent:
    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_UNSIGNED_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
    #else
    template <typename T1, typename T2, std::enable_if_t<detail::is_unsigned_v<T1> && detail::is_integral_v<T2>, bool> = true>
    #endif
    constexpr decimal128_t(T1 coeff, T2 exp, bool sign = false) noexcept;

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_SIGNED_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
    #else
    template <typename T1, typename T2, std::enable_if_t<!detail::is_unsigned_v<T1> && detail::is_integral_v<T2>, bool> = true>
    #endif
    constexpr decimal128_t(T1 coeff, T2 exp) noexcept;

    explicit constexpr decimal128_t(bool value) noexcept;

    // 3.2.4.4 Conversion to integral type
    explicit constexpr operator bool() const noexcept;
    explicit constexpr operator int() const noexcept;
    explicit constexpr operator unsigned() const noexcept;
    explicit constexpr operator long() const noexcept;
    explicit constexpr operator unsigned long() const noexcept;
    explicit constexpr operator long long() const noexcept;
    explicit constexpr operator unsigned long long() const noexcept;

    explicit constexpr operator boost::int128::int128_t() const noexcept;
    explicit constexpr operator boost::int128::uint128_t() const noexcept;

    #ifdef BOOST_DECIMAL_HAS_INT128
    explicit constexpr operator detail::builtin_int128_t() const noexcept;
    explicit constexpr operator detail::builtin_uint128_t() const noexcept;
    #endif

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

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal> && (detail::decimal_val_v<Decimal> > detail::decimal_val_v<decimal128_t>), bool> = true>
    constexpr operator Decimal() const noexcept;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal> && (detail::decimal_val_v<Decimal> <= detail::decimal_val_v<decimal128_t>), bool> = true>
    explicit constexpr operator Decimal() const noexcept;

    // cmath functions that are easier as friends
    friend constexpr auto signbit     BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal128_t rhs) noexcept -> bool;
    friend constexpr auto isnan       BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal128_t rhs) noexcept -> bool;
    friend constexpr auto isinf       BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal128_t rhs) noexcept -> bool;
    friend constexpr auto issignaling BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal128_t rhs) noexcept -> bool;
    friend constexpr auto isnormal    BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal128_t rhs) noexcept -> bool;
    friend constexpr auto isfinite    BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal128_t rhs) noexcept -> bool;

    // 3.2.7 unary arithmetic operators:
    friend constexpr auto operator+(decimal128_t rhs) noexcept -> decimal128_t;
    friend constexpr auto operator-(decimal128_t rhs) noexcept -> decimal128_t;

    // 3.2.8 Binary arithmetic operators
    friend constexpr auto operator+(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> decimal128_t;

    template <typename Integer>
    friend constexpr auto operator+(decimal128_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t);

    template <typename Integer>
    friend constexpr auto operator+(Integer lhs, decimal128_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t);

    friend constexpr auto operator-(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> decimal128_t;

    template <typename Integer>
    friend constexpr auto operator-(decimal128_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t);

    template <typename Integer>
    friend constexpr auto operator-(Integer lhs, decimal128_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t);

    friend constexpr auto operator*(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> decimal128_t;

    template <typename Integer>
    friend constexpr auto operator*(decimal128_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t);

    template <typename Integer>
    friend constexpr auto operator*(Integer lhs, decimal128_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t);

    friend constexpr auto operator/(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> decimal128_t;

    template <typename Integer>
    friend constexpr auto operator/(decimal128_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t);

    template <typename Integer>
    friend constexpr auto operator/(Integer lhs, decimal128_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t);

    friend constexpr auto operator%(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> decimal128_t;

    // 3.2.4.5 Increment and Decrement
    constexpr auto operator++()    noexcept -> decimal128_t&;
    constexpr auto operator++(int) noexcept -> decimal128_t;  // NOLINT : C++14 so constexpr implies const
    constexpr auto operator--()    noexcept -> decimal128_t&;
    constexpr auto operator--(int) noexcept -> decimal128_t;  // NOLINT : C++14 so constexpr implies const

    // 3.2.4.6 Compound Assignment
    constexpr auto operator+=(decimal128_t rhs) noexcept -> decimal128_t&;

    template <typename Integer>
    constexpr auto operator+=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t&);

    template <typename Decimal>
    constexpr auto operator+=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal128_t&);

    constexpr auto operator-=(decimal128_t rhs) noexcept -> decimal128_t&;

    template <typename Integer>
    constexpr auto operator-=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t&);

    template <typename Decimal>
    constexpr auto operator-=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal128_t&);

    constexpr auto operator*=(decimal128_t rhs) noexcept -> decimal128_t&;

    template <typename Integer>
    constexpr auto operator*=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t&);

    template <typename Decimal>
    constexpr auto operator*=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal128_t&);

    constexpr auto operator/=(decimal128_t rhs) noexcept -> decimal128_t&;

    template <typename Integer>
    constexpr auto operator/=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t&);

    template <typename Decimal>
    constexpr auto operator/=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal128_t&);

    constexpr auto operator%=(decimal128_t rhs) noexcept -> decimal128_t&;

    // 3.2.9 Comparison operators:
    // Equality
    friend constexpr auto operator==(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator==(decimal128_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator==(Integer lhs, decimal128_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Inequality
    friend constexpr auto operator!=(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator!=(decimal128_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator!=(Integer lhs, decimal128_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Less
    friend constexpr auto operator<(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator<(decimal128_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator<(Integer lhs, decimal128_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Less equal
    friend constexpr auto operator<=(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator<=(decimal128_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator<=(Integer lhs, decimal128_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Greater
    friend constexpr auto operator>(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator>(decimal128_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator>(Integer lhs, decimal128_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Greater equal
    friend constexpr auto operator>=(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator>=(decimal128_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator>=(Integer lhs, decimal128_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // C++20 spaceship
    #ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR
    friend constexpr auto operator<=>(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> std::partial_ordering;

    template <typename Integer>
    friend constexpr auto operator<=>(decimal128_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering);

    template <typename Integer>
    friend constexpr auto operator<=>(Integer lhs, decimal128_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering);
    #endif

    #if !defined(BOOST_DECIMAL_DISABLE_IOSTREAM)
    friend inline std::string bit_string(decimal128_t rhs) noexcept;
    #endif

    // 3.6.4 Same Quantum
    friend constexpr auto samequantumd128(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> bool;

    // 3.6.5 Quantum exponent
    friend constexpr auto quantexpd128(decimal128_t x) noexcept -> int;

    // 3.6.6 Quantize
    friend constexpr auto quantized128(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> decimal128_t;

    // Bit-wise operators
    friend constexpr auto operator&(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> decimal128_t;

    template <typename Integer>
    friend constexpr auto operator&(decimal128_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t);

    template <typename Integer>
    friend constexpr auto operator&(Integer lhs, decimal128_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t);

    friend constexpr auto operator|(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> decimal128_t;

    template <typename Integer>
    friend constexpr auto operator|(decimal128_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t);

    template <typename Integer>
    friend constexpr auto operator|(Integer lhs, decimal128_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t);

    friend constexpr auto operator^(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> decimal128_t;

    template <typename Integer>
    friend constexpr auto operator^(decimal128_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t);

    template <typename Integer>
    friend constexpr auto operator^(Integer lhs, decimal128_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t);

    friend constexpr auto operator<<(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> decimal128_t;

    template <typename Integer>
    friend constexpr auto operator<<(decimal128_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t);

    template <typename Integer>
    friend constexpr auto operator<<(Integer lhs, decimal128_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t);

    friend constexpr auto operator>>(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> decimal128_t;

    template <typename Integer>
    friend constexpr auto operator>>(decimal128_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t);

    template <typename Integer>
    friend constexpr auto operator>>(Integer lhs, decimal128_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t);

    friend constexpr auto operator~(decimal128_t rhs) noexcept -> decimal128_t;

    // <cmath> functions that need to be friends
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
    friend constexpr auto frexp10(T num, int* expptr) noexcept -> typename T::significand_type;

    friend constexpr auto copysignd128(decimal128_t mag, decimal128_t sgn) noexcept -> decimal128_t;
    friend constexpr auto scalblnd128(decimal128_t num, long exp) noexcept -> decimal128_t;
    friend constexpr auto scalbnd128(decimal128_t num, int exp) noexcept -> decimal128_t;
    friend constexpr auto fmad128(decimal128_t x, decimal128_t y, decimal128_t z) noexcept -> decimal128_t;
};

BOOST_DECIMAL_EXPORT using decimal128 [[deprecated("Use re-named type decimal128_t instead of decimal128")]] = decimal128_t;

#if !defined(BOOST_DECIMAL_DISABLE_IOSTREAM)
inline std::string bit_string(decimal128_t rhs) noexcept
{
    std::stringstream ss;
    ss << std::hex << rhs.bits_.high << rhs.bits_.low;
    return ss.str();
}
#endif

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr auto from_bits(const detail::builtin_uint128_t rhs) noexcept -> decimal128_t
{
    decimal128_t result;
    result.bits_ = rhs;

    return result;
}

constexpr auto to_bits(const decimal128_t rhs) noexcept -> detail::builtin_uint128_t
{
    return static_cast<detail::builtin_uint128_t>(rhs.bits_);
}

#endif

constexpr auto from_bits(const int128::uint128_t rhs) noexcept -> decimal128_t
{
    decimal128_t result;
    result.bits_ = rhs;

    return result;
}

constexpr auto decimal128_t::unbiased_exponent() const noexcept -> exponent_type
{
    exponent_type expval {};

    if ((bits_.high & detail::d128_combination_field_mask) == detail::d128_combination_field_mask)
    {
        expval = (bits_.high & detail::d128_11_exp_mask) >> detail::d128_11_exp_high_word_shift;
    }
    else
    {
        expval = (bits_.high & detail::d128_not_11_exp_mask) >> detail::d128_not_11_exp_high_word_shift;
    }

    return expval;
}

constexpr auto decimal128_t::biased_exponent() const noexcept -> biased_exponent_type
{
    return static_cast<biased_exponent_type>(unbiased_exponent()) - detail::bias_v<decimal128_t>;
}

constexpr auto decimal128_t::full_significand() const noexcept -> significand_type
{
    significand_type significand {};

    if ((bits_.high & detail::d128_combination_field_mask) == detail::d128_combination_field_mask)
    {
        constexpr int128::uint128_t implied_bit {UINT64_C(0b10000000000000000000000000000000000000000000000000),0};
        significand = implied_bit | (bits_ & detail::d128_11_significand_mask);
    }
    else
    {
        significand = bits_ & detail::d128_not_11_significand_mask;
    }

    return significand;
}

constexpr auto decimal128_t::isneg() const noexcept -> bool
{
    return static_cast<bool>(bits_.high & detail::d128_sign_mask);
}

constexpr auto decimal128_t::to_components() const noexcept -> detail::decimal128_t_components
{
    significand_type significand {};
    exponent_type expval {};

    if ((bits_.high & detail::d128_combination_field_mask) == detail::d128_combination_field_mask)
    {
        constexpr int128::uint128_t implied_bit {UINT64_C(0b10000000000000000000000000000000000000000000000000),0};
        significand = implied_bit | (bits_ & detail::d128_11_significand_mask);
        expval = (bits_.high & detail::d128_11_exp_mask) >> detail::d128_11_exp_high_word_shift;
    }
    else
    {
        significand = bits_ & detail::d128_not_11_significand_mask;
        expval = (bits_.high & detail::d128_not_11_exp_mask) >> detail::d128_not_11_exp_high_word_shift;
    }

    const auto sign {static_cast<bool>(bits_.high & detail::d128_sign_mask)};

    return detail::decimal128_t_components {significand, static_cast<biased_exponent_type>(expval) - detail::bias_v<decimal128_t>, sign};
}


template <typename T, std::enable_if_t<detail::is_integral_v<T>, bool>>
constexpr auto decimal128_t::edit_exponent(const T expval) noexcept -> void
{
    *this = decimal128_t(this->full_significand(), expval, this->isneg());
}

constexpr auto decimal128_t::edit_sign(const bool sign) noexcept -> void
{
    if (sign)
    {
        bits_.high |= detail::d128_sign_mask;
    }
    else
    {
        constexpr auto not_sign_mask {~detail::d128_sign_mask};
        bits_.high &= not_sign_mask;
    }
}

#if defined(__GNUC__) && __GNUC__ >= 6
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wduplicated-branches"
#  pragma GCC diagnostic ignored "-Wconversion"
#endif


#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4127)
#endif

// e.g. for sign bits_.high |= detail::d128_sign_mask
#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_UNSIGNED_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
#else
template <typename T1, typename T2, std::enable_if_t<detail::is_unsigned_v<T1> && detail::is_integral_v<T2>, bool>>
#endif
constexpr decimal128_t::decimal128_t(T1 coeff, T2 exp, bool sign) noexcept
{
    bits_.high = sign ? detail::d128_sign_mask : UINT64_C(0);

    // If the coeff is not in range make it so
    // The coefficient needs at least 110 bits so there's a good chance we don't need to
    // We use sizeof instead of std::numeric_limits since __int128 on GCC prior to 14 without GNU mode does not overload
    // numeric_limits
    int coeff_digits {-1};
    BOOST_DECIMAL_IF_CONSTEXPR (sizeof(T1) >= sizeof(significand_type))
    {
        if (coeff > detail::d128_max_significand_value)
        {
            constexpr auto precision_plus_one {detail::precision_v<decimal128_t> + 1};
            coeff_digits = detail::d128_constructor_num_digits(coeff);
            if (coeff_digits > precision_plus_one)
            {
                const auto digits_to_remove {coeff_digits - precision_plus_one};

                #if defined(__GNUC__) && !defined(__clang__)
                #  pragma GCC diagnostic push
                #  pragma GCC diagnostic ignored "-Wconversion"
                #endif

                coeff /= detail::pow10(static_cast<T1>(digits_to_remove));

                #if defined(__GNUC__) && !defined(__clang__)
                #  pragma GCC diagnostic pop
                #endif

                coeff_digits -= digits_to_remove;
                exp += detail::fenv_round<decimal128_t>(coeff, sign) + digits_to_remove;
            }
            // Round as required
            else
            {
                exp += detail::fenv_round<decimal128_t>(coeff, sign);
            }
        }
    }

    constexpr int128::uint128_t zero {0, 0};
    auto reduced_coeff {static_cast<significand_type>(coeff)};

    if (reduced_coeff == zero)
    {
        return;
    }

    // The decimal128_t case is more straightforward than the others
    // The precision of the type is 34, and 34x 9s fits into 113 bits,
    // therefore we never need to use the combination field for additional space

    bits_ |= (reduced_coeff & detail::d128_not_11_significand_mask);

    // If the exponent fits we do not need to use the combination field
    const auto biased_exp {static_cast<std::uint64_t>(exp + detail::bias_v<decimal128_t>)};
    if (biased_exp <= detail::d128_max_biased_exponent)
    {
        bits_.high |= (biased_exp << detail::d128_not_11_exp_high_word_shift) & detail::d128_not_11_exp_mask;
    }
    else
    {
        // If we can fit the extra exponent in the significand, then we can construct the value
        // If we can't, the value is either 0 or infinity depending on the sign of exp

        if (coeff_digits == -1)
        {
            coeff_digits = detail::num_digits(reduced_coeff);
        }

        const auto exp_delta {biased_exp - detail::d128_max_biased_exponent};
        const auto digit_delta {coeff_digits - static_cast<int>(exp_delta)};
        if (digit_delta > 0 && coeff_digits + digit_delta <= detail::precision_v<decimal128_t>)
        {
            exp -= digit_delta;
            reduced_coeff *= detail::pow10(static_cast<significand_type>(digit_delta));
            *this = decimal128_t(reduced_coeff, exp, sign);
        }
        else
        {
            bits_ = exp < 0 ? zero : detail::d128_inf_mask;
        }
    }
}

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_SIGNED_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
#else
template <typename T1, typename T2, std::enable_if_t<!detail::is_unsigned_v<T1> && detail::is_integral_v<T2>, bool>>
#endif
constexpr decimal128_t::decimal128_t(const T1 coeff, const T2 exp) noexcept : decimal128_t(detail::make_positive_unsigned(coeff), exp, coeff < 0) {}

constexpr decimal128_t::decimal128_t(const bool value) noexcept : decimal128_t(static_cast<std::uint64_t>(value), 0, false) {}


#ifdef _MSC_VER
#  pragma warning(pop)
#endif

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
BOOST_DECIMAL_CXX20_CONSTEXPR decimal128_t::decimal128_t(const Float val) noexcept
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (val != val)
    {
        *this = from_bits(detail::d128_nan_mask);
    }
    else if (val == std::numeric_limits<Float>::infinity() || val == -std::numeric_limits<Float>::infinity())
    {
        *this = from_bits(detail::d128_inf_mask);
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

        if (components.exponent > detail::emax_v<decimal128_t>)
        {
            *this = from_bits(detail::d128_inf_mask);
        }
        else
        {
            *this = decimal128_t {components.mantissa, components.exponent, components.sign};
        }
    }
}

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

template <typename Float>
BOOST_DECIMAL_CXX20_CONSTEXPR auto decimal128_t::operator=(const Float& val) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_floating_point_v, Float, decimal128_t&)
{
    *this = decimal128_t{val};
    return *this;
}

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_INTEGRAL Integer>
#else
template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool>>
#endif
constexpr decimal128_t::decimal128_t(const Integer val) noexcept : decimal128_t{val, 0} {}

template <typename Integer>
constexpr auto decimal128_t::operator=(const Integer& val) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t&)
{
    using ConversionType = std::conditional_t<std::is_same<Integer, bool>::value, std::int32_t, Integer>;
    *this = decimal128_t{static_cast<ConversionType>(val), 0};
    return *this;
}

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal>
#else
template <typename Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool>>
#endif
constexpr decimal128_t::decimal128_t(const Decimal val) noexcept
{
    *this = to_decimal<decimal128_t>(val);
}

constexpr decimal128_t::operator bool() const noexcept
{
    constexpr decimal128_t zero {0, 0};
    return *this != zero;
}

constexpr decimal128_t::operator int() const noexcept
{
    return to_integral_128<decimal128_t, int>(*this);
}

constexpr decimal128_t::operator unsigned() const noexcept
{
    return to_integral_128<decimal128_t, unsigned>(*this);
}

constexpr decimal128_t::operator long() const noexcept
{
    return to_integral_128<decimal128_t, long>(*this);
}

constexpr decimal128_t::operator unsigned long() const noexcept
{
    return to_integral_128<decimal128_t, unsigned long>(*this);
}

constexpr decimal128_t::operator long long() const noexcept
{
    return to_integral_128<decimal128_t, long long>(*this);
}

constexpr decimal128_t::operator unsigned long long() const noexcept
{
    return to_integral_128<decimal128_t, unsigned long long>(*this);
}

constexpr decimal128_t::operator boost::int128::int128_t() const noexcept
{
    return to_integral_128<decimal128_t, boost::int128::int128_t>(*this);
}

constexpr decimal128_t::operator boost::int128::uint128_t() const noexcept
{
    return to_integral_128<decimal128_t, boost::int128::uint128_t>(*this);
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr decimal128_t::operator detail::builtin_int128_t() const noexcept
{
    return to_integral_128<decimal128_t, detail::builtin_int128_t>(*this);
}

constexpr decimal128_t::operator detail::builtin_uint128_t() const noexcept
{
    return to_integral_128<decimal128_t, detail::builtin_uint128_t>(*this);
}

#endif //BOOST_DECIMAL_HAS_INT128

BOOST_DECIMAL_CXX20_CONSTEXPR decimal128_t::operator float() const noexcept
{
    return to_float<decimal128_t, float>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal128_t::operator double() const noexcept
{
    return to_float<decimal128_t, double>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal128_t::operator long double() const noexcept
{
    return to_float<decimal128_t, long double>(*this);
}

#ifdef BOOST_DECIMAL_HAS_FLOAT16
constexpr decimal128_t::operator std::float16_t() const noexcept
{
    return static_cast<std::float16_t>(to_float<decimal128_t, float>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_FLOAT32
constexpr decimal128_t::operator std::float32_t() const noexcept
{
    return static_cast<std::float32_t>(to_float<decimal128_t, float>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_FLOAT64
constexpr decimal128_t::operator std::float64_t() const noexcept
{
    return static_cast<std::float64_t>(to_float<decimal128_t, double>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_BRAINFLOAT16
constexpr decimal128_t::operator std::bfloat16_t() const noexcept
{
    return static_cast<std::bfloat16_t>(to_float<decimal128_t, float>(*this));
}
#endif

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal> && (detail::decimal_val_v<Decimal> > detail::decimal_val_v<decimal128_t>), bool>>
constexpr decimal128_t::operator Decimal() const noexcept
{
    return to_decimal<Decimal>(*this);
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal> && (detail::decimal_val_v<Decimal> <= detail::decimal_val_v<decimal128_t>), bool>>
constexpr decimal128_t::operator Decimal() const noexcept
{
    return to_decimal<Decimal>(*this);
}

constexpr auto signbit BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (const decimal128_t rhs) noexcept -> bool
{
    return rhs.bits_.high & detail::d128_sign_mask;
}

constexpr auto isnan BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (const decimal128_t rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return (rhs.bits_.high & detail::d128_nan_mask.high) == detail::d128_nan_mask.high;
    #else
    static_cast<void>(rhs);
    return false;
    #endif
}

constexpr auto isinf BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (const decimal128_t rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return (rhs.bits_.high & detail::d128_nan_mask.high) == detail::d128_inf_mask.high;
    #else
    static_cast<void>(rhs);
    return false;
    #endif
}

constexpr auto issignaling BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (const decimal128_t rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return (rhs.bits_.high & detail::d128_snan_mask.high) == detail::d128_snan_mask.high;
    #else
    static_cast<void>(rhs);
    return false;
    #endif
}

constexpr auto isnormal BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (const decimal128_t rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check for de-normals
    const auto sig {rhs.full_significand()};
    const auto exp {rhs.unbiased_exponent()};

    if (exp <= detail::precision_v<decimal128_t> - 1)
    {
        return false;
    }

    return (sig != 0U) && isfinite(rhs);
    #else
    return rhs.full_significand() != 0U;
    #endif
}

constexpr auto isfinite BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (const decimal128_t rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return (rhs.bits_.high & detail::d128_inf_mask.high) != detail::d128_inf_mask.high;
    #else
    static_cast<void>(rhs);
    return true;
    #endif
}

BOOST_DECIMAL_FORCE_INLINE constexpr auto not_finite(const decimal128_t rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return (rhs.bits_.high & detail::d128_inf_mask.high) == detail::d128_inf_mask.high;
    #else
    static_cast<void>(rhs);
    return false;
    #endif
}

constexpr auto operator+(const decimal128_t rhs) noexcept -> decimal128_t
{
    return rhs;
}

constexpr auto operator-(decimal128_t rhs) noexcept-> decimal128_t
{
    rhs.bits_.high ^= detail::d128_sign_mask;
    return rhs;
}


constexpr auto operator==(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> bool
{
    return equality_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator==(const decimal128_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return mixed_equality_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator==(const Integer lhs, const decimal128_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return mixed_equality_impl(rhs, lhs);
}

constexpr auto operator!=(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> bool
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator!=(const decimal128_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator!=(const Integer lhs, const decimal128_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return !(lhs == rhs);
}

constexpr auto operator<(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs) || not_finite(rhs))
    {
        if (isnan(lhs) || isnan(rhs) ||
            (!lhs.isneg() && rhs.isneg()))
        {
            return false;
        }
        if (lhs.isneg() && !rhs.isneg())
        {
            return true;
        }
        if (isfinite(lhs) && isinf(rhs))
        {
            return !rhs.isneg();
        }
    }
    #endif

    return less_parts_impl<decimal128_t>(lhs.full_significand(), lhs.biased_exponent(), lhs.isneg(),
                                       rhs.full_significand(), rhs.biased_exponent(), rhs.isneg());
}

template <typename Integer>
constexpr auto operator<(const decimal128_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return less_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator<(const Integer lhs, const decimal128_t rhs) noexcept
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

constexpr auto operator<=(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> bool
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
constexpr auto operator<=(const decimal128_t lhs, const Integer rhs) noexcept
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
constexpr auto operator<=(const Integer lhs, const decimal128_t rhs) noexcept
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

constexpr auto operator>(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> bool
{
    return rhs < lhs;
}

template <typename Integer>
constexpr auto operator>(const decimal128_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return rhs < lhs;
}

template <typename Integer>
constexpr auto operator>(const Integer lhs, const decimal128_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return rhs < lhs;
}

constexpr auto operator>=(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> bool
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
constexpr auto operator>=(const decimal128_t lhs, const Integer rhs) noexcept
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
constexpr auto operator>=(const Integer lhs, const decimal128_t rhs) noexcept
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

constexpr auto operator<=>(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> std::partial_ordering
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
constexpr auto operator<=>(const decimal128_t lhs, const Integer rhs) noexcept
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
constexpr auto operator<=>(const Integer lhs, const decimal128_t rhs) noexcept
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

#ifdef BOOST_DECIMAL_DEBUG_ADD_128
static char* mini_to_chars( char (&buffer)[ 64 ], boost::decimal::detail::builtin_uint128_t v )
{
    char* p = buffer + 64;
    *--p = '\0';

    do
    {
        *--p = "0123456789"[ v % 10 ];
        v /= 10;
    }
    while ( v != 0 );

    return p;
}

#if !defined(BOOST_DECIMAL_DISABLE_IOSTREAM)
std::ostream& operator<<( std::ostream& os, boost::decimal::detail::builtin_uint128_t v )
{
    char buffer[ 64 ];

    os << mini_to_chars( buffer, v );
    return os;
}
#endif
#endif

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4127) // If constexpr macro only works for C++17 and above
#endif

constexpr auto d128_div_impl(const decimal128_t& lhs, const decimal128_t& rhs, decimal128_t& q, decimal128_t& r) noexcept -> void
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check pre-conditions
    constexpr decimal128_t zero {0, 0};
    constexpr decimal128_t nan {boost::decimal::from_bits(boost::decimal::detail::d128_snan_mask)};
    constexpr decimal128_t inf {boost::decimal::from_bits(boost::decimal::detail::d128_inf_mask)};

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
    detail::normalize<decimal128_t>(sig_lhs, exp_lhs);

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize<decimal128_t>(sig_rhs, exp_rhs);

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "sig lhs: " << sig_lhs
              << "\nexp lhs: " << exp_lhs
              << "\nsig rhs: " << sig_rhs
              << "\nexp rhs: " << exp_rhs << std::endl;
    #endif

    detail::decimal128_t_components lhs_components {sig_lhs, exp_lhs, lhs.isneg()};
    detail::decimal128_t_components rhs_components {sig_rhs, exp_rhs, rhs.isneg()};
    detail::decimal128_t_components q_components {};

    detail::d128_generic_div_impl(lhs_components, rhs_components, q_components);

    q = decimal128_t(q_components.sig, q_components.exp, q_components.sign);
}

#ifdef _MSC_VER
#  pragma warning(pop)
#endif

constexpr auto d128_mod_impl(const decimal128_t& lhs, const decimal128_t& rhs, const decimal128_t& q, decimal128_t& r) noexcept -> void
{
    constexpr decimal128_t zero {0, 0};

    auto q_trunc {q > zero ? floor(q) : ceil(q)};
    r = lhs - (decimal128_t(q_trunc) * rhs);
}

constexpr auto operator+(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> decimal128_t
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs) || not_finite(rhs))
    {
        return detail::check_non_finite(lhs, rhs);
    }
    #endif

    auto lhs_sig {lhs.full_significand()};
    auto lhs_exp {lhs.biased_exponent()};
    detail::normalize<decimal128_t>(lhs_sig, lhs_exp);

    auto rhs_sig {rhs.full_significand()};
    auto rhs_exp {rhs.biased_exponent()};
    detail::normalize<decimal128_t>(rhs_sig, rhs_exp);

    return detail::d128_add_impl<decimal128_t>(lhs_sig, lhs_exp, lhs.isneg(),
                                             rhs_sig, rhs_exp, rhs.isneg(),
                                             abs(lhs) > abs(rhs));
}

template <typename Integer>
constexpr auto operator+(const decimal128_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t)
{
    using exp_type = decimal128_t::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs))
    {
        return lhs;
    }
    #endif

    auto sig_rhs {static_cast<int128::uint128_t>(detail::make_positive_unsigned(rhs))};
    bool abs_lhs_bigger {abs(lhs) > sig_rhs};

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize<decimal128_t>(sig_lhs, exp_lhs);

    exp_type exp_rhs {0};
    detail::normalize<decimal128_t>(sig_rhs, exp_rhs);

    return detail::d128_add_impl<decimal128_t>(sig_lhs, exp_lhs, lhs.isneg(),
                                             sig_rhs, exp_rhs, (rhs < 0),
                                             abs_lhs_bigger);
}

template <typename Integer>
constexpr auto operator+(const Integer lhs, const decimal128_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t)
{
    return rhs + lhs;
}

// NOLINTNEXTLINE: If subtraction is actually addition than use operator+ and vice versa
constexpr auto operator-(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> decimal128_t
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs) || not_finite(rhs))
    {
        return detail::check_non_finite(lhs, rhs);
    }
    #endif

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize<decimal128_t>(sig_lhs, exp_lhs);

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize<decimal128_t>(sig_rhs, exp_rhs);

    return detail::d128_sub_impl<decimal128_t>(
            sig_lhs, exp_lhs, lhs.isneg(),
            sig_rhs, exp_rhs, rhs.isneg(),
            abs(lhs) > abs(rhs));
}

template <typename Integer>
constexpr auto operator-(const decimal128_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t)
{
    using exp_type = decimal128_t::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs))
    {
        return lhs;
    }
    #endif

    auto sig_rhs {static_cast<int128::uint128_t>(detail::make_positive_unsigned(rhs))};
    const bool abs_lhs_bigger {abs(lhs) > sig_rhs};

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize<decimal128_t>(sig_lhs, exp_lhs);

    exp_type exp_rhs {0};
    detail::normalize<decimal128_t>(sig_rhs, exp_rhs);

    return detail::d128_sub_impl<decimal128_t>(
            sig_lhs, exp_lhs, lhs.isneg(),
            sig_rhs, exp_rhs, (rhs < 0),
            abs_lhs_bigger);
}

template <typename Integer>
constexpr auto operator-(const Integer lhs, const decimal128_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t)
{
    using exp_type = decimal128_t::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(rhs))
    {
        return rhs;
    }
    #endif

    auto sig_lhs {static_cast<int128::uint128_t>(detail::make_positive_unsigned(lhs))};
    const bool abs_lhs_bigger {sig_lhs > abs(rhs)};

    exp_type exp_lhs {0};
    detail::normalize<decimal128_t>(sig_lhs, exp_lhs);

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize<decimal128_t>(sig_rhs, exp_rhs);

    return detail::d128_sub_impl<decimal128_t>(
            sig_lhs, exp_lhs, (lhs < 0),
            sig_rhs, exp_rhs, rhs.isneg(),
            abs_lhs_bigger);
}

constexpr auto operator*(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> decimal128_t
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs) || not_finite(rhs))
    {
        return detail::check_non_finite(lhs, rhs);
    }
    #endif

    const auto lhs_sig {lhs.full_significand()};
    const auto lhs_exp {lhs.biased_exponent()};

    const auto rhs_sig {rhs.full_significand()};
    const auto rhs_exp {rhs.biased_exponent()};

    return detail::d128_mul_impl<decimal128_t>(
            lhs_sig, lhs_exp, lhs.isneg(),
            rhs_sig, rhs_exp, rhs.isneg());
}

template <typename Integer>
constexpr auto operator*(const decimal128_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t)
{
    using exp_type = decimal128_t::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs))
    {
        return lhs;
    }
    #endif

    auto lhs_sig {lhs.full_significand()};
    auto lhs_exp {lhs.biased_exponent()};
    const auto lhs_zeros {detail::remove_trailing_zeros(lhs_sig)};
    lhs_sig = lhs_zeros.trimmed_number;
    lhs_exp += static_cast<std::int32_t>(lhs_zeros.number_of_removed_zeros);

    auto rhs_sig {static_cast<int128::uint128_t>(detail::make_positive_unsigned(rhs))};
    const auto rhs_zeros {detail::remove_trailing_zeros(rhs_sig)};
    rhs_sig = rhs_zeros.trimmed_number;
    const auto rhs_exp = static_cast<exp_type>(rhs_zeros.number_of_removed_zeros);

    return detail::d128_mul_impl<decimal128_t>(
            lhs_sig, lhs_exp, lhs.isneg(),
            rhs_sig, rhs_exp, (rhs < 0));

}

template <typename Integer>
constexpr auto operator*(const Integer lhs, const decimal128_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t)
{
    return rhs * lhs;
}

constexpr auto operator/(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> decimal128_t
{
    decimal128_t q {};
    decimal128_t r {};
    d128_div_impl(lhs, rhs, q, r);

    return q;
}

template <typename Integer>
constexpr auto operator/(const decimal128_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check pre-conditions
    constexpr decimal128_t zero {0, 0};
    constexpr decimal128_t nan {boost::decimal::from_bits(boost::decimal::detail::d128_snan_mask)};
    constexpr decimal128_t inf {boost::decimal::from_bits(boost::decimal::detail::d128_inf_mask)};

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
    detail::normalize<decimal128_t>(lhs_sig, lhs_exp);

    const detail::decimal128_t_components lhs_components {lhs_sig, lhs_exp, lhs.isneg()};

    const detail::decimal128_t_components rhs_components {detail::make_positive_unsigned(rhs), 0, rhs < 0};
    detail::decimal128_t_components q_components {};

    detail::d128_generic_div_impl(lhs_components, rhs_components, q_components);

    return decimal128_t(q_components.sig, q_components.exp, q_components.sign);
}

template <typename Integer>
constexpr auto operator/(const Integer lhs, const decimal128_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check pre-conditions
    constexpr decimal128_t zero {0, 0};
    constexpr decimal128_t inf {boost::decimal::from_bits(boost::decimal::detail::d128_inf_mask)};
    constexpr decimal128_t nan {boost::decimal::from_bits(boost::decimal::detail::d128_snan_mask)};

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
    detail::normalize<decimal128_t>(rhs_sig, rhs_exp);

    const detail::decimal128_t_components lhs_components {detail::make_positive_unsigned(lhs), 0, lhs < 0};
    const detail::decimal128_t_components rhs_components {rhs_sig, rhs_exp, rhs.isneg()};
    detail::decimal128_t_components q_components {};

    detail::d128_generic_div_impl(lhs_components, rhs_components, q_components);

    return decimal128_t(q_components.sig, q_components.exp, q_components.sign);
}

constexpr auto operator%(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> decimal128_t
{
    decimal128_t q {};
    decimal128_t r {};
    d128_div_impl(lhs, rhs, q, r);
    d128_mod_impl(lhs, rhs, q, r);

    return r;
}

constexpr auto decimal128_t::operator++() noexcept -> decimal128_t&
{
    constexpr decimal128_t one{1, 0};
    *this = *this + one;
    return *this;
}

constexpr auto decimal128_t::operator++(int) noexcept -> decimal128_t
{
    return ++(*this);
}

constexpr auto decimal128_t::operator--() noexcept -> decimal128_t&
{
    constexpr decimal128_t one{1, 0};
    *this = *this - one;
    return *this;
}

constexpr auto decimal128_t::operator--(int) noexcept -> decimal128_t
{
    return --(*this);
}

constexpr auto decimal128_t::operator+=(const decimal128_t rhs) noexcept -> decimal128_t&
{
    *this = *this + rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal128_t::operator+=(const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t&)
{
    *this = *this + rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal128_t::operator+=(const Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal128_t&)
{
    *this = *this + rhs;
    return *this;
}

constexpr auto decimal128_t::operator-=(const decimal128_t rhs) noexcept -> decimal128_t&
{
    *this = *this - rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal128_t::operator-=(const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t&)
{
    *this = *this - rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal128_t::operator-=(const Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal128_t&)
{
    *this = *this - rhs;
    return *this;
}

constexpr auto decimal128_t::operator*=(const decimal128_t rhs) noexcept -> decimal128_t&
{
    *this = *this * rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal128_t::operator*=(const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t&)
{
    *this = *this * rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal128_t::operator*=(const Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal128_t&)
{
    *this = *this * rhs;
    return *this;
}

constexpr auto decimal128_t::operator/=(const decimal128_t rhs) noexcept -> decimal128_t&
{
    *this = *this / rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal128_t::operator/=(const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t&)
{
    *this = *this / rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal128_t::operator/=(const Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal128_t&)
{
    *this = *this / rhs;
    return *this;
}

constexpr auto decimal128_t::operator%=(const decimal128_t rhs) noexcept -> decimal128_t&
{
    *this = *this % rhs;
    return *this;
}

// 3.6.4
// Effects: determines if the quantum exponents of x and y are the same.
// If both x and y are NaN, or infinity, they have the same quantum exponents;
// if exactly one operand is infinity or exactly one operand is NaN, they do not have the same quantum exponents.
// The samequantum functions raise no exception.
constexpr auto samequantumd128(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> bool
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
constexpr auto quantexpd128(const decimal128_t x) noexcept -> int
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
constexpr auto quantized128(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> decimal128_t
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
        return boost::decimal::from_bits(boost::decimal::detail::d128_snan_mask);
    }
    else if (isinf(lhs) && isinf(rhs))
    {
        return lhs;
    }
    #endif

    return {lhs.full_significand(), rhs.biased_exponent(), lhs.isneg()};
}

constexpr auto operator&(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> decimal128_t
{
    return from_bits(lhs.bits_ & rhs.bits_);
}

template <typename Integer>
constexpr auto operator&(const decimal128_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t)
{
    return from_bits(lhs.bits_ & static_cast<int128::uint128_t>(rhs));
}

template <typename Integer>
constexpr auto operator&(const Integer lhs, const decimal128_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t)
{
    return from_bits(static_cast<int128::uint128_t>(lhs) & rhs.bits_);
}

constexpr auto operator|(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> decimal128_t
{
    return from_bits(lhs.bits_ | rhs.bits_);
}

template <typename Integer>
constexpr auto operator|(const decimal128_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t)
{
    return from_bits(lhs.bits_ | static_cast<int128::uint128_t>(rhs));
}

template <typename Integer>
constexpr auto operator|(const Integer lhs, const decimal128_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t)
{
    return from_bits(static_cast<int128::uint128_t>(lhs) | rhs.bits_);
}

constexpr auto operator^(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> decimal128_t
{
    return from_bits(lhs.bits_ ^ rhs.bits_);
}

template <typename Integer>
constexpr auto operator^(const decimal128_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t)
{
    return from_bits(lhs.bits_ ^ static_cast<int128::uint128_t>(rhs));
}

template <typename Integer>
constexpr auto operator^(const Integer lhs, const decimal128_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t)
{
    return from_bits(static_cast<int128::uint128_t>(lhs) ^ rhs.bits_);
}

constexpr auto operator<<(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> decimal128_t
{
    return from_bits(lhs.bits_ << static_cast<std::uint64_t>(rhs.bits_));
}

template <typename Integer>
constexpr auto operator<<(const decimal128_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t)
{
    return from_bits(lhs.bits_ << static_cast<std::uint64_t>(rhs));
}

template <typename Integer>
constexpr auto operator<<(const Integer lhs, const decimal128_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t)
{
    return from_bits(static_cast<int128::uint128_t>(lhs) << static_cast<std::uint64_t>(rhs.bits_));
}

constexpr auto operator>>(const decimal128_t& lhs, const decimal128_t& rhs) noexcept -> decimal128_t
{
    return from_bits(lhs.bits_ >> static_cast<std::uint64_t>(rhs.bits_));
}

template <typename Integer>
constexpr auto operator>>(const decimal128_t lhs, const Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t)
{
    return from_bits(lhs.bits_ >> static_cast<std::uint64_t>(rhs));
}

template <typename Integer>
constexpr auto operator>>(const Integer lhs, const decimal128_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_t)
{
    return from_bits(static_cast<int128::uint128_t>(lhs) >> static_cast<std::uint64_t>(rhs.bits_));
}

constexpr auto operator~(const decimal128_t lhs) noexcept -> decimal128_t
{
    return from_bits(~lhs.bits_);
}

constexpr auto copysignd128(decimal128_t mag, const decimal128_t sgn) noexcept -> decimal128_t
{
    mag.edit_sign(sgn.isneg());
    return mag;
}

constexpr auto scalblnd128(decimal128_t num, const long exp) noexcept -> decimal128_t
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    constexpr decimal128_t zero {0, 0};

    if (num == zero || exp == 0 || not_finite(num))
    {
        return num;
    }
    #endif

    num.edit_exponent(num.biased_exponent() + exp);

    return num;
}

constexpr auto scalbnd128(decimal128_t num, const int expval) noexcept -> decimal128_t
{
    return scalblnd128(num, static_cast<long>(expval));
}

} //namespace decimal
} //namespace boost

namespace std {

template<>
#ifdef _MSC_VER
class numeric_limits<boost::decimal::decimal128_t>
#else
struct numeric_limits<boost::decimal::decimal128_t>
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
    static constexpr int  digits = 34;
    static constexpr int  digits10 = digits;
    static constexpr int  max_digits10 = digits;
    static constexpr int  radix = 10;
    static constexpr int  min_exponent = -6143;
    static constexpr int  min_exponent10 = min_exponent;
    static constexpr int  max_exponent = 6144;
    static constexpr int  max_exponent10 = max_exponent;
    static constexpr bool traps = numeric_limits<std::uint64_t>::traps;
    static constexpr bool tinyness_before = true;

    // Member functions
    static constexpr auto (min)        () -> boost::decimal::decimal128_t { return {UINT32_C(1), min_exponent}; }
    static constexpr auto (max)        () -> boost::decimal::decimal128_t { return {boost::int128::uint128_t{UINT64_C(0b1111011010000100110111110101011011000011111000000), UINT64_C(0b0011011110001101100011100110001111111111111111111111111111111111)}, max_exponent - digits + 1}; }
    static constexpr auto lowest       () -> boost::decimal::decimal128_t { return {boost::int128::uint128_t{UINT64_C(0b1111011010000100110111110101011011000011111000000), UINT64_C(0b0011011110001101100011100110001111111111111111111111111111111111)}, max_exponent - digits + 1, true}; }
    static constexpr auto epsilon      () -> boost::decimal::decimal128_t { return {UINT32_C(1), -digits + 1}; }
    static constexpr auto round_error  () -> boost::decimal::decimal128_t { return epsilon(); }
    static constexpr auto infinity     () -> boost::decimal::decimal128_t { return boost::decimal::from_bits(boost::decimal::detail::d128_inf_mask); }
    static constexpr auto quiet_NaN    () -> boost::decimal::decimal128_t { return boost::decimal::from_bits(boost::decimal::detail::d128_nan_mask); }
    static constexpr auto signaling_NaN() -> boost::decimal::decimal128_t { return boost::decimal::from_bits(boost::decimal::detail::d128_snan_mask); }
    static constexpr auto denorm_min   () -> boost::decimal::decimal128_t { return {1, boost::decimal::detail::etiny_v<boost::decimal::decimal128_t>}; }
};

} //namespace std

#endif //BOOST_DECIMAL_decimal128_t_HPP
