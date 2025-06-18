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
#include <boost/int128.hpp>
#include <boost/decimal/detail/attributes.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <limits>
#include <cstdint>
#endif

namespace boost {
namespace decimal {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
BOOST_DECIMAL_FORCE_INLINE constexpr auto equality_impl(DecimalType lhs, DecimalType rhs) noexcept -> bool
{
    using comp_type = typename DecimalType::significand_type;

    // Step 1: Check for NANs per IEEE 754
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }
    #endif

    // Step 2: Fast path
    if (lhs.bits_ == rhs.bits_)
    {
        return true;
    }

    // Step 3: Check -0 == +0
    auto lhs_sig {lhs.full_significand()};
    auto rhs_sig {rhs.full_significand()};
    if (lhs_sig == 0U && rhs_sig == 0U)
    {
        return true;
    }

    // Step 4: Check signs
    const auto lhs_neg {lhs.isneg()};
    const auto rhs_neg {rhs.isneg()};

    if (lhs_neg != rhs_neg)
    {
        return false;
    }

    // Step 5: Check the exponents
    // If the difference is greater than we can represent in the significand than we can assume they are different
    const auto lhs_exp {lhs.biased_exponent()};
    const auto rhs_exp {rhs.biased_exponent()};

    const auto delta_exp {lhs_exp - rhs_exp};

    if (delta_exp > detail::precision_v<DecimalType> || delta_exp < -detail::precision_v<DecimalType>)
    {
        return false;
    }

    // Step 6: Normalize the significand and compare
    // Instead of multiplying the larger number, divide the smaller one
    if (delta_exp >= 0)
    {
        // Check if we can divide rhs_sig safely
        if (delta_exp > 0 && rhs_sig % detail::pow10(static_cast<comp_type>(delta_exp)) != 0U)
        {
            return false;
        }
        rhs_sig /= detail::pow10(static_cast<comp_type>(delta_exp));
    }
    else
    {
        // Check if we can divide lhs_sig safely
        if (lhs_sig % detail::pow10(static_cast<comp_type>(-delta_exp)) != 0U)
        {
            return false;
        }
        lhs_sig /= detail::pow10(static_cast<comp_type>(-delta_exp));
    }

    return lhs_sig == rhs_sig;
}

template <BOOST_DECIMAL_FAST_DECIMAL_FLOATING_TYPE DecimalType>
BOOST_DECIMAL_FORCE_INLINE constexpr auto fast_equality_impl(const DecimalType& lhs, const DecimalType& rhs) noexcept -> bool
{
    if (lhs.exponent_ != rhs.exponent_)
    {
        return false;
    }
    if (lhs.significand_ != rhs.significand_)
    {
        return false;
    }

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(lhs))
    {
        return false;
    }
    #endif

    if (lhs.significand_ == 0U)
    {
        return true; // -0 == +0
    }

    return lhs.sign_ == rhs.sign_;
}

template <BOOST_DECIMAL_FAST_DECIMAL_FLOATING_TYPE DecimalType>
BOOST_DECIMAL_FORCE_INLINE constexpr auto fast_inequality_impl(const DecimalType& lhs, const DecimalType& rhs) noexcept -> bool
{
    return
            #ifndef BOOST_DECIMAL_FAST_MATH
            isnan(lhs) || isnan(rhs) ||
            #endif
            (lhs.sign_ != rhs.sign_) ||
            (lhs.exponent_ != rhs.exponent_) ||
            (lhs.significand_ != rhs.significand_);
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType = decimal32, BOOST_DECIMAL_INTEGRAL T1,
          BOOST_DECIMAL_INTEGRAL U1, BOOST_DECIMAL_INTEGRAL T2, BOOST_DECIMAL_INTEGRAL U2>
constexpr auto equal_parts_impl(T1 lhs_sig, U1 lhs_exp, bool lhs_sign,
                                T2 rhs_sig, U2 rhs_exp, bool rhs_sign) noexcept -> std::enable_if_t<detail::is_ieee_type_v<DecimalType>, bool>
{
    using comp_type = std::conditional_t<(std::numeric_limits<T1>::digits10 > std::numeric_limits<T2>::digits10), T1, T2>;

    BOOST_DECIMAL_ASSERT(lhs_sig >= 0U);
    BOOST_DECIMAL_ASSERT(rhs_sig >= 0U);

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
    if (new_lhs_sig == static_cast<comp_type>(0) && new_rhs_sig == static_cast<comp_type>(0))
    {
        return true;
    }
    if (delta_exp > detail::precision_v<DecimalType> || delta_exp < -detail::precision_v<DecimalType>)
    {
        return false;
    }

    // Step 5: Normalize the significand and compare
    // Instead of multiplying the larger number, divide the smaller one
    if (delta_exp >= 0)
    {
        // Check if we can divide rhs_sig safely
        if (delta_exp > 0 && new_rhs_sig % detail::pow10(static_cast<comp_type>(delta_exp)) != 0U)
        {
            return false;
        }
        new_rhs_sig /= detail::pow10(static_cast<comp_type>(delta_exp));
    }
    else
    {
        // Check if we can divide lhs_sig safely
        if (new_lhs_sig % detail::pow10(static_cast<comp_type>(-delta_exp)) != 0U)
        {
            return false;
        }
        new_lhs_sig /= detail::pow10(static_cast<comp_type>(-delta_exp));
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
                                T2 rhs_sig, U2 rhs_exp, bool rhs_sign) noexcept -> std::enable_if_t<detail::is_fast_type_v<DecimalType>, bool>
{
    using comp_type = std::conditional_t<(std::numeric_limits<T1>::digits10 > std::numeric_limits<T2>::digits10), T1, T2>;

    BOOST_DECIMAL_ASSERT(lhs_sig >= 0U);
    BOOST_DECIMAL_ASSERT(rhs_sig >= 0U);

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
        if (rhs < static_cast<Integer>(0))
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

template <BOOST_DECIMAL_FAST_DECIMAL_FLOATING_TYPE DecimalType>
BOOST_DECIMAL_FORCE_INLINE constexpr auto fast_less_impl(const DecimalType& lhs, const DecimalType& rhs) noexcept -> bool
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
    if (lhs.significand_ == 0U || rhs.significand_ == 0U)
    {
        if (lhs.significand_ == 0U && rhs.significand_ == 0U)
        {
            #ifndef BOOST_DECIMAL_FAST_MATH
            return lhs.sign_ && !rhs.sign_;
            #else
            return false;
            #endif
        }
        return lhs.significand_ == 0U ? !rhs.sign_ : lhs.sign_;
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

template <BOOST_DECIMAL_INTEGRAL T, BOOST_DECIMAL_INTEGRAL U>
BOOST_DECIMAL_FORCE_INLINE constexpr auto fast_type_less_parts_impl(T lhs_sig, U lhs_exp, bool lhs_sign,
                                                                    T rhs_sig, U rhs_exp, bool rhs_sign) noexcept -> bool
{
    if (lhs_sig == 0U || rhs_sig == 0U)
    {
        if (lhs_sig == 0U && rhs_sig == 0U)
        {
            #ifndef BOOST_DECIMAL_FAST_MATH
            return lhs_sign && !rhs_sign;
            #else
            return false;
            #endif
        }
        return lhs_sig == 0U ? !rhs_sign : lhs_sign;
    }

    if (lhs_sign != rhs_sign)
    {
        return lhs_sign;
    }

    if (lhs_exp != rhs_exp)
    {
        return lhs_sign ? lhs_exp > rhs_exp : lhs_exp < rhs_exp;
    }

    return lhs_sign ? lhs_sig > rhs_sig : lhs_sig < rhs_sig;
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
constexpr auto sequential_less_impl(DecimalType lhs, DecimalType rhs) noexcept -> bool
{
    using comp_type = std::conditional_t<detail::decimal_val_v<DecimalType> < 64, std::uint_fast64_t,
    // GCC less than 10 in non-GNU mode, Clang < 10 and MinGW all have issues with the built-in u128,
    // so use the emulated one
    #if defined(BOOST_DECIMAL_HAS_INT128)
    #if ( (defined(__GNUC__) && !defined(__clang__) && __GNUC__ < 10) || (defined(__clang__) && __clang_major__ < 13) )
        int128::uint128_t
    #  else
        detail::builtin_uint128_t
    #  endif
    #else
    int128::uint128_t
    #endif
    >;

    // Step 1: Handle our non-finite values in their own calling functions

    // Step 2: Check if they are bitwise equal:
    /*
    if (lhs.bits_ == rhs.bits_)
    {
        return false;
    }
    */

    // Step 3: Decode and compare signs first:
    const auto lhs_sign {lhs.isneg()};
    const auto rhs_sign {rhs.isneg()};

    if (lhs_sign != rhs_sign)
    {
        return lhs_sign;
    }

    // Step 4: Decode the significand and do a trivial comp
    auto lhs_sig {static_cast<comp_type>(lhs.full_significand())};
    auto rhs_sig {static_cast<comp_type>(rhs.full_significand())};
    if (lhs_sig == static_cast<comp_type>(0) || rhs_sig == static_cast<comp_type>(0))
    {
        return (lhs_sig == rhs_sig) ? false : (lhs_sig == static_cast<comp_type>(0) ? !rhs_sign : lhs_sign);
    }

    // Step 5: Decode the exponent and see if we can even compare the significands
    auto lhs_exp {lhs.biased_exponent()};
    auto rhs_exp {rhs.biased_exponent()};

    const auto delta_exp {lhs_exp - rhs_exp};
    constexpr auto max_delta_diff {std::numeric_limits<comp_type>::digits10 - detail::precision_v<DecimalType>};

    if (delta_exp > max_delta_diff || delta_exp < -max_delta_diff)
    {
        return rhs_sign ? rhs_exp < lhs_exp : rhs_exp > lhs_exp;
    }

    // Step 6: Approximate normalization if we need to and then get the answer
    if (delta_exp >= 0)
    {
        lhs_sig *= detail::pow10(static_cast<comp_type>(delta_exp));
        lhs_exp -= delta_exp;
    }
    else
    {
        rhs_sig *= detail::pow10(static_cast<comp_type>(-delta_exp));
        rhs_exp += delta_exp;
    }

    if (lhs_exp != rhs_exp)
    {
        return lhs_sign ? lhs_exp > rhs_exp : lhs_exp < rhs_exp;
    }

    return lhs_sign ? lhs_sig > rhs_sig : lhs_sig < rhs_sig;
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType = decimal32, BOOST_DECIMAL_INTEGRAL T1,
        BOOST_DECIMAL_INTEGRAL U1, BOOST_DECIMAL_INTEGRAL T2, BOOST_DECIMAL_INTEGRAL U2>
constexpr auto less_parts_impl(T1 lhs_sig, U1 lhs_exp, bool lhs_sign,
                               T2 rhs_sig, U2 rhs_exp, bool rhs_sign, bool normalized = false) noexcept -> std::enable_if_t<detail::decimal_val_v<DecimalType> == 32, bool>
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
        return (new_lhs_sig == new_rhs_sig) ? false : (new_lhs_sig == 0U ? !rhs_sign : lhs_sign);
    }

    const auto delta_exp {lhs_exp - rhs_exp};
    constexpr auto max_delta_diff {std::numeric_limits<std::uint_fast64_t>::digits10 - detail::precision_v<DecimalType>};

    // If we can't do this correctly without normalization then do it and try again
    if (delta_exp > max_delta_diff || delta_exp < -max_delta_diff)
    {
        if (!normalized)
        {
            detail::normalize(lhs_sig, lhs_exp);
            detail::normalize(rhs_sig, rhs_exp);
            return less_parts_impl(lhs_sig, lhs_exp, lhs_sign,
                                   rhs_sig, rhs_exp, rhs_sign, true);
        }
        else
        {
            return rhs_sign ? rhs_exp < lhs_exp : rhs_exp > lhs_exp;
        }
    }

    if (delta_exp >= 0)
    {
        new_lhs_sig *= detail::pow10(static_cast<comp_type>(delta_exp));
        lhs_exp -= delta_exp;
    }
    else
    {
        new_rhs_sig *= detail::pow10(static_cast<comp_type>(-delta_exp));
        rhs_exp += delta_exp;
    }

    if (lhs_exp != rhs_exp)
    {
        return lhs_sign ? lhs_exp > rhs_exp : lhs_exp < rhs_exp;
    }

    return lhs_sign ? new_lhs_sig > new_rhs_sig : new_lhs_sig < new_rhs_sig;
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType = decimal32, BOOST_DECIMAL_INTEGRAL T1,
        BOOST_DECIMAL_INTEGRAL U1, BOOST_DECIMAL_INTEGRAL T2, BOOST_DECIMAL_INTEGRAL U2>
constexpr auto less_parts_impl(T1 lhs_sig, U1 lhs_exp, bool lhs_sign,
                               T2 rhs_sig, U2 rhs_exp, bool rhs_sign) noexcept -> std::enable_if_t<detail::decimal_val_v<DecimalType> == 64 || detail::decimal_val_v<DecimalType> == 128, bool>
{
    using comp_type = typename DecimalType::significand_type;

    BOOST_DECIMAL_ASSERT(lhs_sig >= 0U);
    BOOST_DECIMAL_ASSERT(rhs_sig >= 0U);

    if (lhs_sign != rhs_sign)
    {
        return lhs_sign;
    }

    auto new_lhs_sig {static_cast<comp_type>(lhs_sig)};
    auto new_rhs_sig {static_cast<comp_type>(rhs_sig)};

    if (new_lhs_sig == UINT64_C(0) || new_rhs_sig == UINT64_C(0))
    {
        return (new_lhs_sig == new_rhs_sig) ? false : (new_lhs_sig == 0U ? !rhs_sign : lhs_sign);
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
                               T2 rhs_sig, U2 rhs_exp, bool rhs_sign) noexcept -> std::enable_if_t<detail::is_fast_type_v<DecimalType>, bool>
{
    using comp_type = std::conditional_t<(std::numeric_limits<T1>::digits10 > std::numeric_limits<T2>::digits10), T1, T2>;

    BOOST_DECIMAL_ASSERT(lhs_sig >= 0U);
    BOOST_DECIMAL_ASSERT(rhs_sig >= 0U);

    auto new_lhs_sig {static_cast<comp_type>(lhs_sig)};
    auto new_rhs_sig {static_cast<comp_type>(rhs_sig)};

    detail::normalize<DecimalType>(new_lhs_sig, lhs_exp);
    detail::normalize<DecimalType>(new_rhs_sig, rhs_exp);

    if (new_lhs_sig == 0U || new_rhs_sig == 0U)
    {
        if (new_lhs_sig == 0U && new_rhs_sig == 0U)
        {
            return false;
        }
        return new_lhs_sig == 0U ? !rhs_sign : lhs_sign;
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
        if (rhs < 0U)
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
