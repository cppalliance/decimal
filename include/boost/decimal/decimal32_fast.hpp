// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DECIMAL32_FAST_HPP
#define BOOST_DECIMAL_DECIMAL32_FAST_HPP

#include <boost/decimal/decimal32.hpp>
#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/add_impl.hpp>
#include <boost/decimal/detail/mul_impl.hpp>
#include <boost/decimal/detail/div_impl.hpp>
#include <boost/decimal/detail/promote_significand.hpp>
#include <boost/decimal/detail/ryu/ryu_generic_128.hpp>
#include <boost/decimal/detail/promotion.hpp>
#include <boost/decimal/detail/comparison.hpp>
#include <boost/decimal/detail/cmath/next.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <limits>
#include <cstdint>
#endif

namespace boost {
namespace decimal {

namespace detail {

BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d32_fast_inf = std::numeric_limits<std::uint_fast32_t>::max() - 3;
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d32_fast_qnan = std::numeric_limits<std::uint_fast32_t>::max() - 2;
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d32_fast_snan = std::numeric_limits<std::uint_fast32_t>::max() - 1;

}

BOOST_DECIMAL_EXPORT class decimal32_fast final
{
public:
    using significand_type = std::uint_fast32_t;
    using exponent_type = std::uint_fast8_t;
    using biased_exponent_type = std::int_fast32_t;

private:
    // In regular decimal32 we have to decode the 24 bits of the significand and the 8 bits of the exp
    // Here we just use them directly at the cost of at least 2 extra bytes of internal state
    // since the fast integer types will be at least 32 and 8 bits respectively

    significand_type significand_ {};
    exponent_type exponent_ {};
    bool sign_ {};

    constexpr auto isneg() const noexcept -> bool
    {
        return sign_;
    }

    constexpr auto full_significand() const noexcept -> significand_type
    {
        return significand_;
    }

    constexpr auto unbiased_exponent() const noexcept -> exponent_type
    {
        return exponent_;
    }

    constexpr auto biased_exponent() const noexcept -> biased_exponent_type
    {
        return static_cast<biased_exponent_type>(exponent_) - detail::bias_v<decimal32>;
    }

    friend constexpr auto div_impl(decimal32_fast lhs, decimal32_fast rhs, decimal32_fast& q, decimal32_fast& r) noexcept -> void;

    friend constexpr auto mod_impl(decimal32_fast lhs, decimal32_fast rhs, const decimal32_fast& q, decimal32_fast& r) noexcept -> void;

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

    template <typename DecimalType>
    friend constexpr auto to_dpd_d32(DecimalType val) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, DecimalType, std::uint32_t);

    template <typename ReturnType, typename T>
    friend constexpr auto detail::d32_add_impl(const T& lhs, const T& rhs) noexcept -> ReturnType;

    template <BOOST_DECIMAL_FAST_DECIMAL_FLOATING_TYPE DecimalType>
    BOOST_DECIMAL_FORCE_INLINE friend constexpr auto fast_equality_impl(const DecimalType& lhs, const DecimalType& rhs) noexcept -> bool;

    template <BOOST_DECIMAL_FAST_DECIMAL_FLOATING_TYPE DecimalType>
    BOOST_DECIMAL_FORCE_INLINE friend constexpr auto fast_inequality_impl(const DecimalType& lhs, const DecimalType& rhs) noexcept -> bool;

    template <BOOST_DECIMAL_FAST_DECIMAL_FLOATING_TYPE DecimalType>
    BOOST_DECIMAL_FORCE_INLINE friend constexpr auto fast_less_impl(const DecimalType& lhs, const DecimalType& rhs) noexcept -> bool;

    friend constexpr auto not_finite(const decimal32_fast& val) noexcept -> bool;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
    friend constexpr auto detail::nextafter_impl(DecimalType val, bool direction) noexcept -> DecimalType;

public:
    constexpr decimal32_fast() noexcept = default;

    template <typename T1, typename T2, std::enable_if_t<detail::is_unsigned_v<T1> && detail::is_integral_v<T2>, bool> = true>
    constexpr decimal32_fast(T1 coeff, T2 exp, bool sign = false) noexcept;

    template <typename T1, typename T2, std::enable_if_t<!detail::is_unsigned_v<T1> && detail::is_integral_v<T2>, bool> = true>
    constexpr decimal32_fast(T1 coeff, T2 exp) noexcept;

    explicit constexpr decimal32_fast(bool value) noexcept;

    template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool> = true>
    constexpr decimal32_fast(Integer coeff) noexcept;

    template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool> = true>
    explicit BOOST_DECIMAL_CXX20_CONSTEXPR decimal32_fast(Float val) noexcept;

    constexpr decimal32_fast(const decimal32_fast& val) noexcept = default;
    constexpr decimal32_fast(decimal32_fast&& val) noexcept = default;
    constexpr auto operator=(const decimal32_fast& val) noexcept -> decimal32_fast& = default;
    constexpr auto operator=(decimal32_fast&& val) noexcept -> decimal32_fast& = default;

    // cmath functions that are easier as friends
    friend constexpr auto signbit(decimal32_fast val) noexcept -> bool;
    friend constexpr auto isinf(decimal32_fast val) noexcept -> bool;
    friend constexpr auto isnan(decimal32_fast val) noexcept -> bool;
    friend constexpr auto issignaling(decimal32_fast val) noexcept -> bool;
    friend constexpr auto isnormal(decimal32_fast val) noexcept -> bool;
    friend constexpr auto isfinite(decimal32_fast val) noexcept -> bool;

    // Comparison operators
    friend constexpr auto operator==(decimal32_fast lhs, decimal32_fast rhs) noexcept -> bool;
    friend constexpr auto operator!=(decimal32_fast lhs, decimal32_fast rhs) noexcept -> bool;
    friend constexpr auto operator<(decimal32_fast lhs, decimal32_fast rhs) noexcept -> bool;
    friend constexpr auto operator<=(decimal32_fast lhs, decimal32_fast rhs) noexcept -> bool;
    friend constexpr auto operator>(decimal32_fast lhs, decimal32_fast rhs) noexcept -> bool;
    friend constexpr auto operator>=(decimal32_fast lhs, decimal32_fast rhs) noexcept -> bool;

    // Mixed comparisons
    template <typename Integer>
    friend constexpr auto operator==(decimal32_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator==(Integer lhs, decimal32_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator!=(decimal32_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator!=(Integer lhs, decimal32_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator<(decimal32_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator<(Integer lhs, decimal32_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator<=(decimal32_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator<=(Integer lhs, decimal32_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator>(decimal32_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator>(Integer lhs, decimal32_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator>=(decimal32_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator>=(Integer lhs, decimal32_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    #ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR

    friend constexpr auto operator<=>(decimal32_fast lhs, decimal32_fast rhs) noexcept -> std::partial_ordering;

    template <typename Integer>
    friend constexpr auto operator<=>(decimal32_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering);

    template <typename Integer>
    friend constexpr auto operator<=>(Integer lhs, decimal32_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering);

    #endif

    // Unary operators
    friend constexpr auto operator+(decimal32_fast rhs) noexcept -> decimal32_fast;
    friend constexpr auto operator-(decimal32_fast lhs) noexcept -> decimal32_fast;

    // Binary arithmetic
    friend constexpr auto operator+(decimal32_fast lhs, decimal32_fast rhs) noexcept -> decimal32_fast;
    friend constexpr auto operator-(decimal32_fast lhs, decimal32_fast rhs) noexcept -> decimal32_fast;
    friend constexpr auto operator*(decimal32_fast lhs, decimal32_fast rhs) noexcept -> decimal32_fast;
    friend constexpr auto operator/(decimal32_fast lhs, decimal32_fast rhs) noexcept -> decimal32_fast;
    friend constexpr auto operator%(decimal32_fast lhs, decimal32_fast rhs) noexcept -> decimal32_fast;

    // Mixed type binary arithmetic
    template <typename Integer>
    friend constexpr auto operator+(decimal32_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast);

    template <typename Integer>
    friend constexpr auto operator+(Integer lhs, decimal32_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast);

    template <typename Integer>
    friend constexpr auto operator-(decimal32_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast);

    template <typename Integer>
    friend constexpr auto operator-(Integer lhs, decimal32_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast);

    template <typename Integer>
    friend constexpr auto operator*(decimal32_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast);

    template <typename Integer>
    friend constexpr auto operator*(Integer lhs, decimal32_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast);

    template <typename Integer>
    friend constexpr auto operator/(decimal32_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast);

    template <typename Integer>
    friend constexpr auto operator/(Integer lhs, decimal32_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast);

    // Compound operators
    constexpr auto operator+=(decimal32_fast rhs) noexcept -> decimal32_fast&;
    constexpr auto operator-=(decimal32_fast rhs) noexcept -> decimal32_fast&;
    constexpr auto operator*=(decimal32_fast rhs) noexcept -> decimal32_fast&;
    constexpr auto operator/=(decimal32_fast rhs) noexcept -> decimal32_fast&;
    constexpr auto operator%=(decimal32_fast rhs) noexcept -> decimal32_fast&;

    // Mixed type compound operators
    template <typename Integer>
    constexpr auto operator+=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast&);

    template <typename Integer>
    constexpr auto operator-=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast&);

    template <typename Integer>
    constexpr auto operator*=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast&);

    template <typename Integer>
    constexpr auto operator/=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast&);

    // Increment and decrement
    constexpr auto operator++() noexcept -> decimal32_fast&;
    constexpr auto operator++(int) noexcept -> decimal32_fast&;
    constexpr auto operator--() noexcept -> decimal32_fast&;
    constexpr auto operator--(int) noexcept -> decimal32_fast&;

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


    // Conversion to other decimal type
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal> && (detail::decimal_val_v<Decimal> > detail::decimal_val_v<decimal32_fast>), bool> = true>
    constexpr operator Decimal() const noexcept;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal> && (detail::decimal_val_v<Decimal> <= detail::decimal_val_v<decimal32_fast>), bool> = true>
    explicit constexpr operator Decimal() const noexcept;

    friend constexpr auto direct_init(std::uint_fast32_t significand, std::uint_fast8_t exponent, bool sign) noexcept -> decimal32_fast;

    // <cmath> or extensions that need to be friends
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
    friend constexpr auto frexp10(T num, int* expptr) noexcept -> typename T::significand_type;

    friend constexpr auto copysignd32f(decimal32_fast mag, decimal32_fast sgn) noexcept -> decimal32_fast;
    friend constexpr auto scalbnd32f(decimal32_fast num, int exp) noexcept -> decimal32_fast;
    friend constexpr auto scalblnd32f(decimal32_fast num, long exp) noexcept -> decimal32_fast;
    friend constexpr auto fmad32f(decimal32_fast x, decimal32_fast y, decimal32_fast z) noexcept -> decimal32_fast;

    template <typename T>
    friend constexpr auto ilogb(T d) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, T, int);

    template <typename T>
    friend constexpr auto logb(T num) noexcept
        BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T);

    // Specific decimal functionality
    friend constexpr auto samequantumd32f(decimal32_fast lhs, decimal32_fast rhs) noexcept -> bool;
    friend constexpr auto quantexpd32f(decimal32_fast x) noexcept -> int;
    friend constexpr auto quantized32f(decimal32_fast lhs, decimal32_fast rhs) noexcept -> decimal32_fast;
};

template <typename T1, typename T2, std::enable_if_t<detail::is_unsigned_v<T1> && detail::is_integral_v<T2>, bool>>
constexpr decimal32_fast::decimal32_fast(T1 coeff, T2 exp, bool sign) noexcept
{
    using minimum_coefficient_size = std::conditional_t<(sizeof(T1) > sizeof(significand_type)), T1, significand_type>;

    minimum_coefficient_size min_coeff {coeff};

    sign_ = sign;

    // Normalize in the constructor, so we never have to worry about it again
    detail::normalize<decimal32>(min_coeff, exp, sign);

    significand_ = static_cast<significand_type>(min_coeff);

    const auto biased_exp {significand_ == 0U ? 0 : exp + detail::bias};

    // Decimal32 exponent holds 8 bits
    if (biased_exp > detail::max_biased_exp_v<decimal32_fast>)
    {
        significand_ = detail::d32_fast_inf;
    }
    else if (biased_exp >= 0)
    {
        exponent_ = static_cast<exponent_type>(biased_exp);
    }
    else
    {
        // Flush denorms to zero
        significand_ = static_cast<significand_type>(0);
        exponent_ = static_cast<exponent_type>(detail::bias);
        sign_ = false;
    }
}

template <typename T1, typename T2, std::enable_if_t<!detail::is_unsigned_v<T1> && detail::is_integral_v<T2>, bool>>
constexpr decimal32_fast::decimal32_fast(T1 coeff, T2 exp) noexcept : decimal32_fast(detail::make_positive_unsigned(coeff), exp, coeff < 0) {}

constexpr decimal32_fast::decimal32_fast(bool value) noexcept : decimal32_fast(static_cast<significand_type>(value), 0, false) {}

template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool>>
constexpr decimal32_fast::decimal32_fast(Integer val) noexcept : decimal32_fast{val, 0} {}

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wfloat-equal"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool>>
BOOST_DECIMAL_CXX20_CONSTEXPR decimal32_fast::decimal32_fast(Float val) noexcept
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (val != val)
    {
        significand_ = detail::d32_fast_qnan;
    }
    else if (val == std::numeric_limits<Float>::infinity() || val == -std::numeric_limits<Float>::infinity())
    {
        significand_ = detail::d32_fast_inf;
    }
    else
    #endif
    {
        const auto components {detail::ryu::floating_point_to_fd128(val)};
        *this = decimal32_fast {components.mantissa, components.exponent, components.sign};
    }
}

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

constexpr auto direct_init(std::uint_fast32_t significand, std::uint_fast8_t exponent, bool sign = false) noexcept -> decimal32_fast
{
    decimal32_fast val;
    val.significand_ = significand;
    val.exponent_ = exponent;
    val.sign_ = sign;

    return val;
}

constexpr auto signbit(decimal32_fast val) noexcept -> bool
{
    return val.sign_;
}

constexpr auto isinf(decimal32_fast val) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return val.significand_ == detail::d32_fast_inf;
    #else
    static_cast<void>(val);
    return false;
    #endif
}

constexpr auto isnan(decimal32_fast val) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return val.significand_ >= detail::d32_fast_qnan;
    #else
    static_cast<void>(val);
    return false;
    #endif
}

constexpr auto issignaling(decimal32_fast val) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return val.significand_ == detail::d32_fast_snan;
    #else
    static_cast<void>(val);
    return false;
    #endif
}

constexpr auto isnormal(decimal32_fast val) noexcept -> bool
{
    return (val.significand_ != 0)
    #ifndef BOOST_DECIMAL_FAST_MATH
    && isfinite(val) && (val.exponent_ > static_cast<std::uint8_t>(detail::precision_v<decimal32> - 1))
    #endif
    ;
}

constexpr auto isfinite(decimal32_fast val) noexcept -> bool
{
    return val.significand_ < detail::d32_fast_inf;
}

BOOST_DECIMAL_FORCE_INLINE constexpr auto not_finite(const decimal32_fast& val) noexcept -> bool
{
    return val.significand_ >= detail::d32_fast_inf;
}

constexpr auto operator==(decimal32_fast lhs, decimal32_fast rhs) noexcept -> bool
{
    return fast_equality_impl(lhs, rhs);
}

constexpr auto operator!=(decimal32_fast lhs, decimal32_fast rhs) noexcept -> bool
{
    return fast_inequality_impl(lhs, rhs);
}

constexpr auto operator<(decimal32_fast lhs, decimal32_fast rhs) noexcept -> bool
{
    return fast_less_impl(lhs, rhs);
}

constexpr auto operator<=(decimal32_fast lhs, decimal32_fast rhs) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }
    #endif

    return !(rhs < lhs);
}

constexpr auto operator>(decimal32_fast lhs, decimal32_fast rhs) noexcept -> bool
{
    return rhs < lhs;
}

constexpr auto operator>=(decimal32_fast lhs, decimal32_fast rhs) noexcept -> bool
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
constexpr auto operator==(decimal32_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return mixed_equality_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator==(Integer lhs, decimal32_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return mixed_equality_impl(rhs, lhs);
}

template <typename Integer>
constexpr auto operator!=(decimal32_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator!=(Integer lhs, decimal32_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator<(decimal32_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return less_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator<(Integer lhs, decimal32_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return isnan(rhs) ? false : !less_impl(rhs, lhs) && lhs != rhs;
    #else
    return !less_impl(rhs, lhs) && lhs != rhs;
    #endif
}

template <typename Integer>
constexpr auto operator<=(decimal32_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return isnan(lhs) ? false : !(rhs < lhs);
    #else
    return !(rhs < lhs);
    #endif
}

template <typename Integer>
constexpr auto operator<=(Integer lhs, decimal32_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return isnan(rhs) ? false : !(rhs < lhs);
    #else
    return !(rhs < lhs);
    #endif
}

template <typename Integer>
constexpr auto operator>(decimal32_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return isnan(lhs) ? false : rhs < lhs;
    #else
    return rhs < lhs;
    #endif
}

template <typename Integer>
constexpr auto operator>(Integer lhs, decimal32_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return isnan(rhs) ? false : rhs < lhs;
    #else
    return rhs < lhs;
    #endif
}

template <typename Integer>
constexpr auto operator>=(decimal32_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return isnan(lhs) ? false : !(lhs < rhs);
    #else
    return !(lhs < rhs);
    #endif
}

template <typename Integer>
constexpr auto operator>=(Integer lhs, decimal32_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return isnan(rhs) ? false : !(lhs < rhs);
    #else
    return !(lhs < rhs);
    #endif
}

#ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR

constexpr auto operator<=>(decimal32_fast lhs, decimal32_fast rhs) noexcept -> std::partial_ordering
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
constexpr auto operator<=>(decimal32_fast lhs, Integer rhs) noexcept
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
constexpr auto operator<=>(Integer lhs, decimal32_fast rhs) noexcept
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

constexpr auto operator+(decimal32_fast rhs) noexcept -> decimal32_fast
{
    return rhs;
}

constexpr auto operator-(decimal32_fast rhs) noexcept -> decimal32_fast
{
    rhs.sign_ = !rhs.sign_;
    return rhs;
}

constexpr auto operator+(decimal32_fast lhs, decimal32_fast rhs) noexcept -> decimal32_fast
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (!isfinite(lhs) || !isfinite(rhs))
    {
        return detail::check_non_finite(lhs, rhs);
    }
    #endif

    return detail::d32_add_impl<decimal32_fast>(lhs, rhs);
}

template <typename Integer>
constexpr auto operator+(decimal32_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast)
{
    using promoted_significand_type = detail::promote_significand_t<decimal32_fast, Integer>;
    using exp_type = decimal32_fast::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (!isfinite(lhs))
    {
        return lhs;
    }
    #endif

    auto sig_rhs {static_cast<promoted_significand_type>(detail::make_positive_unsigned(rhs))};

    exp_type exp_rhs {0};
    detail::normalize(sig_rhs, exp_rhs);
    const auto final_sig_rhs {static_cast<detail::decimal32_fast_components::significand_type>(detail::make_positive_unsigned(sig_rhs))};

    return detail::d32_add_impl<decimal32_fast>(lhs.significand_, lhs.biased_exponent(), lhs.sign_,
                                                final_sig_rhs, exp_rhs, (rhs < 0));
}

template <typename Integer>
constexpr auto operator+(Integer lhs, decimal32_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast)
{
    return rhs + lhs;
}

constexpr auto operator-(decimal32_fast lhs, decimal32_fast rhs) noexcept -> decimal32_fast
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (!isfinite(lhs) || !isfinite(rhs))
    {
        return detail::check_non_finite(lhs, rhs);
    }
    #endif

    rhs.sign_ = !rhs.sign_;

    return detail::d32_add_impl<decimal32_fast>(lhs, rhs);
}

template <typename Integer>
constexpr auto operator-(decimal32_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast)
{
    using promoted_significand_type = detail::promote_significand_t<decimal32_fast, Integer>;
    using exp_type = decimal32_fast::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (!isfinite(lhs))
    {
        return lhs;
    }
    #endif

    auto sig_rhs {static_cast<promoted_significand_type>(detail::make_positive_unsigned(rhs))};

    exp_type exp_rhs {0};
    detail::normalize(sig_rhs, exp_rhs);
    auto final_sig_rhs {static_cast<decimal32_fast::significand_type>(detail::make_positive_unsigned(sig_rhs))};

    return detail::d32_add_impl<decimal32_fast>(
            lhs.significand_, lhs.biased_exponent(), lhs.sign_,
            final_sig_rhs, exp_rhs, !(rhs < 0));
}

template <typename Integer>
constexpr auto operator-(Integer lhs, decimal32_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast)
{
    using promoted_significand_type = detail::promote_significand_t<decimal32_fast, Integer>;
    using exp_type = decimal32_fast::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (!isfinite(rhs))
    {
        return rhs;
    }
    #endif

    auto sig_lhs {static_cast<promoted_significand_type>(detail::make_positive_unsigned(lhs))};

    exp_type exp_lhs {0};
    detail::normalize(sig_lhs, exp_lhs);
    auto final_sig_lhs {static_cast<decimal32_fast::significand_type>(detail::make_positive_unsigned(sig_lhs))};

    return detail::d32_add_impl<decimal32_fast>(
            final_sig_lhs, exp_lhs, (lhs < 0),
            rhs.significand_, rhs.biased_exponent(), !rhs.sign_
    );
}

constexpr auto operator*(decimal32_fast lhs, decimal32_fast rhs) noexcept -> decimal32_fast
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (!isfinite(lhs) || !isfinite(rhs))
    {
        return detail::check_non_finite(lhs, rhs);
    }
    #endif

    using mul_type = std::uint_fast64_t;

    const auto isneg {lhs.sign_ != rhs.sign_};
    constexpr auto ten_pow_seven {detail::pow10(static_cast<mul_type>(6))};
    constexpr auto ten_pow_seven_exp_offset {95U};
    constexpr auto ten_pow_six {detail::pow10(static_cast<mul_type>(5))};
    constexpr auto ten_pow_six_exp_offset {96U};

    auto res_sig {(static_cast<mul_type>(lhs.significand_) * static_cast<mul_type>(rhs.significand_))};
    const bool res_sig_14_dig {res_sig > UINT64_C(10000000000000)};
    res_sig /= res_sig_14_dig ? ten_pow_seven : ten_pow_six;
    auto res_exp {lhs.exponent_ + rhs.exponent_};
    res_exp -= res_sig_14_dig ? ten_pow_seven_exp_offset : ten_pow_six_exp_offset;

    res_exp += detail::fenv_round(res_sig, isneg);

    BOOST_DECIMAL_ASSERT(res_sig >= 1'000'000 || res_sig == 0U);
    BOOST_DECIMAL_ASSERT(res_exp <= 9'999'999 || res_sig == 0U);

    return direct_init(static_cast<decimal32_fast::significand_type>(res_sig), static_cast<decimal32_fast::exponent_type>(res_exp) , isneg);
}

template <typename Integer>
constexpr auto operator*(decimal32_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast)
{
    using promoted_significand_type = detail::promote_significand_t<decimal32_fast, Integer>;
    using exp_type = decimal32_fast::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (!isfinite(lhs))
    {
        return lhs;
    }
    #endif

    auto sig_rhs {static_cast<promoted_significand_type>(detail::make_positive_unsigned(rhs))};
    exp_type exp_rhs {0};
    detail::normalize(sig_rhs, exp_rhs);

    // We don't know if the original value of rhs fits into the decimal32_fast significand type
    // but once it's normalized it's guaranteed to fit
    const auto final_sig_rhs {static_cast<decimal32_fast::significand_type>(sig_rhs)};

    return detail::mul_impl<decimal32_fast>(lhs.significand_, lhs.biased_exponent(), lhs.sign_,
                                             final_sig_rhs, exp_rhs, (rhs < 0));
}

template <typename Integer>
constexpr auto operator*(Integer lhs, decimal32_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast)
{
    return rhs * lhs;
}

constexpr auto div_impl(decimal32_fast lhs, decimal32_fast rhs, decimal32_fast& q, decimal32_fast& r) noexcept -> void
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    const bool sign {lhs.isneg() != rhs.isneg()};

    constexpr decimal32_fast zero {0, 0};
    constexpr decimal32_fast nan {direct_init(detail::d32_fast_qnan, UINT8_C(0), false)};
    constexpr decimal32_fast inf {direct_init(detail::d32_fast_inf, UINT8_C(0), false)};

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

    // We promote to uint64 since the significands are currently 32-bits
    // By appending enough zeros to the LHS we end up finding what we need anyway
    constexpr auto ten_pow_precision {detail::pow10(static_cast<std::uint_fast64_t>(detail::precision_v<decimal32>))};
    const auto big_sig_lhs {static_cast<std::uint_fast64_t>(lhs.significand_) * ten_pow_precision};
    const auto res_sig {big_sig_lhs / static_cast<std::uint_fast64_t>(rhs.significand_)};
    const auto res_exp {(lhs.biased_exponent() - detail::precision_v<decimal32>) - rhs.biased_exponent()};

    q = decimal32_fast(res_sig, res_exp, lhs.sign_ != rhs.sign_);
}

constexpr auto mod_impl(decimal32_fast lhs, decimal32_fast rhs, const decimal32_fast& q, decimal32_fast& r) noexcept -> void
{
    constexpr decimal32_fast zero {0, 0};

    // https://en.cppreference.com/w/cpp/numeric/math/fmod
    auto q_trunc {q > zero ? floor(q) : ceil(q)};
    r = lhs - (decimal32_fast(q_trunc) * rhs);
}

constexpr auto operator/(decimal32_fast lhs, decimal32_fast rhs) noexcept -> decimal32_fast
{
    decimal32_fast q {};
    decimal32_fast r {};
    div_impl(lhs, rhs, q, r);

    return q;
}

template <typename Integer>
constexpr auto operator/(decimal32_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast)
{
    using exp_type = decimal32_fast::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check pre-conditions
    constexpr decimal32_fast zero {0, 0};
    constexpr decimal32_fast nan {direct_init(detail::d32_fast_qnan, UINT8_C(0), false)};
    constexpr decimal32_fast inf {direct_init(detail::d32_fast_inf, UINT8_C(0), false)};

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

    const detail::decimal32_fast_components lhs_components {lhs.significand_, lhs.biased_exponent(), lhs.sign_};
    exp_type exp_rhs {};
    const detail::decimal32_fast_components rhs_components {detail::shrink_significand<decimal32_fast::significand_type>(detail::make_positive_unsigned(rhs), exp_rhs), exp_rhs, rhs < 0};

    return detail::generic_div_impl<decimal32_fast>(lhs_components, rhs_components);
}

template <typename Integer>
constexpr auto operator/(Integer lhs, decimal32_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast)
{
    using exp_type = decimal32_fast::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check pre-conditions
    constexpr decimal32_fast zero {0, 0};
    constexpr decimal32_fast nan {direct_init(detail::d32_fast_qnan, UINT8_C(0), false)};
    constexpr decimal32_fast inf {direct_init(detail::d32_fast_inf, UINT8_C(0), false)};

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

    exp_type lhs_exp {};
    const auto lhs_sig {detail::make_positive_unsigned(detail::shrink_significand<decimal32_fast::significand_type>(lhs, lhs_exp))};
    const detail::decimal32_fast_components lhs_components {lhs_sig, lhs_exp, lhs < 0};
    const detail::decimal32_fast_components rhs_components {rhs.significand_, rhs.biased_exponent(), rhs.isneg()};

    return detail::generic_div_impl<decimal32_fast>(lhs_components, rhs_components);
}

constexpr auto operator%(decimal32_fast lhs, decimal32_fast rhs) noexcept -> decimal32_fast
{
    decimal32_fast q {};
    decimal32_fast r {};
    div_impl(lhs, rhs, q, r);
    mod_impl(lhs, rhs, q, r);

    return r;
}

constexpr auto decimal32_fast::operator%=(decimal32_fast rhs) noexcept -> decimal32_fast&
{
    *this = *this % rhs;
    return *this;
}

constexpr auto decimal32_fast::operator+=(decimal32_fast rhs) noexcept -> decimal32_fast&
{
    *this = *this + rhs;
    return *this;
}

constexpr auto decimal32_fast::operator-=(decimal32_fast rhs) noexcept -> decimal32_fast&
{
    *this = *this - rhs;
    return *this;
}

constexpr auto decimal32_fast::operator*=(decimal32_fast rhs) noexcept -> decimal32_fast&
{
    *this = *this * rhs;
    return *this;
}

constexpr auto decimal32_fast::operator/=(decimal32_fast rhs) noexcept -> decimal32_fast&
{
    *this = *this / rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal32_fast::operator+=(Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast&)
{
    *this = *this + rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal32_fast::operator-=(Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast&)
{
    *this = *this - rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal32_fast::operator*=(Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast&)
{
    *this = *this * rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal32_fast::operator/=(Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast&)
{
    *this = *this / rhs;
    return *this;
}

constexpr auto decimal32_fast::operator++() noexcept -> decimal32_fast&
{
    constexpr decimal32_fast one(1, 0);
    *this = *this + one;
    return *this;
}

constexpr auto decimal32_fast::operator++(int) noexcept -> decimal32_fast&
{
    return ++(*this);
}

constexpr auto decimal32_fast::operator--() noexcept -> decimal32_fast&
{
    constexpr decimal32_fast one(1, 0);
    *this = *this - one;
    return *this;
}

constexpr auto decimal32_fast::operator--(int) noexcept -> decimal32_fast&
{
    return --(*this);
}

constexpr decimal32_fast::operator bool() const noexcept
{
    constexpr decimal32_fast zero {0, 0};
    return *this != zero;
}

constexpr decimal32_fast::operator int() const noexcept
{
    return to_integral<decimal32_fast, int>(*this);
}

constexpr decimal32_fast::operator unsigned() const noexcept
{
    return to_integral<decimal32_fast, unsigned>(*this);
}

constexpr decimal32_fast::operator long() const noexcept
{
    return to_integral<decimal32_fast, long>(*this);
}

constexpr decimal32_fast::operator unsigned long() const noexcept
{
    return to_integral<decimal32_fast, unsigned long>(*this);
}

constexpr decimal32_fast::operator long long() const noexcept
{
    return to_integral<decimal32_fast, long long>(*this);
}

constexpr decimal32_fast::operator unsigned long long() const noexcept
{
    return to_integral<decimal32_fast, unsigned long long>(*this);
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr decimal32_fast::operator detail::builtin_int128_t() const noexcept
{
    return to_integral<decimal32_fast, detail::builtin_int128_t>(*this);
}

constexpr decimal32_fast::operator detail::builtin_uint128_t() const noexcept
{
    return to_integral<decimal32_fast, detail::builtin_uint128_t>(*this);
}

#endif

BOOST_DECIMAL_CXX20_CONSTEXPR decimal32_fast::operator float() const noexcept
{
    return to_float<decimal32_fast, float>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal32_fast::operator double() const noexcept
{
    return to_float<decimal32_fast, double>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal32_fast::operator long double() const noexcept
{
    // TODO(mborland): Don't have an exact way of converting to various long doubles
    return static_cast<long double>(to_float<decimal32_fast, double>(*this));
}

#ifdef BOOST_DECIMAL_HAS_FLOAT16
constexpr decimal32_fast::operator std::float16_t() const noexcept
{
    return static_cast<std::float16_t>(to_float<decimal32_fast, float>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_FLOAT32
constexpr decimal32_fast::operator std::float32_t() const noexcept
{
    return static_cast<std::float32_t>(to_float<decimal32_fast, float>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_FLOAT64
constexpr decimal32_fast::operator std::float64_t() const noexcept
{
    return static_cast<std::float64_t>(to_float<decimal32_fast, double>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_BRAINFLOAT16
constexpr decimal32_fast::operator std::bfloat16_t() const noexcept
{
    return static_cast<std::bfloat16_t>(to_float<decimal32_fast, float>(*this));
}
#endif

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal> && (detail::decimal_val_v<Decimal> > detail::decimal_val_v<decimal32_fast>), bool>>
constexpr decimal32_fast::operator Decimal() const noexcept
{
    return to_decimal<Decimal>(*this);
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal> && (detail::decimal_val_v<Decimal> <= detail::decimal_val_v<decimal32_fast>), bool>>
constexpr decimal32_fast::operator Decimal() const noexcept
{
    return to_decimal<Decimal>(*this);
}

constexpr auto scalblnd32f(decimal32_fast num, long exp) noexcept -> decimal32_fast
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    constexpr decimal32_fast zero {0, 0};

    if (num == zero || exp == 0 || !isfinite(num))
    {
        return num;
    }
    #endif

    num = decimal32_fast(num.significand_, num.biased_exponent() + exp, num.sign_);

    return num;
}

constexpr auto scalbnd32f(decimal32_fast num, int expval) noexcept -> decimal32_fast
{
    return scalblnd32f(num, static_cast<long>(expval));
}

constexpr auto copysignd32f(decimal32_fast mag, decimal32_fast sgn) noexcept -> decimal32_fast
{
    mag.sign_ = sgn.sign_;
    return mag;
}

// Effects: determines if the quantum exponents of x and y are the same.
// If both x and y are NaN, or infinity, they have the same quantum exponents;
// if exactly one operand is infinity or exactly one operand is NaN, they do not have the same quantum exponents.
// The samequantum functions raise no exception.
constexpr auto samequantumd32f(decimal32_fast lhs, decimal32_fast rhs) noexcept -> bool
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

// Effects: if x is finite, returns its quantum exponent.
// Otherwise, a domain error occurs and INT_MIN is returned.
constexpr auto quantexpd32f(decimal32_fast x) noexcept -> int
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (!isfinite(x))
    {
        return INT_MIN;
    }
    #endif

    return static_cast<int>(x.unbiased_exponent());
}

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
constexpr auto quantized32f(decimal32_fast lhs, decimal32_fast rhs) noexcept -> decimal32_fast
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
        return direct_init(detail::d32_fast_snan, UINT8_C(0));
    }
    else if (isinf(lhs) && isinf(rhs))
    {
        return lhs;
    }
    #endif

    return {lhs.full_significand(), rhs.biased_exponent(), lhs.isneg()};
}

} // namespace decimal
} // namespace boost

namespace std {

BOOST_DECIMAL_EXPORT template <>
#ifdef _MSC_VER
class numeric_limits<boost::decimal::decimal32_fast>
#else
struct numeric_limits<boost::decimal::decimal32_fast>
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
    static constexpr std::float_denorm_style has_denorm = std::denorm_absent;
    static constexpr bool has_denorm_loss = false;
    #endif

    static constexpr std::float_round_style round_style = std::round_indeterminate;
    static constexpr bool is_iec559 = false;
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
    static constexpr bool traps = numeric_limits<std::uint_fast32_t>::traps;
    static constexpr bool tinyness_before = true;

    // Member functions
    static constexpr auto (min)        () -> boost::decimal::decimal32_fast { return {UINT32_C(1), min_exponent}; }
    static constexpr auto (max)        () -> boost::decimal::decimal32_fast { return {UINT32_C(9'999'999), max_exponent - digits + 1}; }
    static constexpr auto lowest       () -> boost::decimal::decimal32_fast { return {UINT32_C(9'999'999), max_exponent - digits + 1, true}; }
    static constexpr auto epsilon      () -> boost::decimal::decimal32_fast { return {UINT32_C(1), -digits + 1}; }
    static constexpr auto round_error  () -> boost::decimal::decimal32_fast { return epsilon(); }
    static constexpr auto infinity     () -> boost::decimal::decimal32_fast { return boost::decimal::direct_init(boost::decimal::detail::d32_fast_inf, UINT8_C((0))); }
    static constexpr auto quiet_NaN    () -> boost::decimal::decimal32_fast { return boost::decimal::direct_init(boost::decimal::detail::d32_fast_qnan, UINT8_C((0))); }
    static constexpr auto signaling_NaN() -> boost::decimal::decimal32_fast { return boost::decimal::direct_init(boost::decimal::detail::d32_fast_snan, UINT8_C((0))); }

    // With denorm absent returns the same value as min
    static constexpr auto denorm_min   () -> boost::decimal::decimal32_fast { return min(); }
};

} // Namespace std

#endif //BOOST_DECIMAL_DECIMAL32_FAST_HPP
