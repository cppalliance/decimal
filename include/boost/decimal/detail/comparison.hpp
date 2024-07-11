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
#include <boost/decimal/detail/power_tables.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <limits>
#include <cstdint>
#endif

namespace boost {
namespace decimal {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType = decimal32, BOOST_DECIMAL_INTEGRAL T1,
          BOOST_DECIMAL_INTEGRAL U1, BOOST_DECIMAL_INTEGRAL T2, BOOST_DECIMAL_INTEGRAL U2>
constexpr auto equal_parts_impl(T1 lhs_sig, U1 lhs_exp, bool lhs_sign,
                                T2 rhs_sig, U2 rhs_exp, bool rhs_sign) noexcept -> std::enable_if_t<std::is_same<DecimalType, decimal32>::value || std::is_same<DecimalType, decimal64>::value || std::is_same<DecimalType, decimal128>::value, bool>
{
    using comp_type = typename DecimalType::significand_type;

    BOOST_DECIMAL_ASSERT(lhs_sig >= 0);
    BOOST_DECIMAL_ASSERT(rhs_sig >= 0);

    // We con compare signs right away
    if (lhs_sign != rhs_sign)
    {
        return false;
    }

    auto new_lhs_sig {static_cast<comp_type>(lhs_sig)};
    auto new_rhs_sig {static_cast<comp_type>(rhs_sig)};

    const auto delta_exp {lhs_exp - rhs_exp};

    // Check the value of delta exp to avoid to large a value for pow10
    // Also if only one of the significands is 0 then we know the values have to be mismatched
    if (delta_exp > detail::precision_v<DecimalType> || delta_exp < -detail::precision_v<DecimalType> ||
        ((new_lhs_sig == static_cast<comp_type>(0)) ^ (new_rhs_sig == static_cast<comp_type>(0))))
    {
        return false;
    }

    if (delta_exp >= 0)
    {
        new_lhs_sig *= detail::pow10(static_cast<comp_type>(delta_exp));
    }
    else
    {
        new_rhs_sig *= detail::pow10(static_cast<comp_type>(-delta_exp));
    }

    #ifdef BOOST_DECIMAL_DEBUG_EQUAL
    std::cerr << "Normalized Values"
              << "\nlhs_sig: " << new_lhs_sig
              << "\nlhs_exp: " << lhs_exp
              << "\nrhs_sig: " << new_rhs_sig
              << "\nrhs_exp: " << rhs_exp << std::endl;
    #endif

    return new_lhs_sig == new_rhs_sig;
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType = decimal32, BOOST_DECIMAL_INTEGRAL T1,
          BOOST_DECIMAL_INTEGRAL U1, BOOST_DECIMAL_INTEGRAL T2, BOOST_DECIMAL_INTEGRAL U2>
constexpr auto equal_parts_impl(T1 lhs_sig, U1 lhs_exp, bool lhs_sign,
                                T2 rhs_sig, U2 rhs_exp, bool rhs_sign) noexcept -> std::enable_if_t<!(std::is_same<DecimalType, decimal32>::value || std::is_same<DecimalType, decimal64>::value || std::is_same<DecimalType, decimal128>::value), bool>
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
    using exp_type = typename Decimal::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs) || isinf(lhs))
    {
        return false;
    }
    #endif

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
                                     rhs_significand, static_cast<exp_type>(0), rhs_isneg);
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
constexpr auto mixed_decimal_equality_impl(Decimal1 lhs, Decimal2 rhs) noexcept
    -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                         detail::is_decimal_floating_point_v<Decimal2>), bool>
{
    static_assert(!std::is_same<Decimal1, Decimal2>::value, "Equality of same type exists in simpler form");
    using Bigger_Decimal_Type = std::conditional_t<(sizeof(lhs) > sizeof(rhs)), Decimal1, Decimal2>;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }
    #endif

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

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType = decimal32, BOOST_DECIMAL_INTEGRAL T1,
        BOOST_DECIMAL_INTEGRAL U1, BOOST_DECIMAL_INTEGRAL T2, BOOST_DECIMAL_INTEGRAL U2>
constexpr auto less_parts_impl(T1 lhs_sig, U1 lhs_exp, bool lhs_sign,
                               T2 rhs_sig, U2 rhs_exp, bool rhs_sign) noexcept -> std::enable_if_t<std::is_same<DecimalType, decimal32>::value || std::is_same<DecimalType, decimal64>::value, bool>
{
    using comp_type = std::uint_fast64_t;

    BOOST_DECIMAL_ASSERT(lhs_sig >= 0);
    BOOST_DECIMAL_ASSERT(rhs_sig >= 0);

    if (lhs_sign != rhs_sign)
    {
        return lhs_sign;
    }

    auto new_lhs_sig {static_cast<comp_type>(lhs_sig)};
    auto new_rhs_sig {static_cast<comp_type>(rhs_sig)};

    if (new_lhs_sig == UINT64_C(0) || new_rhs_sig == UINT64_C(0))
    {
        return (new_lhs_sig == new_rhs_sig) ? false : (new_lhs_sig == 0 ? !rhs_sign : lhs_sign);
    }

    detail::normalize<DecimalType>(new_lhs_sig, lhs_exp);
    detail::normalize<DecimalType>(new_rhs_sig, rhs_exp);

    if (lhs_exp != rhs_exp)
    {
        return lhs_sign ? lhs_exp > rhs_exp : lhs_exp < rhs_exp;
    }

    return lhs_sign ? new_lhs_sig > new_rhs_sig : new_lhs_sig < new_rhs_sig;
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType = decimal32, BOOST_DECIMAL_INTEGRAL T1,
          BOOST_DECIMAL_INTEGRAL U1, BOOST_DECIMAL_INTEGRAL T2, BOOST_DECIMAL_INTEGRAL U2>
constexpr auto less_parts_impl(T1 lhs_sig, U1 lhs_exp, bool lhs_sign,
                               T2 rhs_sig, U2 rhs_exp, bool rhs_sign) noexcept -> std::enable_if_t<!(std::is_same<DecimalType, decimal32>::value || std::is_same<DecimalType, decimal64>::value), bool>
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
    using exp_type = typename Decimal::biased_exponent_type;

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs))
    {
        return false;
    }
    else if (isinf(lhs))
    {
        return lhs.isneg();
    }
    #endif

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
                                    rhs_significand, static_cast<exp_type>(0), rhs_sign);
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Decimal2>
constexpr auto mixed_decimal_less_impl(Decimal1 lhs, Decimal2 rhs) noexcept
    -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                         detail::is_decimal_floating_point_v<Decimal2>), bool>
{
    using Bigger_Decimal_Type = std::conditional_t<(sizeof(lhs) > sizeof(rhs)), Decimal1, Decimal2>;


    if (
            #ifndef BOOST_DECIMAL_FAST_MATH
            isnan(lhs) || isnan(rhs) ||
            #endif
            (!lhs.isneg() && rhs.isneg())
        )
    {
        return false;
    }
    else if (lhs.isneg() && !rhs.isneg())
    {
        return true;
    }
    #ifndef BOOST_DECIMAL_FAST_MATH
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
    #endif

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
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }
    #endif

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
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }
    #endif

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
