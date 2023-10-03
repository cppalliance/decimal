// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_COMPARISON_HPP
#define BOOST_DECIMAL_DETAIL_COMPARISON_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/normalize.hpp>
#include <cstdint>

namespace boost {
namespace decimal {

template <typename T1, typename T2>
constexpr auto equal_parts_impl(T1 lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                                T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> bool
{
    detail::normalize(lhs_sig, lhs_exp);
    detail::normalize(rhs_sig, rhs_exp);

    return lhs_sign == rhs_sign &&
           lhs_exp == rhs_exp &&
           lhs_sig == rhs_sig;
}

template <typename Decimal, typename Integer>
constexpr auto mixed_equality_impl(Decimal lhs, Integer rhs) noexcept
    -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>
{
    if (isnan(lhs) || isinf(lhs))
    {
        return false;
    }

    bool rhs_isneg {false};
    BOOST_DECIMAL_IF_CONSTEXPR (detail::is_signed_v<Integer>)
    {
        if (rhs < 0)
        {
            rhs_isneg = true;
        }
    }

    const auto rhs_significand {detail::make_positive_unsigned(rhs)};

    return equal_parts_impl(lhs.full_significand(), lhs.biased_exponent(), lhs.isneg(),
                            rhs_significand, INT32_C(0), rhs_isneg);
}

template <typename T1, typename T2>
constexpr auto less_parts_impl(T1 lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                               T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> bool
{
    const bool both_neg {lhs_sign && rhs_sign};

    // Normalize the significands and exponents
    detail::normalize(lhs_sig, lhs_exp);
    detail::normalize(rhs_sig, rhs_exp);

    if (lhs_sig == 0 && rhs_sig != 0)
    {
        return (!rhs_sign);
    }
    else if (lhs_sig != 0 && rhs_sig == 0)
    {
        return lhs_sign;
    }
    else if (lhs_sig == 0 && rhs_sig == 0)
    {
        return false;
    }
    else if (both_neg)
    {
        if (lhs_exp > rhs_exp)
        {
            return true;
        }
        else if (lhs_exp < rhs_exp)
        {
            return false;
        }
        else
        {
            return (lhs_sig > rhs_sig);
        }
    }
    else
    {
        if ((lhs_exp < rhs_exp) && (lhs_sig != static_cast<T1>(0)))
        {
            return true;
        }
        else if (lhs_exp > rhs_exp)
        {
            return false;
        }
        else
        {
            return (lhs_sig < rhs_sig);
        }
    }
}

template <typename Decimal, typename Integer>
constexpr auto less_impl(Decimal lhs, Integer rhs) noexcept
    -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal> && detail::is_integral_v<Integer>), bool>
{
    if (isnan(lhs))
    {
        return false;
    }
    else if (isinf(lhs))
    {
        return lhs.isneg();
    }

    bool lhs_sign {lhs.isneg()};
    bool rhs_sign {false};

    BOOST_DECIMAL_IF_CONSTEXPR (detail::is_signed_v<Integer>)
    {
        if (rhs < 0)
        {
            rhs_sign = true;
        }

        if (lhs_sign && !rhs_sign)
        {
            return true;
        }
        else if (!lhs_sign && rhs_sign)
        {
            return false;
        }
    }
    else
    {
        if (lhs_sign)
        {
            return true;
        }
    }

    const auto rhs_significand {detail::make_positive_unsigned(rhs)};

    return less_parts_impl(lhs.full_significand(), lhs.biased_exponent(), lhs_sign,
                           rhs_significand, INT32_C(0), rhs_sign);
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_COMPARISON_HPP
