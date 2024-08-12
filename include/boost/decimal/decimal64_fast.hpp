// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DECIMAL64_FAST_HPP
#define BOOST_DECIMAL_DECIMAL64_FAST_HPP

#include <boost/decimal/decimal64.hpp>
#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/add_impl.hpp>
#include <boost/decimal/detail/sub_impl.hpp>
#include <boost/decimal/detail/mul_impl.hpp>
#include <boost/decimal/detail/div_impl.hpp>
#include <boost/decimal/detail/promote_significand.hpp>
#include <boost/decimal/detail/ryu/ryu_generic_128.hpp>
#include <limits>
#include <cstdint>

namespace boost {
namespace decimal {

namespace detail {

BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d64_fast_inf = std::numeric_limits<std::uint_fast64_t>::max();
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d64_fast_qnan = std::numeric_limits<std::uint_fast64_t>::max() - 1;
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d64_fast_snan = std::numeric_limits<std::uint_fast64_t>::max() - 2;

struct decimal64_fast_components
{
    using significand_type = std::uint_fast64_t;
    using biased_exponent_type = std::int_fast32_t;

    significand_type sig;
    biased_exponent_type exp;
    bool sign;
};

} // namespace detail

class decimal64_fast final
{
public:
    using significand_type = std::uint_fast64_t;
    using exponent_type = std::uint_fast16_t;
    using biased_exponent_type = std::int_fast32_t;

private:
    // In regular decimal64 we have to decode the significand end exponent
    // Here we will store them directly to avoid the overhead of decoding

    significand_type significand_ {};
    exponent_type exponent_ {};
    bool sign_ {};

    BOOST_DECIMAL_GPU_ENABLED constexpr auto isneg() const noexcept -> bool
    {
        return sign_;
    }

    BOOST_DECIMAL_GPU_ENABLED constexpr auto full_significand() const noexcept -> significand_type
    {
        return significand_;
    }

    BOOST_DECIMAL_GPU_ENABLED constexpr auto unbiased_exponent() const noexcept -> exponent_type
    {
        return exponent_;
    }

    BOOST_DECIMAL_GPU_ENABLED constexpr auto biased_exponent() const noexcept -> biased_exponent_type
    {
        return static_cast<biased_exponent_type>(exponent_) - detail::bias_v<decimal64>;
    }

    // Equality template between any integer type and decimal32
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, BOOST_DECIMAL_INTEGRAL Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto mixed_equality_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto mixed_decimal_equality_impl(Decimal1 lhs, Decimal2 rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                             detail::is_decimal_floating_point_v<Decimal2>), bool>;

    // Template to compare operator< for any integer type and decimal32
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, BOOST_DECIMAL_INTEGRAL Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto less_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto mixed_decimal_less_impl(Decimal1 lhs, Decimal2 rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                             detail::is_decimal_floating_point_v<Decimal2>), bool>;

    template <typename Decimal, typename TargetType>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto to_integral(Decimal val) noexcept
        BOOST_DECIMAL_REQUIRES_TWO_RETURN(detail::is_decimal_floating_point_v, Decimal, detail::is_integral_v, TargetType, TargetType);

    template <typename Decimal, typename TargetType>
    friend BOOST_DECIMAL_CXX20_CONSTEXPR auto to_float(Decimal val) noexcept
        BOOST_DECIMAL_REQUIRES_TWO_RETURN(detail::is_decimal_floating_point_v, Decimal, detail::is_floating_point_v, TargetType, TargetType);

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetType, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto to_decimal(Decimal val) noexcept -> TargetType;

    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto d64_fast_div_impl(decimal64_fast lhs, decimal64_fast rhs, decimal64_fast& q, decimal64_fast& r) noexcept -> void;

    template <typename T>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto ilogb(T d) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, T, int);

    template <typename T>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto logb(T num) noexcept
        BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T);

    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto not_finite(decimal64_fast val) noexcept -> bool;

public:
    BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast() noexcept = default;

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
    #else
    template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1> && detail::is_integral_v<T2>, bool> = true>
    #endif
    BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast(T1 coeff, T2 exp, bool sign = false) noexcept;

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_INTEGRAL Integer>
    #else
    template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool> = true>
    #endif
    BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast(Integer val) noexcept;

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_REAL Float>
    #else
    template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool> = true>
    #endif
    explicit BOOST_DECIMAL_CXX20_CONSTEXPR decimal64_fast(Float val) noexcept;

    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto direct_init_d64(decimal64_fast::significand_type significand, decimal64_fast::exponent_type exponent, bool sign) noexcept -> decimal64_fast;

    // Classification functions
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto signbit(decimal64_fast val) noexcept -> bool;
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto isinf(decimal64_fast val) noexcept -> bool;
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto isnan(decimal64_fast val) noexcept -> bool;
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto issignaling(decimal64_fast val) noexcept -> bool;
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto isnormal(decimal64_fast val) noexcept -> bool;
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto isfinite(decimal64_fast val) noexcept -> bool;

    // Comparison operator
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator==(decimal64_fast lhs, decimal64_fast rhs) noexcept -> bool;
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator!=(decimal64_fast lhs, decimal64_fast rhs) noexcept -> bool;
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator<(decimal64_fast lhs, decimal64_fast rhs) noexcept -> bool;
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator<=(decimal64_fast lhs, decimal64_fast rhs) noexcept -> bool;
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator>(decimal64_fast lhs, decimal64_fast rhs) noexcept -> bool;
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator>=(decimal64_fast lhs, decimal64_fast rhs) noexcept -> bool;

    // Mixed type comparison operators
    template <typename Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator==(decimal64_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator==(Integer lhs, decimal64_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator!=(decimal64_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator!=(Integer lhs, decimal64_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator<(decimal64_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator<(Integer lhs, decimal64_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator<=(decimal64_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator<=(Integer lhs, decimal64_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator>(decimal64_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator>(Integer lhs, decimal64_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator>=(decimal64_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator>=(Integer lhs, decimal64_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    // C++20 Spaceship operator
    #ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator<=>(decimal64_fast lhs, decimal64_fast rhs) noexcept -> std::partial_ordering;

    template <typename Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator<=>(decimal64_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering);

    template <typename Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator<=>(Integer lhs, decimal64_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering);
    #endif

    // Conversions
    BOOST_DECIMAL_GPU_ENABLED explicit constexpr operator bool() const noexcept;
    BOOST_DECIMAL_GPU_ENABLED explicit constexpr operator int() const noexcept;
    BOOST_DECIMAL_GPU_ENABLED explicit constexpr operator unsigned() const noexcept;
    BOOST_DECIMAL_GPU_ENABLED explicit constexpr operator long() const noexcept;
    BOOST_DECIMAL_GPU_ENABLED explicit constexpr operator unsigned long() const noexcept;
    BOOST_DECIMAL_GPU_ENABLED explicit constexpr operator long long() const noexcept;
    BOOST_DECIMAL_GPU_ENABLED explicit constexpr operator unsigned long long() const noexcept;
    BOOST_DECIMAL_GPU_ENABLED explicit constexpr operator std::int8_t() const noexcept;
    BOOST_DECIMAL_GPU_ENABLED explicit constexpr operator std::uint8_t() const noexcept;
    BOOST_DECIMAL_GPU_ENABLED explicit constexpr operator std::int16_t() const noexcept;
    BOOST_DECIMAL_GPU_ENABLED explicit constexpr operator std::uint16_t() const noexcept;

    #ifdef BOOST_DECIMAL_HAS_INT128
    BOOST_DECIMAL_GPU_ENABLED explicit constexpr operator detail::int128_t() const noexcept;
    BOOST_DECIMAL_GPU_ENABLED explicit constexpr operator detail::uint128_t() const noexcept;
    #endif

    explicit BOOST_DECIMAL_CXX20_CONSTEXPR operator float() const noexcept;
    explicit BOOST_DECIMAL_CXX20_CONSTEXPR operator double() const noexcept;
    explicit BOOST_DECIMAL_CXX20_CONSTEXPR operator long double() const noexcept;

    #ifdef BOOST_DECIMAL_HAS_FLOAT16
    BOOST_DECIMAL_GPU_ENABLED explicit constexpr operator std::float16_t() const noexcept;
    #endif
    #ifdef BOOST_DECIMAL_HAS_FLOAT32
    BOOST_DECIMAL_GPU_ENABLED explicit constexpr operator std::float32_t() const noexcept;
    #endif
    #ifdef BOOST_DECIMAL_HAS_FLOAT64
    BOOST_DECIMAL_GPU_ENABLED explicit constexpr operator std::float64_t() const noexcept;
    #endif
    #ifdef BOOST_DECIMAL_HAS_BRAINFLOAT16
    BOOST_DECIMAL_GPU_ENABLED explicit constexpr operator std::bfloat16_t() const noexcept;
    #endif

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool> = true>
    BOOST_DECIMAL_GPU_ENABLED explicit constexpr operator Decimal() const noexcept;

    // Unary Operators
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator+(decimal64_fast val) noexcept -> decimal64_fast;
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator-(decimal64_fast val) noexcept -> decimal64_fast;

    // Basic arithmetic operators
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator+(decimal64_fast lhs, decimal64_fast rhs) noexcept -> decimal64_fast;
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator-(decimal64_fast lhs, decimal64_fast rhs) noexcept -> decimal64_fast;
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator*(decimal64_fast lhs, decimal64_fast rhs) noexcept -> decimal64_fast;
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator/(decimal64_fast lhs, decimal64_fast rhs) noexcept -> decimal64_fast;
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator%(decimal64_fast lhs, decimal64_fast rhs) noexcept -> decimal64_fast;

    // Mixed type arithmetic operators
    template <typename Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator+(decimal64_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast);

    template <typename Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator+(Integer lhs, decimal64_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast);

    template <typename Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator-(decimal64_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast);

    template <typename Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator-(Integer lhs, decimal64_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast);

    template <typename Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator*(decimal64_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast);

    template <typename Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator*(Integer lhs, decimal64_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast);

    template <typename Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator/(decimal64_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast);

    template <typename Integer>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto operator/(Integer lhs, decimal64_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast);

    // Compound Operators
    BOOST_DECIMAL_GPU_ENABLED constexpr auto operator+=(decimal64_fast rhs) noexcept -> decimal64_fast&;
    BOOST_DECIMAL_GPU_ENABLED constexpr auto operator-=(decimal64_fast rhs) noexcept -> decimal64_fast&;
    BOOST_DECIMAL_GPU_ENABLED constexpr auto operator*=(decimal64_fast rhs) noexcept -> decimal64_fast&;
    BOOST_DECIMAL_GPU_ENABLED constexpr auto operator/=(decimal64_fast rhs) noexcept -> decimal64_fast&;

    // Mixed type compound operators
    template <typename Integer>
    BOOST_DECIMAL_GPU_ENABLED constexpr auto operator+=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast&);

    template <typename Integer>
    BOOST_DECIMAL_GPU_ENABLED constexpr auto operator-=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast&);

    template <typename Integer>
    BOOST_DECIMAL_GPU_ENABLED constexpr auto operator*=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast&);

    template <typename Integer>
    BOOST_DECIMAL_GPU_ENABLED constexpr auto operator/=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast&);

    // Increment and decrement
    BOOST_DECIMAL_GPU_ENABLED constexpr auto operator++() noexcept -> decimal64_fast&;
    BOOST_DECIMAL_GPU_ENABLED constexpr auto operator++(int) noexcept -> decimal64_fast&;
    BOOST_DECIMAL_GPU_ENABLED constexpr auto operator--() noexcept -> decimal64_fast&;
    BOOST_DECIMAL_GPU_ENABLED constexpr auto operator--(int) noexcept -> decimal64_fast&;
    
    // Cmath friend functions
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto frexp10(T num, int* expptr) noexcept -> typename T::significand_type;

    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto copysignd64f(decimal64_fast mag, decimal64_fast sgn) noexcept -> decimal64_fast;
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto fmad64f(decimal64_fast x, decimal64_fast y, decimal64_fast z) noexcept -> decimal64_fast;
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto scalbnd64f(decimal64_fast num, int exp) noexcept -> decimal64_fast;
    friend BOOST_DECIMAL_GPU_ENABLED constexpr auto scalblnd64f(decimal64_fast num, long exp) noexcept -> decimal64_fast;
};

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
#else
template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1> && detail::is_integral_v<T2>, bool>>
#endif
BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast::decimal64_fast(T1 coeff, T2 exp, bool sign) noexcept
{
    // Older compilers have issues with conversions from __uint128, so we skip all that and use our uint128
    #if defined(BOOST_DECIMAL_HAS_INT128) && (!defined(__GNUC__) || (defined(__GNUC__) && !defined(__clang__) && __GNUC__ < 10)) && (!defined(__clang__) || (defined(__clang__) && __clang_major__ < 13))
    using Unsigned_Integer_1 = detail::make_unsigned_t<T1>;
    using Unsigned_Integer = std::conditional_t<std::is_same<Unsigned_Integer_1, detail::uint128_t>::value, detail::uint128, Unsigned_Integer_1>;
    #else
    using Unsigned_Integer = detail::make_unsigned_t<T1>;
    #endif

    using Basis_Unsigned_Integer = std::conditional_t<std::numeric_limits<Unsigned_Integer>::digits10 < std::numeric_limits<significand_type>::digits10, significand_type, Unsigned_Integer>;

    const bool isneg {coeff < static_cast<T1>(0) || sign};
    sign_ = isneg;
    auto unsigned_coeff {static_cast<Basis_Unsigned_Integer>(detail::make_positive_unsigned(coeff))};

    // Normalize the value, so we don't have to worrya bout it with operations
    detail::normalize<decimal64>(unsigned_coeff, exp, sign);

    significand_ = static_cast<significand_type>(unsigned_coeff);

    // Normalize the handling of zeros
    if (significand_ == UINT64_C(0))
    {
        exp = 0;
    }

    const auto biased_exp {static_cast<std::uint_fast32_t>(exp + detail::bias_v<decimal64>)};

    if (biased_exp > detail::max_biased_exp_v<decimal64>)
    {
        significand_ = detail::d64_fast_inf;
    }
    else
    {
        exponent_ = static_cast<exponent_type>(biased_exp);
    }
}

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_INTEGRAL Integer>
#else
template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool>>
#endif
BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast::decimal64_fast(Integer val) noexcept
{
    using ConversionType = std::conditional_t<std::is_same<Integer, bool>::value, std::int32_t, Integer>;
    *this = decimal64_fast{static_cast<ConversionType>(val), 0, false};
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
BOOST_DECIMAL_CXX20_CONSTEXPR decimal64_fast::decimal64_fast(Float val) noexcept
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (val != val)
    {
        significand_ = detail::d64_fast_qnan;
    }
    else if (val == std::numeric_limits<Float>::infinity() || val == -std::numeric_limits<Float>::infinity())
    {
        significand_ = detail::d64_fast_inf;
    }
    else
    #endif
    {
        const auto components {detail::ryu::floating_point_to_fd128(val)};
        *this = decimal64_fast {components.mantissa, components.exponent, components.sign};
    }
}

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

BOOST_DECIMAL_GPU_ENABLED constexpr auto direct_init_d64(decimal64_fast::significand_type significand, decimal64_fast::exponent_type exponent, bool sign) noexcept -> decimal64_fast
{
    decimal64_fast val {};
    val.significand_ = significand;
    val.exponent_ = exponent;
    val.sign_ = sign;

    return val;
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto signbit(decimal64_fast val) noexcept -> bool
{
    return val.sign_;
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto isinf(decimal64_fast val) noexcept -> bool
{
    return val.significand_ == detail::d64_fast_inf;
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto isnan(decimal64_fast val) noexcept -> bool
{
    return val.significand_ == detail::d64_fast_qnan ||
           val.significand_ == detail::d64_fast_snan;
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto issignaling(decimal64_fast val) noexcept -> bool
{
    return val.significand_ == detail::d64_fast_snan;
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto isnormal(decimal64_fast val) noexcept -> bool
{
    if (val.exponent_ <= static_cast<decimal64_fast::exponent_type>(detail::precision_v<decimal64> - 1))
    {
        return false;
    }

    return (val.significand_ != 0) && isfinite(val);
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto isfinite(decimal64_fast val) noexcept -> bool
{
    return val.significand_ < detail::d64_fast_snan;
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto not_finite(decimal64_fast val) noexcept -> bool
{
    return val.significand_ >= detail::d64_fast_snan;
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto operator==(decimal64_fast lhs, decimal64_fast rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }
    #endif

    return lhs.sign_ == rhs.sign_ &&
           lhs.exponent_ == rhs.exponent_ &&
           lhs.significand_ == rhs.significand_;
}

template <typename Integer>
BOOST_DECIMAL_GPU_ENABLED constexpr auto operator==(decimal64_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return mixed_equality_impl(lhs, rhs);
}

template <typename Integer>
BOOST_DECIMAL_GPU_ENABLED constexpr auto operator==(Integer lhs, decimal64_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return mixed_equality_impl(rhs, lhs);
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto operator!=(decimal64_fast lhs, decimal64_fast rhs) noexcept -> bool
{
    return !(lhs == rhs);
}

template <typename Integer>
BOOST_DECIMAL_GPU_ENABLED constexpr auto operator!=(decimal64_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return !(lhs == rhs);
}

template <typename Integer>
BOOST_DECIMAL_GPU_ENABLED constexpr auto operator!=(Integer lhs, decimal64_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return !(lhs == rhs);
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto operator<(decimal64_fast lhs, decimal64_fast rhs) noexcept -> bool
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
            return !signbit(rhs);
        }
        else if (isinf(lhs) && isfinite(rhs))
        {
            return signbit(rhs);
        }
    }
    #endif

    // Needed to correctly compare signed and unsigned zeros
    if (lhs.significand_ == 0 || rhs.significand_ == 0)
    {
        if (lhs.significand_ == 0 && rhs.significand_ == 0)
        {
            #ifndef BOOST_DECIMAL_FAST_MATH
            return lhs.sign_ && !rhs.sign_;
            #else
            return false;
            #endif
        }
        return lhs.significand_ == 0 ? !rhs.sign_ : lhs.sign_;
    }

    if (lhs.sign_ != rhs.sign_)
    {
        return lhs.sign_;
    }

    if (lhs.exponent_ != rhs.exponent_)
    {
        return lhs.sign_ ? lhs.exponent_ > rhs.exponent_ : lhs.exponent_ < rhs.exponent_;
    }

    return lhs.sign_ ? lhs.significand_ > rhs.significand_ : lhs.significand_ < rhs.significand_;
}

template <typename Integer>
BOOST_DECIMAL_GPU_ENABLED constexpr auto operator<(decimal64_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return less_impl(lhs, rhs);
}

template <typename Integer>
BOOST_DECIMAL_GPU_ENABLED constexpr auto operator<(Integer lhs, decimal64_fast rhs) noexcept
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

BOOST_DECIMAL_GPU_ENABLED constexpr auto operator<=(decimal64_fast lhs, decimal64_fast rhs) noexcept -> bool
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
BOOST_DECIMAL_GPU_ENABLED constexpr auto operator<=(decimal64_fast lhs, Integer rhs) noexcept
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
BOOST_DECIMAL_GPU_ENABLED constexpr auto operator<=(Integer lhs, decimal64_fast rhs) noexcept
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

BOOST_DECIMAL_GPU_ENABLED constexpr auto operator>(decimal64_fast lhs, decimal64_fast rhs) noexcept -> bool
{
    return rhs < lhs;
}

template <typename Integer>
BOOST_DECIMAL_GPU_ENABLED constexpr auto operator>(decimal64_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return rhs < lhs;
}

template <typename Integer>
BOOST_DECIMAL_GPU_ENABLED constexpr auto operator>(Integer lhs, decimal64_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return rhs < lhs;
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto operator>=(decimal64_fast lhs, decimal64_fast rhs) noexcept -> bool
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
BOOST_DECIMAL_GPU_ENABLED constexpr auto operator>=(decimal64_fast lhs, Integer rhs) noexcept
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
BOOST_DECIMAL_GPU_ENABLED constexpr auto operator>=(Integer lhs, decimal64_fast rhs) noexcept
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

BOOST_DECIMAL_GPU_ENABLED constexpr auto operator<=>(decimal64_fast lhs, decimal64_fast rhs) noexcept -> std::partial_ordering
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
BOOST_DECIMAL_GPU_ENABLED constexpr auto operator<=>(decimal64_fast lhs, Integer rhs) noexcept
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
BOOST_DECIMAL_GPU_ENABLED constexpr auto operator<=>(Integer lhs, decimal64_fast rhs) noexcept
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

#endif // BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR

BOOST_DECIMAL_GPU_ENABLED constexpr auto operator+(decimal64_fast val) noexcept -> decimal64_fast
{
    return val;
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto operator-(decimal64_fast val) noexcept -> decimal64_fast
{
    val.sign_ = !val.sign_;
    return val;
}

BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast::operator bool() const noexcept
{
    constexpr decimal64_fast zero {0, 0};
    return *this != zero;
}

BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast::operator int() const noexcept
{
    return to_integral<decimal64_fast, int>(*this);
}

BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast::operator unsigned() const noexcept
{
    return to_integral<decimal64_fast, unsigned>(*this);
}

BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast::operator long() const noexcept
{
    return to_integral<decimal64_fast, long>(*this);
}

BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast::operator unsigned long() const noexcept
{
    return to_integral<decimal64_fast, unsigned long>(*this);
}

BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast::operator long long() const noexcept
{
    return to_integral<decimal64_fast, long long>(*this);
}

BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast::operator unsigned long long() const noexcept
{
    return to_integral<decimal64_fast, unsigned long long>(*this);
}

BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast::operator std::int8_t() const noexcept
{
    return to_integral<decimal64_fast, std::int8_t>(*this);
}

BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast::operator std::uint8_t() const noexcept
{
    return to_integral<decimal64_fast, std::uint8_t>(*this);
}

BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast::operator std::int16_t() const noexcept
{
    return to_integral<decimal64_fast, std::int16_t>(*this);
}

BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast::operator std::uint16_t() const noexcept
{
    return to_integral<decimal64_fast, std::uint16_t>(*this);
}

#ifdef BOOST_DECIMAL_HAS_INT128

BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast::operator detail::int128_t() const noexcept
{
    return to_integral<decimal64_fast, detail::int128_t>(*this);
}

BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast::operator detail::uint128_t() const noexcept
{
    return to_integral<decimal64_fast, detail::uint128_t>(*this);
}

#endif // BOOST_DECIMAL_HAS_INT128

BOOST_DECIMAL_CXX20_CONSTEXPR decimal64_fast::operator float() const noexcept
{
    return to_float<decimal64_fast, float>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal64_fast::operator double() const noexcept
{
    return to_float<decimal64_fast, double>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal64_fast::operator long double() const noexcept
{
    // TODO(mborland): Don't have an exact way of converting to various long doubles
    return static_cast<long double>(to_float<decimal64_fast, double>(*this));
}

#ifdef BOOST_DECIMAL_HAS_FLOAT16
BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast::operator std::float16_t() const noexcept
{
    return static_cast<std::float16_t>(to_float<decimal64_fast, float>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_FLOAT32
BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast::operator std::float32_t() const noexcept
{
    return static_cast<std::float32_t>(to_float<decimal64_fast, float>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_FLOAT64
BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast::operator std::float64_t() const noexcept
{
    return static_cast<std::float64_t>(to_float<decimal64_fast, double>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_BRAINFLOAT16
BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast::operator std::bfloat16_t() const noexcept
{
    return static_cast<std::bfloat16_t>(to_float<decimal64_fast, float>(*this));
}
#endif

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool>>
BOOST_DECIMAL_GPU_ENABLED constexpr decimal64_fast::operator Decimal() const noexcept
{
    return to_decimal<Decimal>(*this);
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto operator+(decimal64_fast lhs, decimal64_fast rhs) noexcept -> decimal64_fast
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs) || not_finite(rhs))
    {
        return detail::check_non_finite(lhs, rhs);
    }
    #endif

    return detail::d64_add_impl<decimal64_fast>(
            lhs.significand_, lhs.biased_exponent(), lhs.sign_,
            rhs.significand_, rhs.biased_exponent(), rhs.sign_,
            (abs(lhs) > abs(rhs)));
}

template <typename Integer>
BOOST_DECIMAL_GPU_ENABLED constexpr auto operator+(decimal64_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast)
{
    using promoted_significand_type = detail::promote_significand_t<decimal64_fast, Integer>;
    using exp_type = decimal64_fast::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs))
    {
        return lhs;
    }
    #endif

    auto sig_rhs {static_cast<promoted_significand_type>(detail::make_positive_unsigned(rhs))};
    const bool abs_lhs_bigger {abs(lhs) > sig_rhs};

    exp_type exp_rhs {0};
    detail::normalize<decimal64>(sig_rhs, exp_rhs);
    const auto final_sig_rhs {static_cast<decimal64_fast::significand_type>(sig_rhs)};

    return detail::d64_add_impl<decimal64_fast>(lhs.significand_, lhs.biased_exponent(), lhs.sign_,
                                                final_sig_rhs, exp_rhs, (rhs < 0),
                                                abs_lhs_bigger);
}

template <typename Integer>
BOOST_DECIMAL_GPU_ENABLED constexpr auto operator+(Integer lhs, decimal64_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast)
{
    return rhs + lhs;
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto operator-(decimal64_fast lhs, decimal64_fast rhs) noexcept -> decimal64_fast
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs) || not_finite(rhs))
    {
        return detail::check_non_finite(lhs, rhs);
    }
    #endif

    return detail::d64_sub_impl<decimal64_fast>(
            lhs.significand_, lhs.biased_exponent(), lhs.sign_,
            rhs.significand_, rhs.biased_exponent(), rhs.sign_,
            abs(lhs) > abs(rhs)
    );
}

template <typename Integer>
BOOST_DECIMAL_GPU_ENABLED constexpr auto operator-(decimal64_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast)
{
    using promoted_significand_type = detail::promote_significand_t<decimal64_fast, Integer>;
    using exp_type = decimal64_fast::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs))
    {
        return lhs;
    }
    #endif

    auto sig_rhs {static_cast<promoted_significand_type>(detail::make_positive_unsigned(rhs))};
    const bool abs_lhs_bigger {abs(lhs) > detail::make_positive_unsigned(rhs)};

    exp_type exp_rhs {0};
    detail::normalize<decimal64>(sig_rhs, exp_rhs);
    const auto final_sig_rhs {static_cast<decimal64_fast::significand_type>(sig_rhs)};

    return detail::d64_sub_impl<decimal64_fast>(lhs.significand_, lhs.biased_exponent(), lhs.sign_,
                                                final_sig_rhs, exp_rhs, (rhs < 0),
                                                abs_lhs_bigger);
}

template <typename Integer>
BOOST_DECIMAL_GPU_ENABLED constexpr auto operator-(Integer lhs, decimal64_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast)
{
    using promoted_significand_type = detail::promote_significand_t<decimal64_fast, Integer>;
    using exp_type = decimal64_fast::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(rhs))
    {
        return rhs;
    }
    #endif

    auto sig_lhs {static_cast<promoted_significand_type>(detail::make_positive_unsigned(lhs))};
    const bool abs_lhs_bigger {sig_lhs > abs(rhs)};

    exp_type exp_lhs {0};
    detail::normalize<decimal64>(sig_lhs, exp_lhs);
    const auto final_sig_lhs {static_cast<decimal64_fast::significand_type>(sig_lhs)};

    return detail::d64_sub_impl<decimal64_fast>(final_sig_lhs, exp_lhs, (lhs < 0),
                                                rhs.significand_, rhs.biased_exponent(), rhs.sign_,
                                                abs_lhs_bigger);
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto operator*(decimal64_fast lhs, decimal64_fast rhs) noexcept -> decimal64_fast
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs) || not_finite(rhs))
    {
        return detail::check_non_finite(lhs, rhs);
    }
    #endif

    return detail::d64_mul_impl<decimal64_fast>(lhs.significand_, lhs.biased_exponent(), lhs.sign_,
                                                rhs.significand_, rhs.biased_exponent(), rhs.sign_);
}

template <typename Integer>
BOOST_DECIMAL_GPU_ENABLED constexpr auto operator*(decimal64_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast)
{
    using promoted_significand_type = detail::promote_significand_t<decimal64, Integer>;
    using exp_type = decimal64_fast::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs))
    {
        return lhs;
    }
    #endif

    auto rhs_sig {static_cast<promoted_significand_type>(detail::make_positive_unsigned(rhs))};
    exp_type rhs_exp {0};
    detail::normalize<decimal64>(rhs_sig, rhs_exp);
    auto final_rhs_sig {static_cast<decimal64_fast::significand_type>(rhs_sig)};

    return detail::d64_mul_impl<decimal64_fast>(
            lhs.significand_, lhs.biased_exponent(), lhs.sign_,
            final_rhs_sig, rhs_exp, (rhs < 0)
            );
}

template <typename Integer>
BOOST_DECIMAL_GPU_ENABLED constexpr auto operator*(Integer lhs, decimal64_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast)
{
    return rhs * lhs;
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto d64_fast_div_impl(decimal64_fast lhs, decimal64_fast rhs, decimal64_fast& q, decimal64_fast& r) noexcept -> void
{
    const bool sign {lhs.isneg() != rhs.isneg()};

    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check pre-conditions
    constexpr decimal64_fast zero {0, 0};
    constexpr decimal64_fast nan {boost::decimal::direct_init_d64(boost::decimal::detail::d64_fast_snan, 0, false)};
    constexpr decimal64_fast inf {boost::decimal::direct_init_d64(boost::decimal::detail::d64_fast_inf, 0, false)};

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

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "sig lhs: " << sig_lhs
              << "\nexp lhs: " << exp_lhs
              << "\nsig rhs: " << sig_rhs
              << "\nexp rhs: " << exp_rhs << std::endl;
    #endif

    #ifdef BOOST_DECIMAL_HAS_INT128
    using unsigned_int128_type = boost::decimal::detail::uint128_t;
    #else
    using unsigned_int128_type = boost::decimal::detail::uint128;
    #endif

    // If rhs is greater than we need to offset the significands to get the correct values
    // e.g. 4/8 is 0 but 40/8 yields 5 in integer maths
    constexpr auto tens_needed {detail::pow10(static_cast<unsigned_int128_type>(detail::precision_v<decimal64>))};
    const auto big_sig_lhs {static_cast<unsigned_int128_type>(lhs.significand_) * tens_needed};

    const auto res_sig {big_sig_lhs / static_cast<unsigned_int128_type>(rhs.significand_)};
    const auto res_exp {(lhs.biased_exponent() - detail::precision_v<decimal64>) - rhs.biased_exponent()};

    q = decimal64_fast{res_sig, res_exp, sign};
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto d64_fast_mod_impl(decimal64_fast lhs, decimal64_fast rhs, const decimal64_fast& q, decimal64_fast& r) noexcept -> void
{
    constexpr decimal64_fast zero {0, 0};

    // https://en.cppreference.com/w/cpp/numeric/math/fmod
    auto q_trunc {q > zero ? floor(q) : ceil(q)};
    r = lhs - (decimal64_fast(q_trunc) * rhs);
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto operator/(decimal64_fast lhs, decimal64_fast rhs) noexcept -> decimal64_fast
{
    decimal64_fast q {};
    decimal64_fast r {};

    d64_fast_div_impl(lhs, rhs, q, r);

    return q;
}

template <typename Integer>
BOOST_DECIMAL_GPU_ENABLED constexpr auto operator/(decimal64_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast)
{
    using promoted_significand_type = detail::promote_significand_t<decimal64_fast, Integer>;
    using exp_type = detail::decimal64_fast_components::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check pre-conditions
    constexpr decimal64_fast zero {0, 0};
    constexpr decimal64_fast nan {boost::decimal::direct_init_d64(boost::decimal::detail::d64_fast_snan, 0, false)};
    constexpr decimal64_fast inf {boost::decimal::direct_init_d64(boost::decimal::detail::d64_fast_inf, 0, false)};

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

    detail::decimal64_fast_components lhs_components {lhs_sig, lhs_exp, lhs.isneg()};

    auto rhs_sig {static_cast<promoted_significand_type>(detail::make_positive_unsigned(rhs))};
    exp_type rhs_exp {};
    detail::decimal64_fast_components rhs_components {detail::shrink_significand<decimal64_fast::significand_type>(rhs_sig, rhs_exp), rhs_exp, rhs < 0};

    return detail::d64_generic_div_impl<decimal64_fast>(lhs_components, rhs_components);
}

template <typename Integer>
BOOST_DECIMAL_GPU_ENABLED constexpr auto operator/(Integer lhs, decimal64_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast)
{
    using promoted_significand_type = detail::promote_significand_t<decimal64_fast, Integer>;
    using exp_type = detail::decimal64_fast_components::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check pre-conditions
    constexpr decimal64_fast zero {0, 0};
    constexpr decimal64_fast nan {boost::decimal::direct_init_d64(boost::decimal::detail::d64_fast_snan, 0, false)};
    constexpr decimal64_fast inf {boost::decimal::direct_init_d64(boost::decimal::detail::d64_fast_inf, 0, false)};

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
    detail::decimal64_fast_components rhs_components {rhs_sig, rhs_exp, rhs.isneg()};

    auto lhs_sig {static_cast<promoted_significand_type>(detail::make_positive_unsigned(lhs))};
    exp_type lhs_exp {};
    detail::decimal64_fast_components lhs_components {detail::shrink_significand<decimal64_fast::significand_type>(lhs_sig, lhs_exp), lhs_exp, lhs < 0};

    return detail::d64_generic_div_impl<decimal64_fast>(lhs_components, rhs_components);
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto operator%(decimal64_fast lhs, decimal64_fast rhs) noexcept -> decimal64_fast
{
    decimal64_fast q {};
    decimal64_fast r {};
    d64_fast_div_impl(lhs, rhs, q, r);
    d64_fast_mod_impl(lhs, rhs, q, r);

    return r;
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto decimal64_fast::operator+=(decimal64_fast rhs) noexcept -> decimal64_fast &
{
    *this = *this + rhs;
    return *this;
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto decimal64_fast::operator-=(decimal64_fast rhs) noexcept -> decimal64_fast &
{
    *this = *this - rhs;
    return *this;
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto decimal64_fast::operator*=(decimal64_fast rhs) noexcept -> decimal64_fast &
{
    *this = *this * rhs;
    return *this;
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto decimal64_fast::operator/=(decimal64_fast rhs) noexcept -> decimal64_fast &
{
    *this = *this / rhs;
    return *this;
}

template <typename Integer>
BOOST_DECIMAL_GPU_ENABLED constexpr auto decimal64_fast::operator+=(Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast&)
{
    *this = *this + rhs;
    return *this;
}

template <typename Integer>
BOOST_DECIMAL_GPU_ENABLED constexpr auto decimal64_fast::operator-=(Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast&)
{
    *this = *this - rhs;
    return *this;
}

template <typename Integer>
BOOST_DECIMAL_GPU_ENABLED constexpr auto decimal64_fast::operator*=(Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast&)
{
    *this = *this * rhs;
    return *this;
}

template <typename Integer>
BOOST_DECIMAL_GPU_ENABLED constexpr auto decimal64_fast::operator/=(Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal64_fast&)
{
    *this = *this / rhs;
    return *this;
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto decimal64_fast::operator++() noexcept -> decimal64_fast&
{
    constexpr decimal64_fast one {1, 0};
    *this = *this + one;
    return *this;
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto decimal64_fast::operator++(int) noexcept -> decimal64_fast&
{
    return ++(*this);
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto decimal64_fast::operator--() noexcept -> decimal64_fast&
{
    constexpr decimal64_fast one {1, 0};
    *this = *this - one;
    return *this;
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto decimal64_fast::operator--(int) noexcept -> decimal64_fast&
{
    return --(*this);
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto scalblnd64f(decimal64_fast num, long exp) noexcept -> decimal64_fast
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    constexpr decimal64_fast zero {0, 0};

    if (num == zero || exp == 0 || not_finite(num))
    {
        return num;
    }
    #endif

    num = decimal64_fast(num.significand_, num.biased_exponent() + exp, num.sign_);

    return num;
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto scalbnd64f(decimal64_fast num, int expval) noexcept -> decimal64_fast
{
    return scalblnd64f(num, static_cast<long>(expval));
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto copysignd64f(decimal64_fast mag, decimal64_fast sgn) noexcept -> decimal64_fast
{
    mag.sign_ = sgn.sign_;
    return mag;
}

} // namespace decimal
} // namespace boost

namespace std {

template <>
#ifdef _MSC_VER
class numeric_limits<boost::decimal::decimal64_fast>
#else
struct numeric_limits<boost::decimal::decimal64_fast>
#endif
{
#ifdef _MSC_VER
    public:
#endif

    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr bool is_specialized = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr bool is_signed = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr bool is_integer = false;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr bool is_exact = false;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr bool has_infinity = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr bool has_quiet_NaN = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr bool has_signaling_NaN = true;

    // These members were deprecated in C++23
    #if ((!defined(_MSC_VER) && (__cplusplus <= 202002L)) || (defined(_MSC_VER) && (_MSVC_LANG <= 202002L)))
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr std::float_denorm_style has_denorm = std::denorm_present;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr bool has_denorm_loss = true;
    #endif

    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr std::float_round_style round_style = std::round_indeterminate;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr bool is_iec559 = false;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr bool is_bounded = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr bool is_modulo = false;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr int  digits = 16;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr int  digits10 = digits;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr int  max_digits10 = digits;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr int  radix = 10;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr int  min_exponent = -382;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr int  min_exponent10 = min_exponent;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr int  max_exponent = 385;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr int  max_exponent10 = max_exponent;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr bool traps = numeric_limits<std::uint_fast64_t>::traps;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC constexpr bool tinyness_before = true;

    // Member functions
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC BOOST_DECIMAL_GPU_ENABLED constexpr auto (min)        () -> boost::decimal::decimal64_fast { return {1, min_exponent}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC BOOST_DECIMAL_GPU_ENABLED constexpr auto (max)        () -> boost::decimal::decimal64_fast { return {9'999'999'999'999'999, max_exponent}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC BOOST_DECIMAL_GPU_ENABLED constexpr auto lowest       () -> boost::decimal::decimal64_fast { return {-9'999'999'999'999'999, max_exponent}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC BOOST_DECIMAL_GPU_ENABLED constexpr auto epsilon      () -> boost::decimal::decimal64_fast { return {1, -16}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC BOOST_DECIMAL_GPU_ENABLED constexpr auto round_error  () -> boost::decimal::decimal64_fast { return epsilon(); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC BOOST_DECIMAL_GPU_ENABLED constexpr auto infinity     () -> boost::decimal::decimal64_fast { return boost::decimal::direct_init_d64(
                boost::decimal::detail::d64_fast_inf, 0, false); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC BOOST_DECIMAL_GPU_ENABLED constexpr auto quiet_NaN    () -> boost::decimal::decimal64_fast { return boost::decimal::direct_init_d64(
                boost::decimal::detail::d64_fast_qnan, 0, false); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC BOOST_DECIMAL_GPU_ENABLED constexpr auto signaling_NaN() -> boost::decimal::decimal64_fast { return boost::decimal::direct_init_d64(
                boost::decimal::detail::d64_fast_snan, 0, false); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_STATIC BOOST_DECIMAL_GPU_ENABLED constexpr auto denorm_min   () -> boost::decimal::decimal64_fast { return {1, boost::decimal::detail::etiny_v<boost::decimal::decimal64>}; }
};

} // namespace std

#endif //BOOST_DECIMAL_DECIMAL64_FAST_HPP
