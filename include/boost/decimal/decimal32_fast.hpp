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
#include <boost/decimal/detail/ryu/ryu_generic_128.hpp>
#include <limits>
#include <cstdint>

namespace boost {
namespace decimal {

class decimal32_fast final
{
private:
    // In regular decimal32 we have to decode the 24 bits of the significand and the 8 bits of the exp
    // Here we just use them directly at the cost of 8 extra bits of internal state
    //
    //  Inf: significand == std::numeric_limits<std::int32_t>::max()
    // qNaN: significand == std::numeric_limits<std::int32_t>::min()
    // sNaN: significand == std::numeric_limits<std::int32_t>::min() + 1

    std::int32_t significand_;
    std::uint8_t exponent_;

    constexpr auto isneg() noexcept -> bool
    {
        return significand_ < 0;
    }

    constexpr auto full_significand() const noexcept -> std::uint32_t
    {
        return detail::make_positive_unsigned(significand_);
    }

    constexpr auto unbiased_exponent() const noexcept -> std::uint8_t
    {
        return exponent_;
    }

    constexpr auto biased_exponent() const noexcept -> std::int32_t
    {
        return static_cast<std::int32_t>(exponent_) - detail::bias_v<decimal32>;
    }

public:
    constexpr decimal32_fast() noexcept : significand_{}, exponent_{} {}

    template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1> && detail::is_integral_v<T2>, bool> = true>
    constexpr decimal32_fast(T1 coeff, T2 exp, bool sign = false) noexcept;

    template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool> = true>
    constexpr decimal32_fast(Integer coeff) noexcept;

    template <typename Float, std::enable_if_t<detail::is_floating_point_v<Float>, bool> = true>
    BOOST_DECIMAL_CXX20_CONSTEXPR decimal32_fast(Float val) noexcept;

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

    // Unary operators
    friend constexpr auto operator+(decimal32_fast rhs) noexcept -> decimal32_fast;
    friend constexpr auto operator-(decimal32_fast lhs) noexcept -> decimal32_fast;

    // Binary arithmetic
    friend constexpr auto operator+(decimal32_fast lhs, decimal32_fast rhs) noexcept -> decimal32_fast;
    friend constexpr auto operator-(decimal32_fast lhs, decimal32_fast rhs) noexcept -> decimal32_fast;
    friend constexpr auto operator*(decimal32_fast lhs, decimal32_fast rhs) noexcept -> decimal32_fast;

    // Dummy conversion
    explicit constexpr operator std::size_t() const noexcept
    {
        return static_cast<std::size_t>(1);
    }
};

template <typename T1, typename T2, std::enable_if_t<detail::is_integral_v<T1> && detail::is_integral_v<T2>, bool>>
constexpr decimal32_fast::decimal32_fast(T1 coeff, T2 exp, bool sign) noexcept
    : significand_ {}, exponent_ {}
{
    using Unsigned_Integer = detail::make_unsigned_t<T1>;

    const bool isneg {coeff < static_cast<T1>(0) || sign};
    Unsigned_Integer unsigned_coeff {detail::make_positive_unsigned(coeff)};

    auto unsigned_coeff_digits {detail::num_digits(unsigned_coeff)};
    const bool reduced {unsigned_coeff_digits > detail::precision_v<decimal32>};
    while (unsigned_coeff_digits > detail::precision_v<decimal32> + 1)
    {
        unsigned_coeff /= 10;
        ++exp;
        --unsigned_coeff_digits;
    }

    // Round as required
    if (reduced)
    {
        exp += static_cast<T2>(detail::fenv_round(unsigned_coeff, isneg));
    }

    auto reduced_coeff {static_cast<std::uint32_t>(unsigned_coeff)};
    significand_ = static_cast<std::int32_t>(reduced_coeff);
    significand_ = isneg ? -significand_ : significand_;

    auto biased_exp {static_cast<std::uint32_t>(exp + detail::bias)};
    if (biased_exp > std::numeric_limits<std::uint8_t>::max())
    {
        significand_ = std::numeric_limits<std::int32_t>::max();
    }
    else
    {
        exponent_ = static_cast<std::uint8_t>(biased_exp);
    }
}

template <typename Integer, std::enable_if_t<detail::is_integral_v<Integer>, bool>>
constexpr decimal32_fast::decimal32_fast(Integer val) noexcept
{
    using ConversionType = std::conditional_t<std::is_same<Integer, bool>::value, std::int32_t, Integer>;
    *this = decimal32_fast{static_cast<ConversionType>(val), 0};
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
        significand_ = std::numeric_limits<std::int32_t>::min();
    }
    else if (val == std::numeric_limits<Float>::infinity() || val == -std::numeric_limits<Float>::infinity())
    {
        significand_ = std::numeric_limits<std::int32_t>::max();
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

constexpr auto signbit(decimal32_fast val) noexcept -> bool
{
    return val.significand_ < 0;
}

constexpr auto isinf(decimal32_fast val) noexcept -> bool
{
    return val.significand_ == std::numeric_limits<std::int32_t>::max();
}

constexpr auto isnan(decimal32_fast val) noexcept -> bool
{
    return val.significand_ == std::numeric_limits<std::int32_t>::min();
}

constexpr auto issignaling(decimal32_fast val) noexcept -> bool
{
    return val.significand_ == (std::numeric_limits<std::int32_t>::min() + 1);
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
                            rhs.full_significand(), lhs.biased_exponent(), lhs.isneg());
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

constexpr auto operator+(decimal32_fast rhs) noexcept -> decimal32_fast
{
    return rhs;
}

constexpr auto operator-(decimal32_fast rhs) noexcept -> decimal32_fast
{
    rhs.significand_ = -rhs.significand_;
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

    const auto result {add_impl(sig_lhs, exp_lhs, lhs.isneg(), sig_rhs, exp_rhs, rhs.isneg())};

    return {result.sig, result.exp, result.sign};
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

    const auto result {sub_impl(sig_lhs, exp_lhs, lhs.isneg(),
                                sig_rhs, exp_rhs, rhs.isneg(),
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

    const auto result {mul_impl(sig_lhs, exp_lhs, lhs.isneg(), sig_rhs, exp_rhs, rhs.isneg())};

    return {result.sig, result.exp, result.sign};
}

} // namespace decimal
} // namespace boost

#endif //BOOST_DECIMAL_DECIMAL32_FAST_HPP
