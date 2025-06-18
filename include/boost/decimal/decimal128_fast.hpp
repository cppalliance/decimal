// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DECIMAL128_FAST_HPP
#define BOOST_DECIMAL_DECIMAL128_FAST_HPP

#include <boost/decimal/decimal128.hpp>
#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/add_impl.hpp>
#include <boost/decimal/detail/sub_impl.hpp>
#include <boost/decimal/detail/mul_impl.hpp>
#include <boost/decimal/detail/div_impl.hpp>
#include <boost/decimal/detail/ryu/ryu_generic_128.hpp>
#include <boost/decimal/detail/cmath/next.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE

#include <limits>
#include <cstdint>

#endif

namespace boost {
namespace decimal {

namespace detail {

BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d128_fast_inf = boost::int128::uint128_t {UINT64_MAX - 2, UINT64_MAX};
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d128_fast_qnan = boost::int128::uint128_t {UINT64_MAX - 1, UINT64_MAX};
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d128_fast_snan = boost::int128::uint128_t {UINT64_MAX, UINT64_MAX};

BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d128_fast_inf_high_bits = UINT64_MAX - 2;
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d128_fast_qnan_high_bits = UINT64_MAX - 1;
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d128_fast_snan_high_bits = UINT64_MAX;

} // namespace detail

BOOST_DECIMAL_EXPORT class decimal128_fast final
{
public:
    using significand_type = int128::uint128_t;
    using exponent_type = std::uint_fast32_t;
    using biased_exponent_type = std::int_fast32_t;

private:
    // Instead of having to encode and decode at every operation
    // we store the constituent pieces directly

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
        return static_cast<biased_exponent_type>(exponent_) - detail::bias_v<decimal128>;
    }

    template <typename Decimal, typename TargetType>
    friend constexpr auto to_integral_128(Decimal val) noexcept
        BOOST_DECIMAL_REQUIRES_TWO_RETURN(detail::is_decimal_floating_point_v, Decimal, detail::is_integral_v, TargetType, TargetType);

    template <typename Decimal, typename TargetType>
    friend BOOST_DECIMAL_CXX20_CONSTEXPR auto to_float(Decimal val) noexcept
        BOOST_DECIMAL_REQUIRES_TWO_RETURN(detail::is_decimal_floating_point_v, Decimal, detail::is_floating_point_v, TargetType, TargetType);

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetType, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal>
        friend constexpr auto to_decimal(Decimal val) noexcept -> TargetType;

    friend constexpr auto d128f_div_impl(decimal128_fast lhs, decimal128_fast rhs, decimal128_fast& q, decimal128_fast& r) noexcept -> void;

    // Equality template between any integer type and decimal128
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, BOOST_DECIMAL_INTEGRAL Integer>
    friend constexpr auto mixed_equality_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
    friend constexpr auto mixed_decimal_equality_impl(Decimal1 lhs, Decimal2 rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                             detail::is_decimal_floating_point_v<Decimal2>), bool>;

    // Template to compare operator< for any integer type and decimal128
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, BOOST_DECIMAL_INTEGRAL Integer>
    friend constexpr auto less_impl(Decimal lhs, Integer rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
    friend constexpr auto mixed_decimal_less_impl(Decimal1 lhs, Decimal2 rhs) noexcept
        -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                             detail::is_decimal_floating_point_v<Decimal2>), bool>;

    template <typename T>
    friend constexpr auto ilogb(T d) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, T, int);

    template <typename T>
    friend constexpr auto logb(T num) noexcept
        BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T);

    friend constexpr auto not_finite(const decimal128_fast& val) noexcept -> bool;

    template <typename DecimalType>
    friend constexpr auto to_dpd_d128(DecimalType val) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, DecimalType, int128::uint128_t);

    template <BOOST_DECIMAL_FAST_DECIMAL_FLOATING_TYPE DecimalType>
    BOOST_DECIMAL_FORCE_INLINE friend constexpr auto fast_equality_impl(const DecimalType& lhs, const DecimalType& rhs) noexcept -> bool;

    template <BOOST_DECIMAL_FAST_DECIMAL_FLOATING_TYPE DecimalType>
    BOOST_DECIMAL_FORCE_INLINE friend constexpr auto fast_inequality_impl(const DecimalType& lhs, const DecimalType& rhs) noexcept -> bool;

    template <BOOST_DECIMAL_FAST_DECIMAL_FLOATING_TYPE DecimalType>
    BOOST_DECIMAL_FORCE_INLINE friend constexpr auto fast_less_impl(const DecimalType& lhs, const DecimalType& rhs) noexcept -> bool;

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
    friend constexpr auto detail::nextafter_impl(DecimalType val, bool direction) noexcept -> DecimalType;

public:
    constexpr decimal128_fast() noexcept = default;

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
    #else
    template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1> && detail::is_integral_v<T2>, bool> = true>
    #endif
    constexpr decimal128_fast(T1 coeff, T2 exp, bool sign = false) noexcept;

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_INTEGRAL Integer>
    #else
    template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool> = true>
    #endif
    constexpr decimal128_fast(Integer val) noexcept;

    #ifdef BOOST_DECIMAL_HAS_CONCEPTS
    template <BOOST_DECIMAL_REAL Float>
    #else
    template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool> = true>
    #endif
    explicit BOOST_DECIMAL_CXX20_CONSTEXPR decimal128_fast(Float val) noexcept;

    friend constexpr auto direct_init_d128(significand_type significand, exponent_type exponent, bool sign) noexcept -> decimal128_fast;

    // Classification functions
    friend constexpr auto signbit(decimal128_fast val) noexcept -> bool;
    friend constexpr auto isinf(decimal128_fast val) noexcept -> bool;
    friend constexpr auto isnan(decimal128_fast val) noexcept -> bool;
    friend constexpr auto issignaling(decimal128_fast val) noexcept -> bool;
    friend constexpr auto isnormal(decimal128_fast val) noexcept -> bool;
    friend constexpr auto isfinite(decimal128_fast val) noexcept -> bool;

    // Comparison operators
    friend constexpr auto operator==(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool;
    friend constexpr auto operator!=(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool;
    friend constexpr auto operator<(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool;
    friend constexpr auto operator<=(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool;
    friend constexpr auto operator>(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool;
    friend constexpr auto operator>=(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool;

    // Mixed comparison operators
    template <typename Integer>
    friend constexpr auto operator==(decimal128_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator==(Integer lhs, decimal128_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator!=(decimal128_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator!=(Integer lhs, decimal128_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator<(decimal128_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator<(Integer lhs, decimal128_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator<=(decimal128_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator<=(Integer lhs, decimal128_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator>(decimal128_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator>(Integer lhs, decimal128_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator>=(decimal128_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    template <typename Integer>
    friend constexpr auto operator>=(Integer lhs, decimal128_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool);

    #ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR
    friend constexpr auto operator<=>(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> std::partial_ordering;

    template <typename Integer>
    friend constexpr auto operator<=>(const decimal128_fast& lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering);

    template <typename Integer>
    friend constexpr auto operator<=>(Integer lhs, const decimal128_fast& rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, std::partial_ordering);
    #endif

    // Unary arithmetic operators
    friend constexpr auto operator+(decimal128_fast rhs) noexcept -> decimal128_fast;
    friend constexpr auto operator-(decimal128_fast rhs) noexcept -> decimal128_fast;

    // Binary arithmetic operators
    friend constexpr auto operator+(decimal128_fast lhs, decimal128_fast rhs) noexcept -> decimal128_fast;
    friend constexpr auto operator-(decimal128_fast lhs, decimal128_fast rhs) noexcept -> decimal128_fast;
    friend constexpr auto operator*(decimal128_fast lhs, decimal128_fast rhs) noexcept -> decimal128_fast;
    friend constexpr auto operator/(decimal128_fast lhs, decimal128_fast rhs) noexcept -> decimal128_fast;
    friend constexpr auto operator%(decimal128_fast lhs, decimal128_fast rhs) noexcept -> decimal128_fast;

    // Mixed type binary arithmetic operators
    template <typename Integer>
    friend constexpr auto operator+(decimal128_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast);

    template <typename Integer>
    friend constexpr auto operator+(Integer lhs, decimal128_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast);

    template <typename Integer>
    friend constexpr auto operator-(decimal128_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast);

    template <typename Integer>
    friend constexpr auto operator-(Integer lhs, decimal128_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast);

    template <typename Integer>
    friend constexpr auto operator*(decimal128_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast);

    template <typename Integer>
    friend constexpr auto operator*(Integer lhs, decimal128_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast);

    template <typename Integer>
    friend constexpr auto operator/(decimal128_fast lhs, Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast);

    template <typename Integer>
    friend constexpr auto operator/(Integer lhs, decimal128_fast rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast);

    // Compound Arithmetic Operators
    constexpr auto operator+=(decimal128_fast rhs) noexcept -> decimal128_fast&;

    template <typename Integer>
    constexpr auto operator+=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast&);

    constexpr auto operator-=(decimal128_fast rhs) noexcept -> decimal128_fast&;

    template <typename Integer>
    constexpr auto operator-=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast&);

    constexpr auto operator*=(decimal128_fast rhs) noexcept -> decimal128_fast&;

    template <typename Integer>
    constexpr auto operator*=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast&);

    constexpr auto operator/=(decimal128_fast rhs) noexcept -> decimal128_fast&;

    template <typename Integer>
    constexpr auto operator/=(Integer rhs) noexcept
        BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast&);

    // Conversions
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
    explicit constexpr operator int128::int128_t() const noexcept;
    explicit constexpr operator int128::uint128_t() const noexcept;
    #endif

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

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool> = true>
    explicit constexpr operator Decimal() const noexcept;

    // <cmath> functions that are better as friends
    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
    friend constexpr auto frexp10(T num, int* expptr) noexcept -> typename T::significand_type;

    friend constexpr auto copysignd128f(decimal128_fast mag, decimal128_fast sgn) noexcept -> decimal128_fast;
    friend constexpr auto scalblnd128f(decimal128_fast num, long exp) noexcept -> decimal128_fast;
    friend constexpr auto scalbnd128f(decimal128_fast num, int exp) noexcept -> decimal128_fast;
    friend constexpr auto fmad128f(decimal128_fast x, decimal128_fast y, decimal128 z) noexcept -> decimal128;

    // Decimal functions
    // 3.6.4 Same Quantum
    friend constexpr auto samequantumd128f(decimal128_fast lhs, decimal128_fast rhs) noexcept -> bool;

    // 3.6.5 Quantum exponent
    friend constexpr auto quantexpd128f(decimal128_fast x) noexcept -> int;

    // 3.6.6 Quantize
    friend constexpr auto quantized128f(decimal128_fast lhs, decimal128_fast rhs) noexcept -> decimal128_fast;
};

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
#else
template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1> && detail::is_integral_v<T2>, bool>>
#endif
constexpr decimal128_fast::decimal128_fast(T1 coeff, T2 exp, bool sign) noexcept
{
    using Unsigned_Integer = int128::uint128_t;

    using Basis_Unsigned_Integer = std::conditional_t<std::numeric_limits<Unsigned_Integer>::digits10 < std::numeric_limits<significand_type>::digits10, significand_type, Unsigned_Integer>;

    const bool isneg {coeff < static_cast<T1>(0) || sign};
    sign_ = isneg;
    auto unsigned_coeff {static_cast<Basis_Unsigned_Integer>(detail::make_positive_unsigned(coeff))};

    // Normalize the significand in the constructor, so we don't have
    // to calculate the number of digits for operationss
    detail::normalize<decimal128_fast>(unsigned_coeff, exp, sign);

    significand_ = unsigned_coeff;

    // Normalize the handling of 0
    if (significand_ == int128::uint128_t{UINT64_C(0), UINT64_C(0)})
    {
        exp = 0;
    }

    const auto biased_exp {exp + detail::bias_v<decimal128>};

    if (biased_exp > detail::max_biased_exp_v<decimal128>)
    {
        significand_ = detail::d128_fast_inf;
    }
    else if (biased_exp >= 0)
    {
        exponent_ = static_cast<exponent_type>(biased_exp);
    }
    else
    {
        // Flush denorms to zero
        significand_ = static_cast<significand_type>(0);
        exponent_ = static_cast<exponent_type>(0 + detail::bias_v<decimal128>);
        sign_ = false;
    }
}

#ifdef BOOST_DECIMAL_HAS_CONCEPTS
template <BOOST_DECIMAL_INTEGRAL Integer>
#else
template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool>>
#endif
constexpr decimal128_fast::decimal128_fast(Integer val) noexcept : decimal128_fast{val, 0}
{
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
BOOST_DECIMAL_CXX20_CONSTEXPR decimal128_fast::decimal128_fast(Float val) noexcept
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (val != val)
    {
        significand_ = detail::d128_fast_qnan;
    }
    else if (val == std::numeric_limits<Float>::infinity() || val == -std::numeric_limits<Float>::infinity())
    {
        significand_ = detail::d128_fast_inf;
    }
    else
    #endif
    {
        const auto components {detail::ryu::floating_point_to_fd128(val)};
        *this = decimal128_fast {components.mantissa, components.exponent, components.sign};
    }
}

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

constexpr auto direct_init_d128(decimal128_fast::significand_type significand, decimal128_fast::exponent_type exponent, bool sign) noexcept -> decimal128_fast
{
    decimal128_fast val {};
    val.significand_ = significand;
    val.exponent_ = exponent;
    val.sign_ = sign;

    return val;
}

constexpr auto signbit(decimal128_fast val) noexcept -> bool
{
    return val.sign_;
}

constexpr auto isinf(decimal128_fast val) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return val.significand_.high == detail::d128_fast_inf_high_bits;
    #else
    static_cast<void>(val);
    return false;
    #endif
}

constexpr auto isnan(decimal128_fast val) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return val.significand_.high >= detail::d128_fast_qnan_high_bits;
    #else
    static_cast<void>(val);
    return false;
    #endif
}

constexpr auto issignaling(decimal128_fast val) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return val.significand_.high == detail::d128_fast_snan_high_bits;
    #else
    static_cast<void>(val);
    return false;
    #endif
}

constexpr auto isnormal(decimal128_fast val) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (val.exponent_ <= static_cast<decimal128_fast::exponent_type>(detail::precision_v<decimal128> - 1))
    {
        return false;
    }

    return (val.significand_ != 0U) && isfinite(val);
    #else
    return val.significand_ != 0U;
    #endif
}

constexpr auto isfinite(decimal128_fast val) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return val.significand_.high < detail::d128_fast_inf_high_bits;
    #else
    static_cast<void>(val);
    return true;
    #endif
}

constexpr auto not_finite(const decimal128_fast& val) noexcept -> bool
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    return val.significand_.high >= detail::d128_fast_inf_high_bits;
    #else
    static_cast<void>(val);
    return false;
    #endif
}

constexpr auto operator==(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool
{
    return fast_equality_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator==(decimal128_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return mixed_equality_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator==(Integer lhs, decimal128_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return mixed_equality_impl(rhs, lhs);
}

constexpr auto operator!=(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool
{
    return fast_inequality_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator!=(decimal128_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return !(lhs == rhs);
}

template <typename Integer>
constexpr auto operator!=(Integer lhs, decimal128_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return !(lhs == rhs);
}

constexpr auto operator<(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool
{
    return fast_less_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator<(decimal128_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return less_impl(lhs, rhs);
}

template <typename Integer>
constexpr auto operator<(Integer lhs, decimal128_fast rhs) noexcept
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

constexpr auto operator<=(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool
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
constexpr auto operator<=(decimal128_fast lhs, Integer rhs) noexcept
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
constexpr auto operator<=(Integer lhs, decimal128_fast rhs) noexcept
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

constexpr auto operator>(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool
{
    return rhs < lhs;
}

template <typename Integer>
constexpr auto operator>(decimal128_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return rhs < lhs;
}

template <typename Integer>
constexpr auto operator>(Integer lhs, decimal128_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return rhs < lhs;
}

constexpr auto operator>=(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> bool
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
constexpr auto operator>=(decimal128_fast lhs, Integer rhs) noexcept
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
constexpr auto operator>=(Integer lhs, decimal128_fast rhs) noexcept
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

constexpr auto operator<=>(const decimal128_fast& lhs, const decimal128_fast& rhs) noexcept -> std::partial_ordering
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
constexpr auto operator<=>(const decimal128_fast& lhs, Integer rhs) noexcept
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
constexpr auto operator<=>(Integer lhs, const decimal128_fast& rhs) noexcept
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

constexpr auto operator+(decimal128_fast rhs) noexcept -> decimal128_fast
{
    return rhs;
}

constexpr auto operator-(decimal128_fast rhs) noexcept -> decimal128_fast
{
    rhs.sign_ = !rhs.sign_;
    return rhs;
}

constexpr auto operator+(decimal128_fast lhs, decimal128_fast rhs) noexcept -> decimal128_fast
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs) || not_finite(rhs))
    {
        return detail::check_non_finite(lhs, rhs);
    }
    #endif

    return detail::d128_add_impl<decimal128_fast>(
            lhs.significand_, lhs.biased_exponent(), lhs.sign_,
            rhs.significand_, rhs.biased_exponent(), rhs.sign_,
            (abs(lhs) > abs(rhs)));
};

template <typename Integer>
constexpr auto operator+(decimal128_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast)
{
    using exp_type = decimal128_fast::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs))
    {
        return lhs;
    }
    #endif

    auto sig_rhs {static_cast<int128::uint128_t>(detail::make_positive_unsigned(rhs))};
    bool abs_lhs_bigger {abs(lhs) > sig_rhs};

    exp_type exp_rhs {0};
    detail::normalize<decimal128>(sig_rhs, exp_rhs);

    return detail::d128_add_impl<decimal128_fast>(lhs.significand_, lhs.biased_exponent(), lhs.sign_,
                                                  sig_rhs, exp_rhs, (rhs < 0),
                                                  abs_lhs_bigger);
}

template <typename Integer>
constexpr auto operator+(Integer lhs, decimal128_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast)
{
    return rhs + lhs;
}

constexpr auto operator-(decimal128_fast lhs, decimal128_fast rhs) noexcept -> decimal128_fast
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs) || not_finite(rhs))
    {
        return detail::check_non_finite(lhs, rhs);
    }
    #endif

    return detail::d128_sub_impl<decimal128_fast>(
            lhs.significand_, lhs.biased_exponent(), lhs.sign_,
            rhs.significand_, rhs.biased_exponent(), rhs.sign_,
            abs(lhs) > abs(rhs));
}

template <typename Integer>
constexpr auto operator-(decimal128_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast)
{
    using exp_type = decimal128_fast::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs))
    {
        return lhs;
    }
    #endif

    auto sig_rhs {static_cast<int128::uint128_t>(detail::make_positive_unsigned(rhs))};
    const bool abs_lhs_bigger {abs(lhs) > sig_rhs};

    exp_type exp_rhs {0};
    detail::normalize<decimal128>(sig_rhs, exp_rhs);

    return detail::d128_sub_impl<decimal128_fast>(
            lhs.significand_, lhs.biased_exponent(), lhs.sign_,
            sig_rhs, exp_rhs, (rhs < 0),
            abs_lhs_bigger);
}

template <typename Integer>
constexpr auto operator-(Integer lhs, decimal128_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast)
{
    using exp_type = decimal128_fast::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(rhs))
    {
        return rhs;
    }
    #endif

    auto sig_lhs {static_cast<int128::uint128_t>(detail::make_positive_unsigned(lhs))};
    const bool abs_lhs_bigger {sig_lhs > abs(rhs)};

    exp_type exp_lhs {0};
    detail::normalize<decimal128>(sig_lhs, exp_lhs);

    return detail::d128_sub_impl<decimal128_fast>(
            sig_lhs, exp_lhs, (lhs < 0),
            rhs.significand_, rhs.biased_exponent(), rhs.sign_,
            abs_lhs_bigger);
}

constexpr auto operator*(decimal128_fast lhs, decimal128_fast rhs) noexcept -> decimal128_fast
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs) || not_finite(rhs))
    {
        return detail::check_non_finite(lhs, rhs);
    }
    #endif

    auto lhs_sig {lhs.full_significand()};
    auto lhs_exp {lhs.biased_exponent()};
    const auto lhs_zeros {detail::remove_trailing_zeros(lhs_sig)};
    lhs_sig = lhs_zeros.trimmed_number;
    lhs_exp += static_cast<std::int32_t>(lhs_zeros.number_of_removed_zeros);

    auto rhs_sig {rhs.full_significand()};
    auto rhs_exp {rhs.biased_exponent()};
    const auto rhs_zeros {detail::remove_trailing_zeros(rhs_sig)};
    rhs_sig = rhs_zeros.trimmed_number;
    rhs_exp += static_cast<std::int32_t>(rhs_zeros.number_of_removed_zeros);

    return detail::d128_mul_impl<decimal128_fast>(
            lhs_sig, lhs_exp, lhs.sign_,
            rhs_sig, rhs_exp, rhs.sign_);
}

template <typename Integer>
constexpr auto operator*(decimal128_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast)
{
    using exp_type = decimal128_fast::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (not_finite(lhs))
    {
        return lhs;
    }
    #endif

    auto rhs_sig {static_cast<int128::uint128_t>(detail::make_positive_unsigned(rhs))};
    exp_type rhs_exp {0};
    detail::normalize<decimal128_fast>(rhs_sig, rhs_exp);

    return detail::d128_fast_mul_impl<decimal128_fast>(
            lhs.significand_, lhs.biased_exponent(), lhs.sign_,
            rhs_sig, rhs_exp, (rhs < 0));
}

template <typename Integer>
constexpr auto operator*(Integer lhs, decimal128_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast)
{
    return rhs * lhs;
}

constexpr auto d128f_div_impl(decimal128_fast lhs, decimal128_fast rhs, decimal128_fast& q, decimal128_fast& r) noexcept -> void
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check pre-conditions
    constexpr decimal128_fast zero {0, 0};
    constexpr decimal128_fast nan {boost::decimal::direct_init_d128(boost::decimal::detail::d128_fast_qnan, 0, false)};
    constexpr decimal128_fast inf {boost::decimal::direct_init_d128(boost::decimal::detail::d128_fast_inf, 0, false)};

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

    #ifdef BOOST_DECIMAL_DEBUG
    std::cerr << "sig lhs: " << sig_lhs
              << "\nexp lhs: " << exp_lhs
              << "\nsig rhs: " << sig_rhs
              << "\nexp rhs: " << exp_rhs << std::endl;
    #endif

    detail::decimal128_fast_components lhs_components {lhs.significand_, lhs.biased_exponent(), lhs.isneg()};
    detail::decimal128_fast_components rhs_components {rhs.significand_, rhs.biased_exponent(), rhs.isneg()};
    detail::decimal128_fast_components q_components {};

    detail::d128_generic_div_impl(lhs_components, rhs_components, q_components);

    q = decimal128_fast(q_components.sig, q_components.exp, q_components.sign);
}

constexpr auto d128f_mod_impl(decimal128_fast lhs, decimal128_fast rhs, const decimal128_fast& q, decimal128_fast& r) -> void
{
    constexpr decimal128_fast zero {0, 0};

    auto q_trunc {q > zero ? floor(q) : ceil(q)};
    r = lhs - (decimal128_fast(q_trunc) * rhs);
};

constexpr auto operator/(decimal128_fast lhs, decimal128_fast rhs) noexcept -> decimal128_fast
{
    decimal128_fast q {};
    decimal128_fast r {};
    d128f_div_impl(lhs, rhs, q, r);

    return q;
};

template <typename Integer>
constexpr auto operator/(decimal128_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check pre-conditions
    constexpr decimal128_fast zero {0, 0};
    constexpr decimal128_fast nan {boost::decimal::direct_init_d128(boost::decimal::detail::d128_fast_qnan, 0, false)};
    constexpr decimal128_fast inf {boost::decimal::direct_init_d128(boost::decimal::detail::d128_fast_inf, 0, false)};

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

    detail::decimal128_fast_components lhs_components {lhs.significand_, lhs.biased_exponent(), lhs.isneg()};

    auto rhs_sig {detail::make_positive_unsigned(rhs)};
    detail::decimal128_fast_components rhs_components {rhs_sig, 0, rhs < 0};
    detail::decimal128_fast_components q_components {};

    detail::d128_generic_div_impl(lhs_components, rhs_components, q_components);

    return {q_components.sig, q_components.exp, q_components.sign};
}

template <typename Integer>
constexpr auto operator/(Integer lhs, decimal128_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    // Check pre-conditions
    constexpr decimal128_fast zero {0, 0};
    constexpr decimal128_fast nan {boost::decimal::direct_init_d128(boost::decimal::detail::d128_fast_qnan, 0, false)};
    constexpr decimal128_fast inf {boost::decimal::direct_init_d128(boost::decimal::detail::d128_fast_inf, 0, false)};

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

    detail::decimal128_fast_components lhs_components {detail::make_positive_unsigned(lhs), 0, lhs < 0};
    detail::decimal128_fast_components rhs_components {rhs.significand_, rhs.biased_exponent(), rhs.isneg()};
    detail::decimal128_fast_components q_components {};

    detail::d128_generic_div_impl(lhs_components, rhs_components, q_components);

    return {q_components.sig, q_components.exp, q_components.sign};
}

constexpr auto operator%(decimal128_fast lhs, decimal128_fast rhs) noexcept -> decimal128_fast
{
    decimal128_fast q {};
    decimal128_fast r {};
    d128f_div_impl(lhs, rhs, q, r);
    d128f_mod_impl(lhs, rhs, q, r);

    return r;
};

constexpr auto decimal128_fast::operator+=(decimal128_fast rhs) noexcept -> decimal128_fast&
{
    *this = *this + rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal128_fast::operator+=(Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast&)
{
    *this = *this + rhs;
    return *this;
}

constexpr auto decimal128_fast::operator-=(decimal128_fast rhs) noexcept -> decimal128_fast&
{
    *this = *this - rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal128_fast::operator-=(Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast&)
{
    *this = *this - rhs;
    return *this;
}

constexpr auto decimal128_fast::operator*=(decimal128_fast rhs) noexcept -> decimal128_fast&
{
    *this = *this * rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal128_fast::operator*=(Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast&)
{
    *this = *this * rhs;
    return *this;
}

constexpr auto decimal128_fast::operator/=(decimal128_fast rhs) noexcept -> decimal128_fast&
{
    *this = *this / rhs;
    return *this;
}

template <typename Integer>
constexpr auto decimal128_fast::operator/=(Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal128_fast&)
{
    *this = *this / rhs;
    return *this;
}

constexpr decimal128_fast::operator bool() const noexcept
{
    constexpr decimal128_fast zero {0, 0};
    return *this != zero;
}

constexpr decimal128_fast::operator int() const noexcept
{
    return to_integral_128<decimal128_fast, int>(*this);
}

constexpr decimal128_fast::operator unsigned() const noexcept
{
    return to_integral_128<decimal128_fast, unsigned>(*this);
}

constexpr decimal128_fast::operator long() const noexcept
{
    return to_integral_128<decimal128_fast, long>(*this);
}

constexpr decimal128_fast::operator unsigned long() const noexcept
{
    return to_integral_128<decimal128_fast, unsigned long>(*this);
}

constexpr decimal128_fast::operator long long() const noexcept
{
    return to_integral_128<decimal128_fast, long long>(*this);
}

constexpr decimal128_fast::operator unsigned long long() const noexcept
{
    return to_integral_128<decimal128_fast, unsigned long long>(*this);
}

constexpr decimal128_fast::operator std::int8_t() const noexcept
{
    return to_integral_128<decimal128_fast, std::int8_t>(*this);
}

constexpr decimal128_fast::operator std::uint8_t() const noexcept
{
    return to_integral_128<decimal128_fast, std::uint8_t>(*this);
}

constexpr decimal128_fast::operator std::int16_t() const noexcept
{
    return to_integral_128<decimal128_fast, std::int16_t>(*this);
}

constexpr decimal128_fast::operator std::uint16_t() const noexcept
{
    return to_integral_128<decimal128_fast, std::uint16_t>(*this);
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr decimal128_fast::operator boost::int128::int128_t() const noexcept
{
    return to_integral_128<decimal128_fast, int128::int128_t>(*this);
}

constexpr decimal128_fast::operator boost::int128::uint128_t() const noexcept
{
    return to_integral_128<decimal128_fast, int128::uint128_t>(*this);
}

#endif // BOOST_DECIMAL_HAS_INT128

BOOST_DECIMAL_CXX20_CONSTEXPR decimal128_fast::operator float() const noexcept
{
    return to_float<decimal128_fast, float>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal128_fast::operator double() const noexcept
{
    return to_float<decimal128_fast, double>(*this);
}

BOOST_DECIMAL_CXX20_CONSTEXPR decimal128_fast::operator long double() const noexcept
{
    return to_float<decimal128_fast, long double>(*this);
}

#ifdef BOOST_DECIMAL_HAS_FLOAT16
constexpr decimal128_fast::operator std::float16_t() const noexcept
{
    return static_cast<std::float16_t>(to_float<decimal128_fast, float>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_FLOAT32
constexpr decimal128_fast::operator std::float32_t() const noexcept
{
    return static_cast<std::float32_t>(to_float<decimal128_fast, float>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_FLOAT64
constexpr decimal128_fast::operator std::float64_t() const noexcept
{
    return static_cast<std::float64_t>(to_float<decimal128_fast, double>(*this));
}
#endif
#ifdef BOOST_DECIMAL_HAS_BRAINFLOAT16
constexpr decimal128_fast::operator std::bfloat16_t() const noexcept
{
    return static_cast<std::bfloat16_t>(to_float<decimal128_fast, float>(*this));
}
#endif

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool>>
constexpr decimal128_fast::operator Decimal() const noexcept
{
    return to_decimal<Decimal>(*this);
}

constexpr auto copysignd128f(decimal128_fast mag, decimal128_fast sgn) noexcept -> decimal128_fast
{
    mag.sign_ = sgn.sign_;
    return mag;
}

constexpr auto scalblnd128f(decimal128_fast num, long exp) noexcept -> decimal128_fast
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    constexpr decimal128_fast zero {0, 0};

    if (num == zero || exp == 0 || not_finite(num))
    {
        return num;
    }
    #endif

    num = decimal128_fast(num.significand_, num.biased_exponent() + exp, num.sign_);

    return num;
}

constexpr auto scalbnd128f(decimal128_fast num, int exp) noexcept -> decimal128_fast
{
    return scalblnd128f(num, static_cast<long>(exp));
}

// 3.6.4
// Effects: determines if the quantum exponents of x and y are the same.
// If both x and y are NaN, or infinity, they have the same quantum exponents;
// if exactly one operand is infinity or exactly one operand is NaN, they do not have the same quantum exponents.
// The samequantum functions raise no exception.
constexpr auto samequantumd128f(decimal128_fast lhs, decimal128_fast rhs) noexcept -> bool
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
constexpr auto quantexpd128f(decimal128_fast x) noexcept -> int
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
constexpr auto quantized128f(decimal128_fast lhs, decimal128_fast rhs) noexcept -> decimal128_fast
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
        return boost::decimal::direct_init_d128(boost::decimal::detail::d128_fast_qnan, 0, false);
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

BOOST_DECIMAL_EXPORT template<>
#ifdef _MSC_VER
class numeric_limits<boost::decimal::decimal128_fast>
#else
struct numeric_limits<boost::decimal::decimal128_fast>
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
    static constexpr bool is_iec559 = false;
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
    static constexpr auto (min)        () -> boost::decimal::decimal128_fast { return {1, min_exponent}; }
    static constexpr auto (max)        () -> boost::decimal::decimal128_fast { return {boost::int128::uint128_t{UINT64_C(0b1111011010000100110111110101011011000011111000000), UINT64_C(0b0011011110001101100011100110001111111111111111111111111111111111)}, max_exponent - digits + 1}; }
    static constexpr auto lowest       () -> boost::decimal::decimal128_fast { return {boost::int128::uint128_t{UINT64_C(0b1111011010000100110111110101011011000011111000000), UINT64_C(0b0011011110001101100011100110001111111111111111111111111111111111)}, max_exponent - digits + 1, true}; }
    static constexpr auto epsilon      () -> boost::decimal::decimal128_fast { return {1, -digits + 1}; }
    static constexpr auto round_error  () -> boost::decimal::decimal128_fast { return epsilon(); }
    static constexpr auto infinity     () -> boost::decimal::decimal128_fast { return boost::decimal::direct_init_d128(boost::decimal::detail::d128_fast_inf, 0, false); }
    static constexpr auto quiet_NaN    () -> boost::decimal::decimal128_fast { return boost::decimal::direct_init_d128(boost::decimal::detail::d128_fast_qnan, 0, false); }
    static constexpr auto signaling_NaN() -> boost::decimal::decimal128_fast { return boost::decimal::direct_init_d128(boost::decimal::detail::d128_fast_snan, 0, false); }
    static constexpr auto denorm_min   () -> boost::decimal::decimal128_fast { return min(); }
};

} // namspace std

#endif //BOOST_DECIMAL_DECIMAL128_FAST_HPP
