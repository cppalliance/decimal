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
#include <boost/decimal/detail/sub_impl.hpp>
#include <boost/decimal/detail/mul_impl.hpp>
#include <boost/decimal/detail/ryu/ryu_generic_128.hpp>
#include <limits>
#include <cstdint>

namespace boost {
namespace decimal {

namespace detail {

BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d32_fast_inf = std::numeric_limits<std::uint_fast32_t>::max();
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d32_fast_qnan = std::numeric_limits<std::uint_fast32_t>::max() - 1;
BOOST_DECIMAL_CONSTEXPR_VARIABLE auto d32_fast_snan = std::numeric_limits<std::uint_fast32_t>::max() - 2;

struct decimal32_fast_components
{
    using sig_type = std::uint_fast32_t;

    std::uint_fast32_t sig;
    std::int32_t exp;
    bool sign;
};

}

class decimal32_fast final
{
public:
    using significand_type = std::uint_fast32_t;

private:
    // In regular decimal32 we have to decode the 24 bits of the significand and the 8 bits of the exp
    // Here we just use them directly at the cost of at least 2 extra bytes of internal state
    // since the fast integer types will be at least 32 and 8 bits respectively

    std::uint_fast32_t significand_ {};
    std::uint_fast8_t exponent_ {};
    bool sign_ {};

    constexpr auto isneg() const noexcept -> bool
    {
        return sign_;
    }

    constexpr auto full_significand() const noexcept -> std::uint_fast32_t
    {
        return significand_;
    }

    constexpr auto unbiased_exponent() const noexcept -> std::uint_fast8_t
    {
        return exponent_;
    }

    constexpr auto biased_exponent() const noexcept -> std::int32_t
    {
        return static_cast<std::int32_t>(exponent_) - detail::bias_v<decimal32>;
    }

    friend constexpr auto div_impl(decimal32_fast lhs, decimal32_fast rhs, decimal32_fast& q, decimal32_fast& r) noexcept -> void;

    friend constexpr auto mod_impl(decimal32_fast lhs, decimal32_fast rhs, const decimal32_fast& q, decimal32_fast& r) noexcept -> void;

    // Attempts conversion to integral type:
    // If this is nan sets errno to EINVAL and returns 0
    // If this is not representable sets errno to ERANGE and returns 0
    template <typename Decimal, typename TargetType>
    friend constexpr auto to_integral(Decimal val) noexcept
        BOOST_DECIMAL_REQUIRES_TWO_RETURN(detail::is_decimal_floating_point_v, Decimal, detail::is_integral_v, TargetType, TargetType);

    template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
    friend constexpr auto frexp10(T num, int* expptr) noexcept -> typename T::significand_type;

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

public:
    constexpr decimal32_fast() noexcept {}

    template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1> && detail::is_integral_v<T2>, bool> = true>
    constexpr decimal32_fast(T1 coeff, T2 exp, bool sign = false) noexcept;

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

    // Compound operators
    constexpr auto operator+=(decimal32_fast rhs) noexcept -> decimal32_fast&;
    constexpr auto operator-=(decimal32_fast rhs) noexcept -> decimal32_fast&;
    constexpr auto operator*=(decimal32_fast rhs) noexcept -> decimal32_fast&;
    constexpr auto operator/=(decimal32_fast rhs) noexcept -> decimal32_fast&;
    constexpr auto operator%=(decimal32_fast rhs) noexcept -> decimal32_fast&;

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

    friend constexpr auto direct_init(std::uint_fast32_t significand, std::uint_fast8_t exponent, bool sign) noexcept -> decimal32_fast;
};

template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1> && detail::is_integral_v<T2>, bool>>
constexpr decimal32_fast::decimal32_fast(T1 coeff, T2 exp, bool sign) noexcept
{
    using Unsigned_Integer = detail::make_unsigned_t<T1>;

    const bool isneg {coeff < static_cast<T1>(0) || sign};
    sign_ = isneg;
    Unsigned_Integer unsigned_coeff {detail::make_positive_unsigned(coeff)};

    auto unsigned_coeff_digits {detail::num_digits(unsigned_coeff)};
    const bool reduced {unsigned_coeff_digits > detail::precision_v<decimal32>};

    // Strip digits and round as required
    if (reduced)
    {
        const auto digits_to_remove {static_cast<Unsigned_Integer>(unsigned_coeff_digits - (detail::precision_v<decimal32> + 1))};

        #if defined(__GNUC__) && !defined(__clang__)
        #  pragma GCC diagnostic push
        #  pragma GCC diagnostic ignored "-Wconversion"
        #endif

        unsigned_coeff /= static_cast<Unsigned_Integer>(detail::pow10(digits_to_remove));

        #if defined(__GNUC__) && !defined(__clang__)
        #  pragma GCC diagnostic pop
        #endif

        exp += static_cast<std::uint_fast8_t>(digits_to_remove);
        exp += static_cast<T2>(detail::fenv_round(unsigned_coeff, isneg));
    }

    significand_ = static_cast<std::uint_fast32_t>(unsigned_coeff);

    // Normalize the handling of zeros
    if (significand_ == UINT32_C(0))
    {
        exp = 0;
    }

    auto biased_exp {static_cast<std::uint_fast32_t>(exp + detail::bias)};
    if (biased_exp > std::numeric_limits<std::uint8_t>::max())
    {
        significand_ = detail::d32_fast_inf;
    }
    else
    {
        exponent_ = static_cast<std::uint_fast8_t>(biased_exp);
    }
}

template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool>>
constexpr decimal32_fast::decimal32_fast(Integer val) noexcept
{
    using ConversionType = std::conditional_t<std::is_same<Integer, bool>::value, std::int32_t, Integer>;
    *this = decimal32_fast{static_cast<ConversionType>(val), 0, false};
}

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
    if (val != val)
    {
        significand_ = detail::d32_fast_qnan;
    }
    else if (val == std::numeric_limits<Float>::infinity() || val == -std::numeric_limits<Float>::infinity())
    {
        significand_ = detail::d32_fast_inf;
    }
    else
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
    return val.significand_ == detail::d32_fast_inf;
}

constexpr auto isnan(decimal32_fast val) noexcept -> bool
{
    return val.significand_ == detail::d32_fast_qnan || val.significand_ == detail::d32_fast_snan;
}

constexpr auto issignaling(decimal32_fast val) noexcept -> bool
{
    return val.significand_ == detail::d32_fast_snan;
}

constexpr auto isnormal(decimal32_fast val) noexcept -> bool
{
    if (val.exponent_ <= static_cast<std::uint8_t>(detail::precision_v<decimal32> - 1))
    {
        return false;
    }

    return (val.significand_ != 0) && isfinite(val);
}

constexpr auto operator==(decimal32_fast lhs, decimal32_fast rhs) noexcept -> bool
{
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }

    return equal_parts_impl(lhs.full_significand(), lhs.biased_exponent(), lhs.isneg(),
                            rhs.full_significand(), rhs.biased_exponent(), rhs.isneg());
}

constexpr auto operator!=(decimal32_fast lhs, decimal32_fast rhs) noexcept -> bool
{
    return !(lhs == rhs);
}

constexpr auto operator<(decimal32_fast lhs, decimal32_fast rhs) noexcept -> bool
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

    return less_parts_impl(lhs.full_significand(), lhs.biased_exponent(), lhs.isneg(),
                           rhs.full_significand(), rhs.biased_exponent(), rhs.isneg());
}

constexpr auto operator<=(decimal32_fast lhs, decimal32_fast rhs) noexcept -> bool
{
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }

    return !(rhs < lhs);
}

constexpr auto operator>(decimal32_fast lhs, decimal32_fast rhs) noexcept -> bool
{
    return rhs < lhs;
}

constexpr auto operator>=(decimal32_fast lhs, decimal32_fast rhs) noexcept -> bool
{
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }

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
    return isnan(rhs) ? false : !less_impl(rhs, lhs) && lhs != rhs;
}

template <typename Integer>
constexpr auto operator<=(decimal32_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return isnan(lhs) ? false : !(rhs < lhs);
}

template <typename Integer>
constexpr auto operator<=(Integer lhs, decimal32_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return isnan(rhs) ? false : !(rhs < lhs);
}

template <typename Integer>
constexpr auto operator>(decimal32_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return isnan(lhs) ? false : rhs < lhs;
}

template <typename Integer>
constexpr auto operator>(Integer lhs, decimal32_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return isnan(rhs) ? false : rhs < lhs;
}

template <typename Integer>
constexpr auto operator>=(decimal32_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return isnan(lhs) ? false : !(lhs < rhs);
}

template <typename Integer>
constexpr auto operator>=(Integer lhs, decimal32_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, bool)
{
    return isnan(rhs) ? false : !(lhs < rhs);
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
    constexpr decimal32_fast zero {0, 0};

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

    const auto result {detail::add_impl<detail::decimal32_fast_components>(sig_lhs, exp_lhs, lhs.isneg(), sig_rhs, exp_rhs, rhs.isneg())};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator+(decimal32_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast)
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

    auto lhs_components {detail::decimal32_fast_components{sig_lhs, exp_lhs, lhs.isneg()}};
    auto sig_rhs {rhs};
    std::int32_t exp_rhs {0};
    detail::normalize(sig_rhs, exp_rhs);
    auto unsigned_sig_rhs = detail::shrink_significand<std::uint_fast32_t>(detail::make_positive_unsigned(sig_rhs), exp_rhs);
    auto rhs_components {detail::decimal32_fast_components{unsigned_sig_rhs, exp_rhs, (rhs < 0)}};

    if (!lhs_bigger)
    {
        detail::swap(lhs_components, rhs_components);
        lhs_bigger = !lhs_bigger;
        abs_lhs_bigger = !abs_lhs_bigger;
    }

    detail::decimal32_fast_components result {};

    if (!lhs_components.sign && rhs_components.sign)
    {
        result = detail::sub_impl<detail::decimal32_fast_components>(lhs_components.sig, lhs_components.exp, lhs_components.sign,
                                                                     rhs_components.sig, rhs_components.exp, rhs_components.sign,
                                                                     abs_lhs_bigger);
    }
    else
    {
        result = detail::add_impl<detail::decimal32_fast_components>(lhs_components.sig, lhs_components.exp, lhs_components.sign,
                                                                     rhs_components.sig, rhs_components.exp, rhs_components.sign);
    }

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator+(Integer lhs, decimal32_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast)
{
    return rhs + lhs;
}

constexpr auto operator-(decimal32_fast lhs, decimal32_fast rhs) noexcept -> decimal32_fast
{
    constexpr decimal32_fast zero {0, 0};

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

    const auto result {detail::sub_impl<detail::decimal32_fast_components>(sig_lhs, exp_lhs, lhs.isneg(),
                                                                                                         sig_rhs, exp_rhs, rhs.isneg(),
                                                                                                         abs_lhs_bigger)};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator-(decimal32_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast)
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
    auto lhs_components {detail::decimal32_fast_components{sig_lhs, exp_lhs, lhs.isneg()}};

    auto sig_rhs {rhs};
    std::int32_t exp_rhs {0};
    detail::normalize(sig_rhs, exp_rhs);
    auto unsigned_sig_rhs = detail::shrink_significand<std::uint_fast32_t>(detail::make_positive_unsigned(sig_rhs), exp_rhs);
    auto rhs_components {detail::decimal32_fast_components{unsigned_sig_rhs, exp_rhs, (rhs < 0)}};

    const auto result {detail::sub_impl<detail::decimal32_fast_components>(lhs_components.sig, lhs_components.exp, lhs_components.sign,
                                                                      rhs_components.sig, rhs_components.exp, rhs_components.sign,
                                                                      abs_lhs_bigger)};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator-(Integer lhs, decimal32_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast)
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
    auto unsigned_sig_lhs = detail::shrink_significand<std::uint_fast32_t>(detail::make_positive_unsigned(sig_lhs), exp_lhs);
    auto lhs_components {detail::decimal32_fast_components{unsigned_sig_lhs, exp_lhs, (lhs < 0)}};

    auto sig_rhs {rhs.full_significand()};
    auto exp_rhs {rhs.biased_exponent()};
    detail::normalize(sig_rhs, exp_rhs);
    auto rhs_components {detail::decimal32_fast_components{sig_rhs, exp_rhs, rhs.isneg()}};

    const auto result {detail::sub_impl<detail::decimal32_fast_components>(lhs_components.sig, lhs_components.exp, lhs_components.sign,
                                                                      rhs_components.sig, rhs_components.exp, rhs_components.sign,
                                                                      abs_lhs_bigger)};

    return {result.sig, result.exp, result.sign};
}

constexpr auto operator*(decimal32_fast lhs, decimal32_fast rhs) noexcept -> decimal32_fast
{
    constexpr decimal32_fast zero {0, 0};

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

    const auto result {detail::mul_impl<detail::decimal32_fast_components>(sig_lhs, exp_lhs, lhs.isneg(), sig_rhs, exp_rhs, rhs.isneg())};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator*(decimal32_fast lhs, Integer rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast)
{
    if (isnan(lhs) || isinf(lhs))
    {
        return lhs;
    }

    auto sig_lhs {lhs.full_significand()};
    auto exp_lhs {lhs.biased_exponent()};
    detail::normalize(sig_lhs, exp_lhs);
    auto lhs_components {detail::decimal32_fast_components{sig_lhs, exp_lhs, lhs.isneg()}};

    auto sig_rhs {rhs};
    std::int32_t exp_rhs {0};
    detail::normalize(sig_rhs, exp_rhs);
    auto unsigned_sig_rhs {detail::shrink_significand(detail::make_positive_unsigned(sig_rhs), exp_rhs)};
    auto rhs_components {detail::decimal32_fast_components{unsigned_sig_rhs, exp_rhs, (rhs < 0)}};

    const auto result {detail::mul_impl<detail::decimal32_fast_components>(
                                                        lhs_components.sig, lhs_components.exp, lhs_components.sign,
                                                        rhs_components.sig, rhs_components.exp, rhs_components.sign
                                                     )};

    return {result.sig, result.exp, result.sign};
}

template <typename Integer>
constexpr auto operator*(Integer lhs, decimal32_fast rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_integral_v, Integer, decimal32_fast)
{
    return rhs * lhs;
}

constexpr auto div_impl(decimal32_fast lhs, decimal32_fast rhs, decimal32_fast& q, decimal32_fast& r) noexcept -> void
{
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

    detail::decimal32_components lhs_components {static_cast<std::uint32_t>(sig_lhs), exp_lhs, lhs.isneg()};
    detail::decimal32_components rhs_components {static_cast<std::uint32_t>(sig_rhs), exp_rhs, rhs.isneg()};
    detail::decimal32_components q_components {};

    generic_div_impl(lhs_components, rhs_components, q_components);

    q = decimal32_fast(q_components.sig, q_components.exp, q_components.sign);
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

constexpr decimal32_fast::operator std::int8_t() const noexcept
{
    return to_integral<decimal32_fast, std::int8_t>(*this);
}

constexpr decimal32_fast::operator std::uint8_t() const noexcept
{
    return to_integral<decimal32_fast, std::uint8_t>(*this);
}

constexpr decimal32_fast::operator std::int16_t() const noexcept
{
    return to_integral<decimal32_fast, std::int16_t>(*this);
}

constexpr decimal32_fast::operator std::uint16_t() const noexcept
{
    return to_integral<decimal32_fast, std::uint16_t>(*this);
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr decimal32_fast::operator detail::int128_t() const noexcept
{
    return to_integral<decimal32_fast, detail::int128_t>(*this);
}

constexpr decimal32_fast::operator detail::uint128_t() const noexcept
{
    return to_integral<decimal32_fast, detail::uint128_t>(*this);
}

#endif

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, std::enable_if_t<detail::is_decimal_floating_point_v<Decimal>, bool>>
constexpr decimal32_fast::operator Decimal() const noexcept
{
    return to_decimal<Decimal>(*this);
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
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool traps = numeric_limits<std::uint_fast32_t>::traps;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool tinyness_before = true;

    // Member functions
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto (min)        () -> boost::decimal::decimal32_fast { return {1, min_exponent}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto (max)        () -> boost::decimal::decimal32_fast { return {9'999'999, max_exponent}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto lowest       () -> boost::decimal::decimal32_fast { return {-9'999'999, max_exponent}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto epsilon      () -> boost::decimal::decimal32_fast { return {1, -7}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto round_error  () -> boost::decimal::decimal32_fast { return epsilon(); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto infinity     () -> boost::decimal::decimal32_fast { return boost::decimal::direct_init(boost::decimal::detail::d32_fast_inf, UINT8_C((0))); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto quiet_NaN    () -> boost::decimal::decimal32_fast { return boost::decimal::direct_init(boost::decimal::detail::d32_fast_qnan, UINT8_C((0))); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto signaling_NaN() -> boost::decimal::decimal32_fast { return boost::decimal::direct_init(boost::decimal::detail::d32_fast_snan, UINT8_C((0))); }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto denorm_min   () -> boost::decimal::decimal32_fast { return {1, boost::decimal::detail::etiny}; }
};

} // Namespace std

#endif //BOOST_DECIMAL_DECIMAL32_FAST_HPP
