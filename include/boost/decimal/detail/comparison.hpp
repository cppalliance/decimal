// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_COMPARISON_HPP
#define BOOST_DECIMAL_DETAIL_COMPARISON_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/normalize.hpp>
#include <boost/decimal/detail/promotion.hpp>
#include <boost/decimal/detail/to_decimal.hpp>
#include <boost/decimal/detail/shrink_significand.hpp>
#include <boost/decimal/detail/cmath/isfinite.hpp>
#include <boost/decimal/detail/concepts.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <limits>
#include <cstdint>
#endif

namespace boost {
namespace decimal {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType = decimal32, BOOST_DECIMAL_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
constexpr auto equal_parts_impl(T1 lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                                T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> bool
{
    using comp_type = std::conditional_t<(std::numeric_limits<T1>::digits10 > std::numeric_limits<T2>::digits10), T1, T2>;

    BOOST_DECIMAL_ASSERT(lhs_sig >= 0);
    BOOST_DECIMAL_ASSERT(rhs_sig >= 0);

    auto new_lhs_sig {static_cast<comp_type>(lhs_sig)};
    auto new_rhs_sig {static_cast<comp_type>(rhs_sig)};

    detail::normalize<DecimalType>(new_lhs_sig, lhs_exp);
    detail::normalize<DecimalType>(new_rhs_sig, rhs_exp);

    #ifdef BOOST_DECIMAL_DEBUG_EQUAL
    std::cerr << "Normalized Values"
              << "\nlhs_sig: " << new_lhs_sig
              << "\nlhs_exp: " << lhs_exp
              << "\nrhs_sig: " << new_rhs_sig
              << "\nrhs_exp: " << rhs_exp << std::endl;
    #endif

    return lhs_sign == rhs_sign &&
           lhs_exp == rhs_exp &&
           new_lhs_sig == new_rhs_sig;
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, BOOST_DECIMAL_INTEGRAL Integer>
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

    return equal_parts_impl<Decimal>(lhs.full_significand(), lhs.biased_exponent(), lhs.isneg(),
                                     rhs_significand, INT32_C(0), rhs_isneg);
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
constexpr auto mixed_decimal_equality_impl(Decimal1 lhs, Decimal2 rhs) noexcept
    -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                         detail::is_decimal_floating_point_v<Decimal2>), bool>
{
    static_assert(!std::is_same<Decimal1, Decimal2>::value, "Equality of same type exists in simpler form");
    using Bigger_Decimal_Type = std::conditional_t<(sizeof(lhs) > sizeof(rhs)), Decimal1, Decimal2>;

    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }

    const auto new_lhs = to_decimal<Bigger_Decimal_Type>(lhs);
    const auto new_rhs = to_decimal<Bigger_Decimal_Type>(rhs);

    return new_lhs == new_rhs;
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
constexpr auto operator==(Decimal1 lhs, Decimal2 rhs) noexcept
    -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                         detail::is_decimal_floating_point_v<Decimal2>), bool>
{
    return mixed_decimal_equality_impl(lhs, rhs);
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
constexpr auto operator!=(Decimal1 lhs, Decimal2 rhs) noexcept
    -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                         detail::is_decimal_floating_point_v<Decimal2>), bool>
{
    return !(mixed_decimal_equality_impl(lhs, rhs));
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType = decimal32, BOOST_DECIMAL_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
constexpr auto less_parts_impl(T1 lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                               T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> bool
{
    using comp_type = std::conditional_t<(std::numeric_limits<T1>::digits10 > std::numeric_limits<T2>::digits10), T1, T2>;

    BOOST_DECIMAL_ASSERT(lhs_sig >= 0);
    BOOST_DECIMAL_ASSERT(rhs_sig >= 0);

    auto new_lhs_sig {static_cast<comp_type>(lhs_sig)};
    auto new_rhs_sig {static_cast<comp_type>(rhs_sig)};

    detail::normalize<DecimalType>(new_lhs_sig, lhs_exp);
    detail::normalize<DecimalType>(new_rhs_sig, rhs_exp);

    if (new_lhs_sig == 0 || new_rhs_sig == 0)
    {
        if (new_lhs_sig == 0 && new_rhs_sig == 0)
        {
            return false;
        }
        return new_lhs_sig == 0 ? !rhs_sign : lhs_sign;
    }

    if (lhs_sign != rhs_sign)
    {
        return lhs_sign;
    }

    if (lhs_exp != rhs_exp)
    {
        return lhs_sign ? lhs_exp > rhs_exp : lhs_exp < rhs_exp;
    }

    return lhs_sign ? new_lhs_sig > new_rhs_sig : new_lhs_sig < new_rhs_sig;
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal, BOOST_DECIMAL_INTEGRAL Integer>
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

    return less_parts_impl<Decimal>(lhs.full_significand(), lhs.biased_exponent(), lhs_sign,
                                    rhs_significand, INT32_C(0), rhs_sign);
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
constexpr auto mixed_decimal_less_impl(Decimal1 lhs, Decimal2 rhs) noexcept
    -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                         detail::is_decimal_floating_point_v<Decimal2>), bool>
{
    using Bigger_Decimal_Type = std::conditional_t<(sizeof(lhs) > sizeof(rhs)), Decimal1, Decimal2>;

    if (isnan(lhs) || isnan(rhs) || (!lhs.isneg() && rhs.isneg()))
    {
        return false;
    }
    else if (lhs.isneg() && !rhs.isneg())
    {
        return true;
    }
    else if (boost::decimal::isfinite(lhs) && isinf(rhs))
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

    return less_parts_impl<Bigger_Decimal_Type>(lhs.full_significand(), lhs.biased_exponent(), lhs.isneg(),
                                                rhs.full_significand(), rhs.biased_exponent(), rhs.isneg());
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
constexpr auto operator<(Decimal1 lhs, Decimal2 rhs) noexcept
    -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                         detail::is_decimal_floating_point_v<Decimal2>), bool>
{
    return mixed_decimal_less_impl(lhs, rhs);
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
constexpr auto operator<=(Decimal1 lhs, Decimal2 rhs) noexcept
    -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                         detail::is_decimal_floating_point_v<Decimal2>), bool>
{
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }

    return !(rhs < lhs);
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
constexpr auto operator>(Decimal1 lhs, Decimal2 rhs) noexcept
    -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                         detail::is_decimal_floating_point_v<Decimal2>), bool>
{
    return rhs < lhs;
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
constexpr auto operator>=(Decimal1 lhs, Decimal2 rhs) noexcept
    -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                         detail::is_decimal_floating_point_v<Decimal2>), bool>
{
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }

    return !(lhs < rhs);
}

#ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
constexpr auto operator<=>(Decimal1 lhs, Decimal2 rhs) noexcept
    -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                         detail::is_decimal_floating_point_v<Decimal2>), std::partial_ordering>
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

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_COMPARISON_HPP
