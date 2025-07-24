// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_decimal64_t_HPP
#define BOOST_DECIMAL_decimal64_t_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/bit_cast.hpp>
#include <boost/decimal/detail/config.hpp>
#include "detail/int128.hpp"
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
#include <boost/decimal/detail/components.hpp>
#include <boost/decimal/detail/cmath/next.hpp>

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
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_inf_mask  = UINT64_C(0x7800000000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_nan_mask  = UINT64_C(0x7C00000000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_snan_mask = UINT64_C(0x7E00000000000000);

//    Comb.  Exponent          Significand
// s        eeeeeeeeee    [ttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
// s   11   eeeeeeeeee   [100t][tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_sign_mask = UINT64_C(0b1'00000'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_combination_field_mask = UINT64_C(0b0'11'00000000'000'0000000000'0000000000'0000000000'0000000000'0000000000);

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_not_11_exp_mask = UINT64_C(0b0'11'11111111'000'0000000000'0000000000'0000000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_not_11_exp_shift = UINT64_C(53);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_11_exp_mask = UINT64_C(0b0'00'1111111111'0'0000000000'0000000000'0000000000'0000000000'0000000000);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_11_exp_shift = UINT64_C(51);

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_not_11_significand_mask = UINT64_C(0b0'00'00000000'111'1111111111'1111111111'1111111111'1111111111'1111111111);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_11_significand_mask = UINT64_C(0b0'00'0000000000'1'1111111111'1111111111'1111111111'1111111111'1111111111);

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_biggest_no_combination_significand = d64_not_11_significand_mask;

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_max_biased_exponent = UINT64_C(767);
BOOST_DECIMAL_CONSTEXPR_VARIABLE std::uint64_t d64_max_significand_value = UINT64_C(9'999'999'999'999'999);

} //namespace detail

#if defined(__GNUC__) && __GNUC__ >= 8
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif

BOOST_DECIMAL_EXPORT class decimal64_t final
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

    constexpr auto to_components() const noexcept -> detail::decimal64_t_components;

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
    friend constexpr auto from_bits(std::uint64_t bits) noexcept -> decimal64_t;
    friend constexpr auto to_bits(decimal64_t rhs) noexcept -> std::uint64_t;

    // Equality template between any integer type and decimal64_t
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, BOOST_DECIMAL_INTEGRAL Integer>
    friend constexpr auto mixed_equality_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
    friend constexpr auto mixed_decimal_equality_impl(Decimal1 lhs, Decimal2 rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                             detail::is_decimal_floating_point_v<Decimal2>), bool>;

    // Template to compare operator< for any integer type and decimal64_t
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, BOOST_DECIMAL_INTEGRAL Integer>
    friend constexpr auto less_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
    friend constexpr auto mixed_decimal_less_impl(Decimal1 lhs, Decimal2 rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                             detail::is_decimal_floating_point_v<Decimal2>), bool>;

    friend constexpr auto d64_div_impl(decimal64_t lhs, decimal64_t rhs, decimal64_t& q, decimal64_t& r) noexcept -> void;

    friend constexpr auto d64_mod_impl(decimal64_t lhs, decimal64_t rhs, const decimal64_t& q, decimal64_t& r) noexcept -> void;

    template <typename T>
    friend constexpr auto ilogb(T d) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, T, int);

    template <typename T>
    friend constexpr auto logb(T num) noexcept
        BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T);

    // Micro-optimization: Nearly every call to isfinite in the basic operators is !isfinite.
    // We can super easily combine this into a single op
    friend constexpr auto not_finite(decimal64_t rhs) noexcept -> bool;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
    friend constexpr auto equality_impl(DecimalType lhs, DecimalType rhs) noexcept -> bool;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
    friend constexpr auto sequential_less_impl(DecimalType lhs, DecimalType rhs) noexcept -> bool;

    friend constexpr auto to_bid_d64(decimal64_t val) noexcept -> std::uint64_t;

    friend constexpr auto from_bid_d64(std::uint64_t bits) noexcept -> decimal64_t;

    template <typename DecimalType>
    friend constexpr auto to_dpd_d64(DecimalType val) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, DecimalType, std::uint64_t);

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
    friend constexpr auto detail::nextafter_impl(DecimalType val, bool direction) noexcept -> DecimalType;

public:
    // 3.2.3.1 construct/copy/destroy
    constexpr decimal64_t() noexcept = default;
    constexpr decimal64_t& operator=(const decimal64_t& rhs) noexcept = default;
    constexpr decimal64_t& operator=(decimal64_t&& rhs) noexcept = default;
    constexpr decimal64_t(const decimal64_t& rhs) noexcept = default;
    constexpr decimal64_t(decimal64_t&& rhs) noexcept = default;

    // 3.2.2.2 Conversion form floating-point type
    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_REAL Float>
    #else
    template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool> = true>
    #endif
    #ifndef BOOST_DECIMAL_ALLOW_IMPLICIT_CONVERSIONS
    explicit
    #endif
    BOOST_DECIMAL_CXX20_CONSTEXPR decimal64_t(Float val) noexcept;

    template <typename Float>
    BOOST_DECIMAL_CXX20_CONSTEXPR auto operator=(const Float& val) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_floating_point_v, Float, decimal64_t&);

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal>
    #else
    template <typename Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool> = true>
    #endif
    explicit constexpr decimal64_t(Decimal val) noexcept;

    // 3.2.3.3 Conversion from integral type
    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_INTEGRAL Integer>
    #else
    template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool> = true>
    #endif
    constexpr decimal64_t(Integer val) noexcept;

    template <typename Integer>
    constexpr auto operator=(const Integer& val) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t&);

    // 3.2.3.4 Conversion to integral type
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


    // Conversion to other decimal type
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal> && (detail::decimal_val_v<Decimal> > detail::decimal_val_v<decimal64_t>), bool> = true>
    constexpr operator Decimal() const noexcept;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal> && (detail::decimal_val_v<Decimal> <= detail::decimal_val_v<decimal64_t>), bool> = true>
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
    template <BOOST_DECIMAL_UNSIGNED_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
    #else
    template <typename T1, typename T2, std::enable_if_t<detail::is_unsigned_v<T1> && detail::is_integral_v<T2>, bool> = true>
    #endif
    constexpr decimal64_t(T1 coeff, T2 exp, bool sign = false) noexcept;

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_SIGNED_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
    #else
    template <typename T1, typename T2, std::enable_if_t<!detail::is_unsigned_v<T1> && detail::is_integral_v<T2>, bool> = true>
    #endif
    constexpr decimal64_t(T1 coeff, T2 exp) noexcept;

    explicit constexpr decimal64_t(bool value) noexcept;

    // cmath functions that are easier as friends
    friend constexpr auto signbit     BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64_t rhs) noexcept -> bool;
    friend constexpr auto isnan       BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64_t rhs) noexcept -> bool;
    friend constexpr auto isinf       BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64_t rhs) noexcept -> bool;
    friend constexpr auto issignaling BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64_t rhs) noexcept -> bool;
    friend constexpr auto isnormal    BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64_t rhs) noexcept -> bool;
    friend constexpr auto isfinite    BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64_t rhs) noexcept -> bool;

    // 3.2.7 unary arithmetic operators:
    friend constexpr auto operator+(decimal64_t rhs) noexcept -> decimal64_t;
    friend constexpr auto operator-(decimal64_t rhs) noexcept -> decimal64_t;

    // 3.2.8 Binary arithmetic operators
    friend constexpr auto operator+(decimal64_t lhs, decimal64_t rhs) noexcept -> decimal64_t;

    template <typename Integer>
    friend constexpr auto operator+(decimal64_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t);

    template <typename Integer>
    friend constexpr auto operator+(Integer lhs, decimal64_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t);

    friend constexpr auto operator-(decimal64_t lhs, decimal64_t rhs) noexcept -> decimal64_t;

    template <typename Integer>
    friend constexpr auto operator-(decimal64_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t);

    template <typename Integer>
    friend constexpr auto operator-(Integer lhs, decimal64_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t);

    friend constexpr auto operator*(decimal64_t lhs, decimal64_t rhs) noexcept -> decimal64_t;

    template <typename Integer>
    friend constexpr auto operator*(decimal64_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t);

    template <typename Integer>
    friend constexpr auto operator*(Integer lhs, decimal64_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t);

    friend constexpr auto operator/(decimal64_t lhs, decimal64_t rhs) noexcept -> decimal64_t;

    template <typename Integer>
    friend constexpr auto operator/(decimal64_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t);

    template <typename Integer>
    friend constexpr auto operator/(Integer lhs, decimal64_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t);

    friend constexpr auto operator%(decimal64_t lhs, decimal64_t rhs) noexcept -> decimal64_t;

    // 3.2.3.5 Increment and Decrement
    constexpr auto operator++()    noexcept -> decimal64_t&;
    constexpr auto operator++(int) noexcept -> decimal64_t;  // NOLINT : C++14 so constexpr implies const
    constexpr auto operator--()    noexcept -> decimal64_t&;
    constexpr auto operator--(int) noexcept -> decimal64_t;  // NOLINT : C++14 so constexpr implies const

    // 3.2.3.6 Compound assignment
    constexpr auto operator+=(decimal64_t rhs) noexcept -> decimal64_t&;

    template <typename Integer>
    constexpr auto operator+=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t&);

    template <typename Decimal>
    constexpr auto operator+=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal64_t&);

    constexpr auto operator-=(decimal64_t rhs) noexcept -> decimal64_t&;

    template <typename Integer>
    constexpr auto operator-=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t&);

    template <typename Decimal>
    constexpr auto operator-=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal64_t&);

    constexpr auto operator*=(decimal64_t rhs) noexcept -> decimal64_t&;

    template <typename Integer>
    constexpr auto operator*=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t&);

    template <typename Decimal>
    constexpr auto operator*=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal64_t&);

    constexpr auto operator/=(decimal64_t rhs) noexcept -> decimal64_t&;

    template <typename Integer>
    constexpr auto operator/=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t&);

    template <typename Decimal>
    constexpr auto operator/=(Decimal rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal64_t&);

    constexpr auto operator%=(decimal64_t rhs) noexcept -> decimal64_t&;

    // 3.2.9 Comparison operators:
    // Equality
    friend constexpr auto operator==(decimal64_t lhs, decimal64_t rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator==(decimal64_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator==(Integer lhs, decimal64_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Inequality
    friend constexpr auto operator!=(decimal64_t lhs, decimal64_t rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator!=(decimal64_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator!=(Integer lhs, decimal64_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Less
    friend constexpr auto operator<(decimal64_t lhs, decimal64_t rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator<(decimal64_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator<(Integer lhs, decimal64_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Less equal
    friend constexpr auto operator<=(decimal64_t lhs, decimal64_t rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator<=(decimal64_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator<=(Integer lhs, decimal64_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Greater
    friend constexpr auto operator>(decimal64_t lhs, decimal64_t rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator>(decimal64_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator>(Integer lhs, decimal64_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // Greater equal
    friend constexpr auto operator>=(decimal64_t lhs, decimal64_t rhs) noexcept -> bool;

    template <typename Integer>
    friend constexpr auto operator>=(decimal64_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator>=(Integer lhs, decimal64_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // C++20 spaceship
    #ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR
    friend constexpr auto operator<=>(decimal64_t lhs, decimal64_t rhs) noexcept -> std::partial_ordering;

    template <typename Integer>
    friend constexpr auto operator<=>(decimal64_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering);

    template <typename Integer>
    friend constexpr auto operator<=>(Integer lhs, decimal64_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering);
    #endif

    // 3.6.4 Same Quantum
    friend constexpr auto samequantumd64(decimal64_t lhs, decimal64_t rhs) noexcept -> bool;

    // 3.6.5 Quantum exponent
    friend constexpr auto quantexpd64(decimal64_t x) noexcept -> int;

    // 3.6.6 Quantize
    friend constexpr auto quantized64(decimal64_t lhs, decimal64_t rhs) noexcept -> decimal64_t;

    // Bit-wise operators
    friend constexpr auto operator&(decimal64_t lhs, decimal64_t rhs) noexcept -> decimal64_t;

    template <typename Integer>
    friend constexpr auto operator&(decimal64_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t);

    template <typename Integer>
    friend constexpr auto operator&(Integer lhs, decimal64_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t);

    friend constexpr auto operator|(decimal64_t lhs, decimal64_t rhs) noexcept -> decimal64_t;

    template <typename Integer>
    friend constexpr auto operator|(decimal64_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t);

    template <typename Integer>
    friend constexpr auto operator|(Integer lhs, decimal64_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t);

    friend constexpr auto operator^(decimal64_t lhs, decimal64_t rhs) noexcept -> decimal64_t;

    template <typename Integer>
    friend constexpr auto operator^(decimal64_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t);

    template <typename Integer>
    friend constexpr auto operator^(Integer lhs, decimal64_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t);

    friend constexpr auto operator<<(decimal64_t lhs, decimal64_t rhs) noexcept -> decimal64_t;

    template <typename Integer>
    friend constexpr auto operator<<(decimal64_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t);

    template <typename Integer>
    friend constexpr auto operator<<(Integer lhs, decimal64_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t);

    friend constexpr auto operator>>(decimal64_t lhs, decimal64_t rhs) noexcept -> decimal64_t;

    template <typename Integer>
    friend constexpr auto operator>>(decimal64_t lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t);

    template <typename Integer>
    friend constexpr auto operator>>(Integer lhs, decimal64_t rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t);

    friend constexpr auto operator~(decimal64_t lhs) noexcept -> decimal64_t;

    // <cmath> functions that need to be friends
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
    friend constexpr auto frexp10(T num, int* expptr) noexcept -> typename T::significand_type;

    friend constexpr auto copysignd64(decimal64_t mag, decimal64_t sgn) noexcept -> decimal64_t;
    friend constexpr auto fmad64(decimal64_t x, decimal64_t y, decimal64_t z) noexcept -> decimal64_t;
    friend constexpr auto scalbnd64(decimal64_t num, int exp) noexcept -> decimal64_t;
    friend constexpr auto scalblnd64(decimal64_t num, long exp) noexcept -> decimal64_t;
};

[[deprecated("Use re-named type decimal64_t instead of decimal64")]] BOOST_DECIMAL_EXPORT using decimal64 = decimal64_t;

#if defined(__GNUC__) && __GNUC__ >= 8
#  pragma GCC diagnostic pop
#endif

constexpr auto from_bits(std::uint64_t bits) noexcept -> decimal64_t
{
    decimal64_t result;
    result.bits_ = bits;

    return result;
}

constexpr auto to_bits(decimal64_t rhs) noexcept -> std::uint64_t
{
    return rhs.bits_;
}

#if defined(__GNUC__) && __GNUC__ >= 6
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wduplicated-branches"
#  pragma GCC diagnostic ignored "-Wbool-compare"
#  pragma GCC diagnostic ignored "-Wconversion"
#endif

// 3.2.5 initialization from coefficient and exponent:
#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_UNSIGNED_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
#else
template <typename T1, typename T2, std::enable_if_t<detail::is_unsigned_v<T1> && detail::is_integral_v<T2>, bool>>
#endif
constexpr decimal64_t::decimal64_t(T1 coeff, T2 exp, bool sign) noexcept
{
    bits_ = sign ? detail::d64_sign_mask : UINT64_C(0);

    // If the coeff is not in range make it so
    int coeff_digits {-1};
    if (coeff > detail::d64_max_significand_value)
    {
        coeff_digits = detail::d64_constructor_num_digits(coeff);
        if (coeff_digits > detail::precision_v<decimal64_t> + 1)
        {
            const auto digits_to_remove {coeff_digits - (detail::precision_v<decimal64_t> + 1)};

            #if defined(__GNUC__) && !defined(__clang__)
            #  pragma GCC diagnostic push
            #  pragma GCC diagnostic ignored "-Wconversion"
            #endif

            coeff /= detail::pow10(static_cast<T1>(digits_to_remove));

            #if defined(__GNUC__) && !defined(__clang__)
            #  pragma GCC diagnostic pop
            #endif

            coeff_digits -= digits_to_remove;
            exp += detail::fenv_round<decimal64_t>(coeff, sign) + digits_to_remove;
        }
        else
        {
            exp += detail::fenv_round<decimal64_t>(coeff, sign);
        }
    }

    auto reduced_coeff {static_cast<significand_type>(coeff)};
    bool big_combination {false};

    if (reduced_coeff == 0U)
    {
        // Normalize our handling of zeros
        return;
    }

    if (reduced_coeff <= detail::d64_biggest_no_combination_significand)
    {
        // If the coefficient fits directly we don't need to use the combination field
        // bits_.significand = reduced_coeff;
        bits_ |= (reduced_coeff & detail::d64_not_11_significand_mask);
    }
    else
    {
        // Have to use the full combination field
        bits_ |= (detail::d64_combination_field_mask | (reduced_coeff & detail::d64_11_significand_mask));
        big_combination = true;
    }

    // If the exponent fits we do not need to use the combination field
    const auto biased_exp {static_cast<std::uint64_t>(exp + detail::bias_v<decimal64_t>)};
    if (biased_exp <= detail::d64_max_biased_exponent)
    {
        if (big_combination)
        {
            bits_ |= (biased_exp << detail::d64_11_exp_shift) & detail::d64_11_exp_mask;
        }
        else
        {
            bits_ |= (biased_exp << detail::d64_not_11_exp_shift) & detail::d64_not_11_exp_mask;
        }
    }
    else
    {
        // If we can fit the extra exponent in the significand then we can construct the value
        // If we can't the value is either 0 or infinity depending on the sign of exp

        if (coeff_digits == -1)
        {
            coeff_digits = detail::num_digits(reduced_coeff);
        }

        const auto exp_delta {biased_exp - detail::d64_max_biased_exponent};
        const auto digit_delta {coeff_digits - static_cast<int>(exp_delta)};
        if (digit_delta > 0 && coeff_digits + digit_delta <= detail::precision_v<decimal64_t>)
        {
            exp -= digit_delta;
            reduced_coeff *= detail::pow10(static_cast<significand_type>(digit_delta));
            *this = decimal64_t(reduced_coeff, exp, sign);
        }
        else
        {
            bits_ = exp < 0 ? UINT64_C(0) : detail::d64_inf_mask;
        }
    }
}

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_SIGNED_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
#else
template <typename T1, typename T2, std::enable_if_t<!detail::is_unsigned_v<T1> && detail::is_integral_v<T2>, bool>>
#endif
constexpr decimal64_t::decimal64_t(T1 coeff, T2 exp) noexcept : decimal64_t(detail::make_positive_unsigned(coeff), exp, coeff < 0) {}

constexpr decimal64_t::decimal64_t(bool value) noexcept : decimal64_t(static_cast<significand_type>(value), 0, false) {}


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
BOOST_DECIMAL_CXX20_CONSTEXPR decimal64_t::decimal64_t(Float val) noexcept
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

        if (components.exponent > detail::emax_v<decimal64_t>)
        {
            *this = from_bits(detail::d64_inf_mask);
        }
        else
        {
            *this = decimal64_t {components.mantissa, components.exponent, components.sign};
        }
    }
}

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

template <typename Float>
BOOST_DECIMAL_CXX20_CONSTEXPR auto decimal64_t::operator=(const Float& val) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_floating_point_v, Float, decimal64_t&)
{
    *this = decimal64_t{val};
    return *this;
}

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal>
#else
template <typename Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool>>
#endif
constexpr decimal64_t::decimal64_t(Decimal val) noexcept
{
    *this = to_decimal<decimal64_t>(val);
}

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_INTEGRAL Integer>
#else
template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool>>
#endif
constexpr decimal64_t::decimal64_t(Integer val) noexcept : decimal64_t{val, 0}
{
}

template <typename Integer>
constexpr auto decimal64_t::operator=(const Integer& val) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t&)
{
    using ConversionType = std::conditional_t<std::is_same<Integer, bool>::value, std::int32_t, Integer>;
    *this = decimal64_t{static_cast<ConversionType>(val), 0};
    return *this;
}

constexpr decimal64_t::operator bool() const noexcept
{
    constexpr decimal64_t zero {0, 0};
    return *this != zero;
}

constexpr decimal64_t::operator int() const noexcept
{
    return to_integral<decimal64_t, int>(*this);
}

constexpr decimal64_t::operator unsigned() const noexcept
{
    return to_integral<decimal64_t, unsigned>(*this);
}

constexpr decimal64_t::operator long() const noexcept
{
    return to_integral<decimal64_t, long>(*this);
}

constexpr decimal64_t::operator unsigned long() const noexcept
{
    return to_integral<decimal64_t, unsigned long>(*this);
}

constexpr decimal64_t::operator long long() const noexcept
{
    return to_integral<decimal64_t, long long>(*this);
}

constexpr decimal64_t::operator unsigned long long() const noexcept
{
    return to_integral<decimal64_t, unsigned long long>(*this);
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr decimal64_t::operator detail::builtin_int128_t() const noexcept
{
    return to_integral<decimal64_t, detail::builtin_int128_t>(*this);
}

constexpr decimal64_t::operator detail::builtin_uint128_t() const noexcept
{
    return to_integral<decimal64_t, detail::builtin_uint128_t>(*this);
}

#endif

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal> && (detail::decimal_val_v<Decimal> > detail::decimal_val_v<decimal64_t>), bool>>
constexpr decimal64_t::operator Decimal() const noexcept
{
    return to_decimal<Decimal>(*this);
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal> && (detail::decimal_val_v<Decimal> <= detail::decimal_val_v<decimal64_t>), bool>>
constexpr decimal64_t::operator Decimal() const noexcept
{
    return to_decimal<Decimal>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal64_t::operator float() const noexcept
{
    return to_float<decimal64_t, float>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal64_t::operator double() const noexcept
{
    return to_float<decimal64_t, double>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal64_t::operator long double() const noexcept
{
    return to_float<decimal64_t, long double>(*this);
}

#ifdef BOOST_DECIMAL_HAS_FLOAT16
constexpr decimal64_t::operator std::float16_t() const noexcept
{
    return static_cast<std::float16_t>(to_float<decimal64_t, float>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_FLOAT32
constexpr decimal64_t::operator std::float32_t() const noexcept
{
    return static_cast<std::float32_t>(to_float<decimal64_t, float>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_FLOAT64
constexpr decimal64_t::operator std::float64_t() const noexcept
{
    return static_cast<std::float64_t>(to_float<decimal64_t, double>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_BRAINFLOAT16
constexpr decimal64_t::operator std::bfloat16_t() const noexcept
{
    return static_cast<std::bfloat16_t>(to_float<decimal64_t, float>(*this));
}
#endif

constexpr auto decimal64_t::unbiased_exponent() const noexcept -> exponent_type
{
    exponent_type expval {};

    if ((bits_ & detail::d64_combination_field_mask) == detail::d64_combination_field_mask)
    {
        expval = (bits_ & detail::d64_11_exp_mask) >> detail::d64_11_exp_shift;
    }
    else
    {
        expval = (bits_ & detail::d64_not_11_exp_mask) >> detail::d64_not_11_exp_shift;
    }

    return expval;
}

constexpr auto decimal64_t::biased_exponent() const noexcept -> biased_exponent_type
{
    return static_cast<std::int32_t>(unbiased_exponent()) - detail::bias_v<decimal64_t>;
}

constexpr auto decimal64_t::full_significand() const noexcept -> significand_type
{
    significand_type significand {};

    if ((bits_ & detail::d64_combination_field_mask) == detail::d64_combination_field_mask)
    {
        constexpr std::uint64_t implied_bit {UINT64_C(0b1000'0000000000'0000000000'0000000000'0000000000'0000000000)};
        significand = implied_bit | (bits_ & detail::d64_11_significand_mask);
    }
    else
    {
        significand = bits_ & detail::d64_not_11_significand_mask;
    }

    return significand;
}

constexpr auto decimal64_t::isneg() const noexcept -> bool
{
    return static_cast<bool>(bits_ & detail::d64_sign_mask);
}

constexpr auto decimal64_t::to_components() const noexcept -> detail::decimal64_t_components
{
    detail::decimal64_t_components components {};

    exponent_type expval {};
    significand_type significand {};

    if ((bits_ & detail::d64_combination_field_mask) == detail::d64_combination_field_mask)
    {
        constexpr std::uint64_t implied_bit {UINT64_C(0b1000'0000000000'0000000000'0000000000'0000000000'0000000000)};
        significand = implied_bit | (bits_ & detail::d64_11_significand_mask);
        expval = (bits_ & detail::d64_11_exp_mask) >> detail::d64_11_exp_shift;
    }
    else
    {
        significand = bits_ & detail::d64_not_11_significand_mask;
        expval = (bits_ & detail::d64_not_11_exp_mask) >> detail::d64_not_11_exp_shift;
    }

    components.sig = significand;
    components.exp = static_cast<biased_exponent_type>(expval) - detail::bias_v<decimal64_t>;
    components.sign = bits_ & detail::d64_sign_mask;

    return components;
}

template <typename T>
constexpr auto decimal64_t::edit_exponent(T expval) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, T, void)
{
    *this = decimal64_t(this->full_significand(), expval, this->isneg());
}

constexpr auto decimal64_t::edit_sign(bool sign) noexcept -> void
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

constexpr auto signbit BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64_t rhs) noexcept -> bool
{
    return rhs.bits_ & detail::d64_sign_mask;
}

constexpr auto isnan BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64_t rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return (rhs.bits_ & detail::d64_nan_mask) == detail::d64_nan_mask;
    #else
    static_cast<void>(rhs);
    return false;
    #endif
}

constexpr auto isinf BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64_t rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return ((rhs.bits_ & detail::d64_nan_mask) == detail::d64_inf_mask);
    #else
    static_cast<void>(rhs);
    return false;
    #endif
}

constexpr auto issignaling BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64_t rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return (rhs.bits_ & detail::d64_snan_mask) == detail::d64_snan_mask;
    #else
    static_cast<void>(rhs);
    return false;
    #endif
}

constexpr auto isnormal BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64_t rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check for de-normals
    const auto sig {rhs.full_significand()};
    const auto exp {rhs.unbiased_exponent()};

    if (exp <= detail::precision_v<decimal64_t> - 1)
    {
        return false;
    }

    return (sig != 0) && isfinite(rhs);
    #else
    return rhs.full_significand() != 0;
    #endif
}

constexpr auto isfinite BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (decimal64_t rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return ((rhs.bits_ & detail::d64_inf_mask) != detail::d64_inf_mask);
    #else
    static_cast<void>(rhs);
    return true;
    #endif
}

BOOST_DECIMAL_FORCE_INLINE constexpr auto not_finite(decimal64_t rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return ((rhs.bits_ & detail::d64_inf_mask) == detail::d64_inf_mask);
    #else
    static_cast<void>(rhs);
    return false;
    #endif
}

constexpr auto operator+(decimal64_t rhs) noexcept -> decimal64_t
{
    return rhs;
}

constexpr auto operator-(decimal64_t rhs) noexcept-> decimal64_t
{
    rhs.bits_ ^= detail::d64_sign_mask;
    return rhs;
}

constexpr auto d64_div_impl(decimal64_t lhs, decimal64_t rhs, decimal64_t& q, decimal64_t& r) noexcept -> void
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check pre-conditions
    constexpr decimal64_t zero {0, 0};
    constexpr decimal64_t nan {boost::decimal::from_bits(boost::decimal::detail::d64_snan_mask)};
    constexpr decimal64_t inf {boost::decimal::from_bits(boost::decimal::detail::d64_inf_mask)};

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
    detail::normalize<decimal64_t>(sig_lhs, exp_lhs);

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize<decimal64_t>(sig_rhs, exp_rhs);

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "sig lhs: " << sig_lhs
              << "\nexp lhs: " << exp_lhs
              << "\nsig rhs: " << sig_rhs
              << "\nexp rhs: " << exp_rhs << std::endl;
    #endif

    detail::decimal64_t_components lhs_components {sig_lhs, exp_lhs, lhs.isneg()};
    detail::decimal64_t_components rhs_components {sig_rhs, exp_rhs, rhs.isneg()};

    q = detail::d64_generic_div_impl<decimal64_t>(lhs_components, rhs_components);
}

constexpr auto d64_mod_impl(decimal64_t lhs, decimal64_t rhs, const decimal64_t& q, decimal64_t& r) noexcept -> void
{
    constexpr decimal64_t zero {0, 0};

    // https://en.cppreference.com/w/cpp/numeric/math/fmod
    auto q_trunc {q > zero ? floor(q) : ceil(q)};
    r = lhs - (decimal64_t(q_trunc) * rhs);
}

constexpr auto operator+(decimal64_t lhs, decimal64_t rhs) noexcept -> decimal64_t
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs) || not_finite(rhs))
    {
        return detail::check_non_finite(lhs, rhs);
    }
    #endif

    auto lhs_components {lhs.to_components()};
    detail::normalize<decimal64_t>(lhs_components.sig, lhs_components.exp);
    auto rhs_components {rhs.to_components()};
    detail::normalize<decimal64_t>(rhs_components.sig, rhs_components.exp);

    return detail::d64_add_impl<decimal64_t>(lhs_components, rhs_components);
}

template <typename Integer>
constexpr auto operator+(decimal64_t lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t)
{
    using promoted_significand_type = detail::promote_significand_t<decimal64_t, Integer>;
    using exp_type = decimal64_t::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs))
    {
        return lhs;
    }
    #endif

    auto sig_rhs {static_cast<promoted_significand_type>(detail::make_positive_unsigned(rhs))};
    bool abs_lhs_bigger {abs(lhs) > sig_rhs};

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize<decimal64_t>(sig_lhs, exp_lhs);

    exp_type exp_rhs {0};
    detail::normalize<decimal64_t>(sig_rhs, exp_rhs);
    const auto final_sig_rhs {static_cast<decimal64_t::significand_type>(sig_rhs)};

    return detail::d64_add_impl<decimal64_t>(sig_lhs, exp_lhs, lhs.isneg(),
                                           final_sig_rhs, exp_rhs, (rhs < 0),
                                           abs_lhs_bigger);
}

template <typename Integer>
constexpr auto operator+(Integer lhs, decimal64_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t)
{
    return rhs + lhs;
}

// NOLINTNEXTLINE : If subtraction is actually addition than use operator+ and vice versa
constexpr auto operator-(decimal64_t lhs, decimal64_t rhs) noexcept -> decimal64_t
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs) || not_finite(rhs))
    {
        return detail::check_non_finite(lhs, rhs);
    }
    #endif

    auto lhs_components {lhs.to_components()};
    detail::normalize<decimal64_t>(lhs_components.sig, lhs_components.exp);
    auto rhs_components {rhs.to_components()};
    detail::normalize<decimal64_t>(rhs_components.sig, rhs_components.exp);
    rhs_components.sign = !rhs_components.sign;

    return detail::d64_add_impl<decimal64_t>(lhs_components, rhs_components);
}

template <typename Integer>
constexpr auto operator-(decimal64_t lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t)
{
    using promoted_significand_type = detail::promote_significand_t<decimal64_t, Integer>;
    using exp_type = decimal64_t::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs))
    {
        return lhs;
    }
    #endif

    auto sig_rhs {static_cast<promoted_significand_type>(detail::make_positive_unsigned(rhs))};
    const bool abs_lhs_bigger {abs(lhs) > sig_rhs};

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize<decimal64_t>(sig_lhs, exp_lhs);

    exp_type exp_rhs {0};
    detail::normalize<decimal64_t>(sig_rhs, exp_rhs);
    const auto final_sig_rhs {static_cast<decimal64_t::significand_type>(sig_rhs)};

    return detail::d64_add_impl<decimal64_t>(sig_lhs, exp_lhs, lhs.isneg(),
                                           final_sig_rhs, exp_rhs, !(rhs < 0),
                                           abs_lhs_bigger);
}

template <typename Integer>
constexpr auto operator-(Integer lhs, decimal64_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t)
{
    using promoted_significand_type = detail::promote_significand_t<decimal64_t, Integer>;
    using exp_type = decimal64_t::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(rhs))
    {
        return rhs;
    }
    #endif

    auto sig_lhs {static_cast<promoted_significand_type>(detail::make_positive_unsigned(lhs))};
    const bool abs_lhs_bigger {sig_lhs > abs(rhs)};

    exp_type exp_lhs {0};
    detail::normalize<decimal64_t>(sig_lhs, exp_lhs);
    const auto final_sig_lhs {static_cast<decimal64_t::significand_type>(detail::make_positive_unsigned(sig_lhs))};

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize<decimal64_t>(sig_rhs, exp_rhs);

    return detail::d64_add_impl<decimal64_t>(final_sig_lhs, exp_lhs, (lhs < 0),
                                           sig_rhs, exp_rhs, !rhs.isneg(),
                                           abs_lhs_bigger);
}

constexpr auto operator*(decimal64_t lhs, decimal64_t rhs) noexcept -> decimal64_t
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs) || not_finite(rhs))
    {
        return detail::check_non_finite(lhs, rhs);
    }
    #endif

    auto lhs_components {lhs.to_components()};
    detail::normalize<decimal64_t>(lhs_components.sig, lhs_components.exp);
    auto rhs_components {rhs.to_components()};
    detail::normalize<decimal64_t>(rhs_components.sig, rhs_components.exp);

    return detail::d64_mul_impl<decimal64_t>(lhs_components, rhs_components);
}

template <typename Integer>
constexpr auto operator*(decimal64_t lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t)
{
    using promoted_significand_type = detail::promote_significand_t<decimal64_t, Integer>;
    using exp_type = decimal64_t::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs))
    {
        return lhs;
    }
    #endif

    auto lhs_sig {lhs.full_significand()};
    auto lhs_exp {lhs.biased_exponent()};
    detail::normalize<decimal64_t>(lhs_sig, lhs_exp);

    auto rhs_sig {static_cast<promoted_significand_type>(detail::make_positive_unsigned(rhs))};
    exp_type rhs_exp {0};
    detail::normalize<decimal64_t>(rhs_sig, rhs_exp);
    const auto final_rhs_sig {static_cast<decimal64_t::significand_type>(rhs_sig)};

    return detail::d64_mul_impl<decimal64_t>(lhs_sig, lhs_exp, lhs.isneg(),
                                           final_rhs_sig, rhs_exp, (rhs < 0));
}

template <typename Integer>
constexpr auto operator*(Integer lhs, decimal64_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t)
{
    return rhs * lhs;
}

constexpr auto operator/(decimal64_t lhs, decimal64_t rhs) noexcept -> decimal64_t
{
    decimal64_t q {};
    decimal64_t r {};
    d64_div_impl(lhs, rhs, q, r);

    return q;
}

template <typename Integer>
constexpr auto operator/(decimal64_t lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t)
{
    using sig_type = decimal64_t::significand_type;
    using exp_type = decimal64_t::biased_exponent_type;
    using integer_type = std::conditional_t<(std::numeric_limits<Integer>::digits10 > std::numeric_limits<sig_type>::digits10), detail::make_unsigned_t<Integer>, sig_type>;

    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check pre-conditions
    constexpr decimal64_t zero {0, 0};
    constexpr decimal64_t nan {boost::decimal::from_bits(boost::decimal::detail::d64_snan_mask)};
    constexpr decimal64_t inf {boost::decimal::from_bits(boost::decimal::detail::d64_inf_mask)};

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
    detail::normalize<decimal64_t>(lhs_sig, lhs_exp);
    detail::decimal64_t_components lhs_components {lhs_sig, lhs_exp, lhs.isneg()};

    auto rhs_sig {static_cast<integer_type>(detail::make_positive_unsigned(rhs))};
    exp_type rhs_exp {};
    detail::normalize<decimal64_t>(rhs_sig, rhs_exp);
    detail::decimal64_t_components rhs_components {static_cast<sig_type>(rhs_sig), rhs_exp, rhs < 0};

    return detail::d64_generic_div_impl<decimal64_t>(lhs_components, rhs_components);
}

template <typename Integer>
constexpr auto operator/(Integer lhs, decimal64_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t)
{
    using sig_type = decimal64_t::significand_type;
    using exp_type = decimal64_t::biased_exponent_type;
    using integer_type = std::conditional_t<(std::numeric_limits<Integer>::digits10 > std::numeric_limits<sig_type>::digits10), detail::make_unsigned_t<Integer>, sig_type>;

    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check pre-conditions
    constexpr decimal64_t zero {0, 0};
    constexpr decimal64_t inf {boost::decimal::from_bits(boost::decimal::detail::d64_inf_mask)};
    constexpr decimal64_t nan {boost::decimal::from_bits(boost::decimal::detail::d64_snan_mask)};

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
    detail::normalize<decimal64_t>(rhs_sig, rhs_exp);

    exp_type lhs_exp {};
    auto lhs_sig {static_cast<integer_type>(detail::make_positive_unsigned(lhs))};
    detail::normalize<decimal64_t>(lhs_sig, lhs_exp);
    detail::decimal64_t_components lhs_components {static_cast<sig_type>(lhs_sig), lhs_exp, lhs < 0};
    detail::decimal64_t_components rhs_components {rhs_sig, rhs_exp, rhs.isneg()};

    return detail::d64_generic_div_impl<decimal64_t>(lhs_components, rhs_components);
}

constexpr auto operator%(decimal64_t lhs, decimal64_t rhs) noexcept -> decimal64_t
{
    decimal64_t q {};
    decimal64_t r {};
    d64_div_impl(lhs, rhs, q, r);
    d64_mod_impl(lhs, rhs, q, r);

    return r;
}

constexpr auto decimal64_t::operator++() noexcept -> decimal64_t&
{
    constexpr decimal64_t one{1, 0};
    *this = *this + one;
    return *this;
}

constexpr auto decimal64_t::operator++(int) noexcept -> decimal64_t
{
    return ++(*this);
}

constexpr auto decimal64_t::operator--() noexcept -> decimal64_t&
{
    constexpr decimal64_t one{1, 0};
    *this = *this - one;
    return *this;
}

constexpr auto decimal64_t::operator--(int) noexcept -> decimal64_t
{
    return --(*this);
}

constexpr auto decimal64_t::operator+=(decimal64_t rhs) noexcept -> decimal64_t&
{
    *this = *this + rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal64_t::operator+=(Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t&)
{
    *this = *this + rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal64_t::operator+=(Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal64_t&)
{
    *this = *this + rhs;
    return *this;
}

constexpr auto decimal64_t::operator-=(decimal64_t rhs) noexcept -> decimal64_t&
{
    *this = *this - rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal64_t::operator-=(Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t&)
{
    *this = *this - rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal64_t::operator-=(Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal64_t&)
{
    *this = *this - rhs;
    return *this;
}

constexpr auto decimal64_t::operator*=(decimal64_t rhs) noexcept -> decimal64_t&
{
    *this = *this * rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal64_t::operator*=(Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t&)
{
    *this = *this * rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal64_t::operator*=(Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal64_t&)
{
    *this = *this * rhs;
    return *this;
}

constexpr auto decimal64_t::operator/=(decimal64_t rhs) noexcept -> decimal64_t&
{
    *this = *this / rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal64_t::operator/=(Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t&)
{
    *this = *this / rhs;
    return *this;
}

template <typename Decimal>
constexpr auto decimal64_t::operator/=(Decimal rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, Decimal, decimal64_t&)
{
    *this = *this / rhs;
    return *this;
}

constexpr auto decimal64_t::operator%=(decimal64_t rhs) noexcept -> decimal64_t&
{
    *this = *this % rhs;
    return *this;
}

constexpr auto operator==(decimal64_t lhs, decimal64_t rhs) noexcept -> bool
{
    return equality_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator==(decimal64_t lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return mixed_equality_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator==(Integer lhs, decimal64_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return mixed_equality_impl(rhs, lhs);
}

constexpr auto operator!=(decimal64_t lhs, decimal64_t rhs) noexcept -> bool
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator!=(decimal64_t lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator!=(Integer lhs, decimal64_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return !(lhs == rhs);
}

constexpr auto operator<(decimal64_t lhs, decimal64_t rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs) || not_finite(rhs))
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
            return !rhs.isneg();
        }
    }
    #endif

    return sequential_less_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator<(decimal64_t lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return less_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator<(Integer lhs, decimal64_t rhs) noexcept
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

constexpr auto operator<=(decimal64_t lhs, decimal64_t rhs) noexcept -> bool
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
constexpr auto operator<=(decimal64_t lhs, Integer rhs) noexcept
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
constexpr auto operator<=(Integer lhs, decimal64_t rhs) noexcept
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

constexpr auto operator>(decimal64_t lhs, decimal64_t rhs) noexcept -> bool
{
    return rhs < lhs;
}

template <typename Integer>
constexpr auto operator>(decimal64_t lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return rhs < lhs;
}

template <typename Integer>
constexpr auto operator>(Integer lhs, decimal64_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return rhs < lhs;
}

constexpr auto operator>=(decimal64_t lhs, decimal64_t rhs) noexcept -> bool
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
constexpr auto operator>=(decimal64_t lhs, Integer rhs) noexcept
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
constexpr auto operator>=(Integer lhs, decimal64_t rhs) noexcept
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

constexpr auto operator<=>(decimal64_t lhs, decimal64_t rhs) noexcept -> std::partial_ordering
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
constexpr auto operator<=>(decimal64_t lhs, Integer rhs) noexcept
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
constexpr auto operator<=>(Integer lhs, decimal64_t rhs) noexcept
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

constexpr auto operator&(decimal64_t lhs, decimal64_t rhs) noexcept -> decimal64_t
{
    return from_bits(lhs.bits_ & rhs.bits_);
}

template <typename Integer>
constexpr auto operator&(decimal64_t lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t)
{
    return from_bits(lhs.bits_ & static_cast<std::uint64_t>(rhs));
}

template <typename Integer>
constexpr auto operator&(Integer lhs, decimal64_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t)
{
    return from_bits(static_cast<std::uint64_t>(lhs) & rhs.bits_);
}

constexpr auto operator|(decimal64_t lhs, decimal64_t rhs) noexcept -> decimal64_t
{
    return from_bits(lhs.bits_ | rhs.bits_);
}

template <typename Integer>
constexpr auto operator|(decimal64_t lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t)
{
    return from_bits(lhs.bits_ | static_cast<std::uint64_t>(rhs));
}

template <typename Integer>
constexpr auto operator|(Integer lhs, decimal64_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t)
{
    return from_bits(static_cast<std::uint64_t>(lhs) | rhs.bits_);
}

constexpr auto operator^(decimal64_t lhs, decimal64_t rhs) noexcept -> decimal64_t
{
    return from_bits(lhs.bits_ ^ rhs.bits_);
}

template <typename Integer>
constexpr auto operator^(decimal64_t lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t)
{
    return from_bits(lhs.bits_ ^ static_cast<std::uint64_t>(rhs));
}

template <typename Integer>
constexpr auto operator^(Integer lhs, decimal64_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t)
{
    return from_bits(static_cast<std::uint64_t>(lhs) ^ rhs.bits_);
}

constexpr auto operator<<(decimal64_t lhs, decimal64_t rhs) noexcept -> decimal64_t
{
    return from_bits(lhs.bits_ << rhs.bits_);
}

template <typename Integer>
constexpr auto operator<<(decimal64_t lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t)
{
    return from_bits(lhs.bits_ << static_cast<std::uint64_t>(rhs));
}

template <typename Integer>
constexpr auto operator<<(Integer lhs, decimal64_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t)
{
    return from_bits(static_cast<std::uint64_t>(lhs) << rhs.bits_);
}

constexpr auto operator>>(decimal64_t lhs, decimal64_t rhs) noexcept -> decimal64_t
{
    return from_bits(lhs.bits_ >> rhs.bits_);
}

template <typename Integer>
constexpr auto operator>>(decimal64_t lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t)
{
    return from_bits(lhs.bits_ >> static_cast<std::uint64_t>(rhs));
}

template <typename Integer>
constexpr auto operator>>(Integer lhs, decimal64_t rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_t)
{
    return from_bits(static_cast<std::uint64_t>(lhs) >> rhs.bits_);
}

constexpr auto operator~(decimal64_t lhs) noexcept -> decimal64_t
{
    return from_bits(~lhs.bits_);
}

// 3.6.4
// Effects: determines if the quantum exponents of x and y are the same.
// If both x and y are NaN, or infinity, they have the same quantum exponents;
// if exactly one operand is infinity or exactly one operand is NaN, they do not have the same quantum exponents.
// The samequantum functions raise no exception.
constexpr auto samequantumd64(decimal64_t lhs, decimal64_t rhs) noexcept -> bool
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
constexpr auto quantexpd64(decimal64_t x) noexcept -> int
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(x))
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
constexpr auto quantized64(decimal64_t lhs, decimal64_t rhs) noexcept -> decimal64_t
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

constexpr auto scalblnd64(decimal64_t num, long exp) noexcept -> decimal64_t
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    constexpr decimal64_t zero {0, 0};

    if (num == zero || exp == 0 || not_finite(num))
    {
        return num;
    }
    #endif

    num.edit_exponent(num.biased_exponent() + exp);

    return num;
}

constexpr auto scalbnd64(decimal64_t num, int expval) noexcept -> decimal64_t
{
    return scalblnd64(num, static_cast<long>(expval));
}

constexpr auto copysignd64(decimal64_t mag, decimal64_t sgn) noexcept -> decimal64_t
{
    mag.edit_sign(sgn.isneg());
    return mag;
}

} //namespace decimal
} //namespace boost

namespace std {

BOOST_DECIMAL_EXPORT template <>
#ifdef _MSC_VER
class numeric_limits<boost::decimal::decimal64_t>
#else
struct numeric_limits<boost::decimal::decimal64_t>
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
    static constexpr int  digits = 16;
    static constexpr int  digits10 = digits;
    static constexpr int  max_digits10 = digits;
    static constexpr int  radix = 10;
    static constexpr int  min_exponent = -383;
    static constexpr int  min_exponent10 = min_exponent;
    static constexpr int  max_exponent = 384;
    static constexpr int  max_exponent10 = max_exponent;
    static constexpr bool traps = numeric_limits<std::uint64_t>::traps;
    static constexpr bool tinyness_before = true;

    // Member functions
    static constexpr auto (min)        () -> boost::decimal::decimal64_t { return {UINT32_C(1), min_exponent}; }
    static constexpr auto (max)        () -> boost::decimal::decimal64_t { return {boost::decimal::detail::d64_max_significand_value, max_exponent - digits + 1}; }
    static constexpr auto lowest       () -> boost::decimal::decimal64_t { return {boost::decimal::detail::d64_max_significand_value, max_exponent - digits + 1, true}; }
    static constexpr auto epsilon      () -> boost::decimal::decimal64_t { return {UINT32_C(1), -digits + 1}; }
    static constexpr auto round_error  () -> boost::decimal::decimal64_t { return epsilon(); }
    static constexpr auto infinity     () -> boost::decimal::decimal64_t { return boost::decimal::from_bits(boost::decimal::detail::d64_inf_mask); }
    static constexpr auto quiet_NaN    () -> boost::decimal::decimal64_t { return boost::decimal::from_bits(boost::decimal::detail::d64_nan_mask); }
    static constexpr auto signaling_NaN() -> boost::decimal::decimal64_t { return boost::decimal::from_bits(boost::decimal::detail::d64_snan_mask); }
    static constexpr auto denorm_min   () -> boost::decimal::decimal64_t { return {1, boost::decimal::detail::etiny_v<boost::decimal::decimal64_t>}; }
};

} // Namespace std

#endif //BOOST_DECIMAL_decimal64_t_HPP
