// Copyright 2023 - 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_ADD_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_ADD_IMPL_HPP

#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/fenv_rounding.hpp>
#include <boost/decimal/detail/components.hpp>
#include <boost/decimal/detail/power_tables.hpp>
#include "int128.hpp"

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <cstdint>
#endif

namespace boost {
namespace decimal {
namespace detail {

template <typename ReturnType, typename T>
constexpr auto d32_add_impl(const T& lhs, const T& rhs) noexcept -> ReturnType
{
    // Each of the significands is maximally 23 bits.
    // Rather than doing division to get proper alignment we will promote to 64 bits
    // And do a single mul followed by an add
    using add_type = std::int_fast64_t;
    using promoted_sig_type = std::uint_fast64_t;

    promoted_sig_type big_lhs {lhs.full_significand()};
    promoted_sig_type big_rhs {rhs.full_significand()};
    auto lhs_exp {lhs.biased_exponent()};
    const auto rhs_exp {rhs.biased_exponent()};

    // Align to larger exponent
    if (lhs_exp != rhs_exp)
    {
        constexpr auto max_shift {detail::make_positive_unsigned(detail::precision_v<decimal32_t> + 1)};
        const auto shift {detail::make_positive_unsigned(lhs_exp - rhs_exp)};

        if (shift > max_shift)
        {
            return big_lhs != 0U && (lhs_exp > rhs_exp) ?
                ReturnType{lhs.full_significand(), lhs.biased_exponent(), lhs.isneg()} :
                ReturnType{rhs.full_significand(), rhs.biased_exponent(), rhs.isneg()};
        }
        else if (lhs_exp < rhs_exp)
        {
            big_rhs *= detail::pow10<promoted_sig_type>(shift);
            lhs_exp = rhs_exp - static_cast<decimal32_t_components::biased_exponent_type>(shift);
        }
        else
        {
            big_lhs *= detail::pow10<promoted_sig_type>(shift);
            lhs_exp -= static_cast<decimal32_t_components::biased_exponent_type>(shift);
        }
    }

    // Perform signed addition with overflow protection
    const auto signed_lhs {detail::make_signed_value<add_type>(static_cast<add_type>(big_lhs), lhs.isneg())};
    const auto signed_rhs {detail::make_signed_value<add_type>(static_cast<add_type>(big_rhs), rhs.isneg())};

    const auto new_sig {signed_lhs + signed_rhs};

    return ReturnType{new_sig, lhs_exp};
}

template <typename ReturnType, typename T>
constexpr auto d32_fast_add_only_impl(const T& lhs, const T& rhs) noexcept -> ReturnType
{
    // Each of the significands is maximally 23 bits.
    // Rather than doing division to get proper alignment we will promote to 64 bits
    // And do a single mul followed by an add
    using promoted_sig_type = std::uint_fast64_t;

    int max_result_digits_overage {1};

    promoted_sig_type big_lhs {lhs.full_significand()};
    promoted_sig_type big_rhs {rhs.full_significand()};
    auto lhs_exp {lhs.biased_exponent()};
    const auto rhs_exp {rhs.biased_exponent()};

    // Align to larger exponent
    if (lhs_exp != rhs_exp)
    {
        constexpr auto max_shift {detail::make_positive_unsigned(detail::precision_v<decimal32_t> + 1)};
        const auto shift {detail::make_positive_unsigned(lhs_exp - rhs_exp)};

        if (shift > max_shift)
        {
            return big_lhs != 0U && (lhs_exp > rhs_exp) ?
                ReturnType{lhs.full_significand(), lhs.biased_exponent(), lhs.isneg()} :
                ReturnType{rhs.full_significand(), rhs.biased_exponent(), rhs.isneg()};
        }

        if (lhs_exp < rhs_exp)
        {
            big_rhs *= detail::pow10<promoted_sig_type>(shift);
            lhs_exp = rhs_exp - static_cast<decimal32_t_components::biased_exponent_type>(shift);
        }
        else
        {
            big_lhs *= detail::pow10<promoted_sig_type>(shift);
            lhs_exp -= static_cast<decimal32_t_components::biased_exponent_type>(shift);
        }

        max_result_digits_overage = static_cast<int>(shift);
    }

    auto res_sig {big_lhs + big_rhs};

    constexpr promoted_sig_type max_non_normalized_value {9'999'999U};
    if (res_sig > max_non_normalized_value)
    {
        constexpr promoted_sig_type max_non_compensated_value {99'999'999U};
        if (res_sig > max_non_compensated_value)
        {
            const auto offset_power {max_result_digits_overage == 1 ? 1 : max_result_digits_overage - 1};
            const auto offset {detail::pow10(static_cast<promoted_sig_type>(offset_power))};
            res_sig /= offset;
            lhs_exp += offset_power;
        }

        lhs_exp += detail::fenv_round(res_sig, false);
    }

    BOOST_DECIMAL_ASSERT(res_sig >= 1'000'000U || res_sig == 0U);
    BOOST_DECIMAL_ASSERT(res_sig <= max_non_normalized_value || res_sig == 0U);

    return ReturnType{static_cast<typename ReturnType::significand_type>(res_sig), lhs_exp, false};
}

template <typename ReturnType, typename T, typename U>
constexpr auto d32_add_impl(T lhs_sig, U lhs_exp, bool lhs_sign,
                            T rhs_sig, U rhs_exp, bool rhs_sign) noexcept -> ReturnType
{
    // Each of the significands is maximally 23 bits.
    // Rather than doing division to get proper alignment we will promote to 64 bits
    // And do a single mul followed by an add
    using add_type = std::int_fast64_t;
    using promoted_sig_type = std::uint_fast64_t;

    promoted_sig_type big_lhs {lhs_sig};
    promoted_sig_type big_rhs {rhs_sig};

    // Align to larger exponent
    if (lhs_exp != rhs_exp)
    {
        constexpr auto max_shift {detail::make_positive_unsigned(detail::precision_v<decimal32_t> + 1)};
        const auto shift {detail::make_positive_unsigned(lhs_exp - rhs_exp)};

        if (shift > max_shift)
        {
            return lhs_sig != 0U && (lhs_exp > rhs_exp) ? ReturnType{lhs_sig, lhs_exp, lhs_sign} : ReturnType{rhs_sig, rhs_exp, rhs_sign};
        }

        if (lhs_exp < rhs_exp)
        {
            big_rhs *= detail::pow10<promoted_sig_type>(shift);
            lhs_exp = rhs_exp - static_cast<U>(shift);
        }
        else
        {
            big_lhs *= detail::pow10<promoted_sig_type>(shift);
            lhs_exp -= static_cast<U>(shift);
        }
    }

    // Perform signed addition with overflow protection
    const auto signed_lhs {detail::make_signed_value<add_type>(static_cast<add_type>(big_lhs), lhs_sign)};
    const auto signed_rhs {detail::make_signed_value<add_type>(static_cast<add_type>(big_rhs), rhs_sign)};

    const auto new_sig {signed_lhs + signed_rhs};

    return {new_sig, lhs_exp};
}

template <typename ReturnType, typename T>
constexpr auto d64_add_impl(const T& lhs, const T& rhs) noexcept -> ReturnType
{
    // Each of the significands is maximally 23 bits.
    // Rather than doing division to get proper alignment we will promote to 64 bits
    // And do a single mul followed by an add
    using add_type = boost::int128::int128_t;
    using promoted_sig_type = boost::int128::uint128_t;

    promoted_sig_type big_lhs {lhs.full_significand()};
    promoted_sig_type big_rhs {rhs.full_significand()};
    auto lhs_exp {lhs.biased_exponent()};
    const auto rhs_exp {rhs.biased_exponent()};

    // Align to larger exponent
    if (lhs_exp != rhs_exp)
    {
        constexpr auto max_shift {detail::make_positive_unsigned(detail::precision_v<decimal64_t> + 1)};
        const auto shift {detail::make_positive_unsigned(lhs_exp - rhs_exp)};

        if (shift > max_shift)
        {
            return lhs.full_significand() != 0U && (lhs_exp > rhs_exp) ?
                ReturnType{lhs.full_significand(), lhs.biased_exponent(), lhs.isneg()} :
                ReturnType{rhs.full_significand(), rhs.biased_exponent(), rhs.isneg()};
        }

        if (lhs_exp < rhs_exp)
        {
            big_rhs *= detail::pow10<promoted_sig_type>(shift);
            lhs_exp = rhs_exp - static_cast<decimal64_t_components::biased_exponent_type>(shift);
        }
        else
        {
            big_lhs *= detail::pow10<promoted_sig_type>(shift);
            lhs_exp -= static_cast<decimal64_t_components::biased_exponent_type>(shift);
        }
    }

    // Perform signed addition with overflow protection
    const auto signed_lhs {detail::make_signed_value<add_type>(static_cast<add_type>(big_lhs), lhs.isneg())};
    const auto signed_rhs {detail::make_signed_value<add_type>(static_cast<add_type>(big_rhs), rhs.isneg())};

    const auto new_sig {signed_lhs + signed_rhs};

    return ReturnType{new_sig, lhs_exp};
}

template <typename ReturnType, typename T, typename U>
constexpr auto d64_add_impl(T lhs_sig, U lhs_exp, bool lhs_sign,
                            T rhs_sig, U rhs_exp, bool rhs_sign,
                            bool abs_lhs_bigger) noexcept -> ReturnType
{
    using add_type = std::int_fast64_t;

    auto delta_exp {lhs_exp > rhs_exp ? lhs_exp - rhs_exp : rhs_exp - lhs_exp};
    auto signed_sig_lhs {static_cast<add_type>(detail::make_signed_value(lhs_sig, lhs_sign))};
    auto signed_sig_rhs {static_cast<add_type>(detail::make_signed_value(rhs_sig, rhs_sign))};

    #ifdef BOOST_DECIMAL_DEBUG_ADD
    std::cerr << "Starting sig lhs: " << lhs_sig
              << "\nStarting exp lhs: " << lhs_exp
              << "\nStarting sig rhs: " << rhs_sig
              << "\nStarting exp rhs: " << rhs_exp << std::endl;
    #endif

    if (delta_exp > detail::precision_v<decimal64_t> + 1)
    {
        // If the difference in exponents is more than the digits of accuracy
        // we return the larger of the two
        //
        // e.g. 1e20 + 1e-20 = 1e20

        return abs_lhs_bigger ? ReturnType{lhs_sig, lhs_exp, lhs_sign} :
                                ReturnType{rhs_sig, rhs_exp, rhs_sign};
    }

    // The two numbers can be added together without special handling
    //
    // If we can add to the lhs sig rather than dividing we can save some precision
    // 32-bit signed int can have 9 digits and our normalized significand has 7

    auto& sig_bigger {abs_lhs_bigger ? signed_sig_lhs : signed_sig_rhs};
    auto& exp_bigger {abs_lhs_bigger ? lhs_exp : rhs_exp};
    auto& sig_smaller {abs_lhs_bigger ? signed_sig_rhs : signed_sig_lhs};
    auto& sign_smaller {abs_lhs_bigger ? rhs_sign : lhs_sign};

    if (delta_exp <= 2)
    {
        sig_bigger *= pow10(static_cast<add_type>(delta_exp));
        exp_bigger -= delta_exp;
        delta_exp = 0;
    }
    else
    {
        sig_bigger *= 100;
        delta_exp -= 2;
        exp_bigger -= 2;

        if (delta_exp > 1)
        {
            sig_smaller /= pow10(static_cast<add_type>(delta_exp - 1));
            delta_exp = 1;
        }

        if (delta_exp == 1)
        {
            detail::fenv_round<decimal64_t>(sig_smaller, sign_smaller);
        }
    }

    // Cast the results to signed types so that we can apply a sign at the end if necessary
    // Both of the significands are maximally 24 bits, so they fit into a 32-bit signed type just fine
    const auto new_sig {sig_bigger + sig_smaller};
    const auto new_exp {exp_bigger};
    const auto new_sign {new_sig < 0};
    const auto res_sig {detail::make_positive_unsigned(new_sig)};

    #ifdef BOOST_DECIMAL_DEBUG_ADD
    std::cerr << "Final sig lhs: " << lhs_sig
              << "\nFinal sig rhs: " << rhs_sig
              << "\nResult sig: " << new_sig << std::endl;
    #endif

    return {res_sig, new_exp, new_sign};
}

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4127) // If constexpr macro only works for C++17 and above
#endif

template <typename ReturnType, BOOST_DECIMAL_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL U1,
                               BOOST_DECIMAL_INTEGRAL T2, BOOST_DECIMAL_INTEGRAL U2>
constexpr auto d128_add_impl(T1 lhs_sig, U1 lhs_exp, bool lhs_sign,
                             T2 rhs_sig, U2 rhs_exp, bool rhs_sign) noexcept -> ReturnType
{
    const bool sign {lhs_sign};

    auto delta_exp {lhs_exp > rhs_exp ? lhs_exp - rhs_exp : rhs_exp - lhs_exp};

    if (delta_exp > detail::precision_v<decimal128_t> + 1)
    {
        // If the difference in exponents is more than the digits of accuracy
        // we return the larger of the two
        //
        // e.g. 1e20 + 1e-20 = 1e20

        return {lhs_sig, lhs_exp, lhs_sign};
    }

    if (delta_exp == detail::precision_v<decimal128_t> + 1)
    {
        // Only need to see if we need to add one to the
        // significand of the bigger value
        //
        // e.g. 1.234567e5 + 9.876543e-2 = 1.234568e5

        BOOST_DECIMAL_IF_CONSTEXPR (std::numeric_limits<T2>::digits10 > std::numeric_limits<std::uint64_t>::digits10)
        {
            constexpr boost::int128::uint128_t max_value {UINT64_C(0xF684DF56C3E0), UINT64_C(0x1BC6C73200000000)};
            if (rhs_sig >= max_value)
            {
                ++lhs_sig;
            }

            return {lhs_sig, lhs_exp, lhs_sign};
        }
        else
        {
            return {lhs_sig, lhs_exp, lhs_sign};
        }
    }

    // The two numbers can be added together without special handling
    //
    // If we can add to the lhs sig rather than dividing we can save some precision
    // 64-bit sign int can have 19 digits, and our normalized significand has 16

    if (delta_exp <= 3)
    {
        lhs_sig *= detail::pow10(static_cast<boost::int128::uint128_t>(delta_exp));
        lhs_exp -= delta_exp;
    }
    else
    {
        lhs_sig *= 1000U;
        delta_exp -= 3;
        lhs_exp -= 3;

        if (delta_exp > 1)
        {
            rhs_sig /= pow10(static_cast<boost::int128::uint128_t>(delta_exp - 1));
            delta_exp = 1;
        }

        if (delta_exp == 1)
        {
            detail::fenv_round<decimal128_t>(rhs_sig, rhs_sign);
        }
    }

    const auto new_sig {static_cast<typename ReturnType::significand_type>(lhs_sig) +
                        static_cast<typename ReturnType::significand_type>(rhs_sig)};
    const auto new_exp {lhs_exp};

    #ifdef BOOST_DECIMAL_DEBUG_ADD_128
    std::cerr << "Res Sig: " << static_cast<detail::builtin_uint128_t>(new_sig)
              << "\nRes Exp: " << new_exp
              << "\nRes Neg: " << sign << std::endl;
    #endif

    return {new_sig, new_exp, sign};
}

template <typename ReturnType, typename T, typename U>
constexpr auto d128_add_impl(T lhs_sig, U lhs_exp, bool lhs_sign,
                             T rhs_sig, U rhs_exp, bool rhs_sign,
                             bool abs_lhs_bigger) noexcept -> ReturnType
{
    auto delta_exp {lhs_exp > rhs_exp ? lhs_exp - rhs_exp : rhs_exp - lhs_exp};

    #ifdef BOOST_DECIMAL_DEBUG_ADD
    std::cerr << "Starting sig lhs: " << lhs_sig
              << "\nStarting exp lhs: " << lhs_exp
              << "\nStarting sig rhs: " << rhs_sig
              << "\nStarting exp rhs: " << rhs_exp << std::endl;
    #endif

    if (delta_exp > detail::precision_v<decimal128_t> + 1)
    {
        // If the difference in exponents is more than the digits of accuracy
        // we return the larger of the two
        //
        // e.g. 1e20 + 1e-20 = 1e20

        return abs_lhs_bigger ? ReturnType{lhs_sig, lhs_exp, lhs_sign} :
                                ReturnType{rhs_sig, rhs_exp, rhs_sign};
    }

    // The two numbers can be added together without special handling
    //
    // If we can add to the lhs sig rather than dividing we can save some precision
    // 32-bit signed int can have 9 digits and our normalized significand has 7

    auto& sig_bigger {abs_lhs_bigger ? lhs_sig : rhs_sig};
    auto& exp_bigger {abs_lhs_bigger ? lhs_exp : rhs_exp};
    auto& sig_smaller {abs_lhs_bigger ? rhs_sig : lhs_sig};
    auto& sign_smaller {abs_lhs_bigger ? rhs_sign : lhs_sign};
    auto& sign_bigger {abs_lhs_bigger ? lhs_sign : rhs_sign};

    if (delta_exp <= 2)
    {
        sig_bigger *= pow10(static_cast<boost::int128::uint128_t>(delta_exp));
        exp_bigger -= delta_exp;
        delta_exp = 0;
    }
    else
    {
        sig_bigger *= 100U;
        delta_exp -= 2;
        exp_bigger -= 2;

        if (delta_exp > 1)
        {
            sig_smaller /= pow10(static_cast<boost::int128::uint128_t>(delta_exp - 1));
            delta_exp = 1;
        }

        if (delta_exp == 1)
        {
            detail::fenv_round<decimal128_t>(sig_smaller, sign_smaller);
        }
    }

    // Cast the results to signed types so that we can apply a sign at the end if necessary
    // Both of the significands are maximally 24 bits, so they fit into a 32-bit signed type just fine
    auto signed_sig_lhs {detail::make_signed_value(sig_bigger, sign_bigger)};
    auto signed_sig_rhs {detail::make_signed_value(sig_smaller, sign_smaller)};

    const auto new_sig {signed_sig_lhs + signed_sig_rhs};
    const auto new_exp {exp_bigger};
    const auto new_sign {new_sig < 0};
    const auto res_sig {detail::make_positive_unsigned(new_sig)};

    #ifdef BOOST_DECIMAL_DEBUG_ADD
    std::cerr << "Final sig lhs: " << lhs_sig
              << "\nFinal sig rhs: " << rhs_sig
              << "\nResult sig: " << new_sig << std::endl;
    #endif

    return {res_sig, new_exp, new_sign};
}

#ifdef _MSC_VER
#  pragma warning(pop)
#endif

} // namespace detail
} // namespace decimal
} // namespace boost

#endif //BOOST_DECIMAL_DETAIL_ADD_IMPL_HPP
