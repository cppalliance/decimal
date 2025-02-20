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

    // Align to larger exponent
    if (lhs_exp != rhs.biased_exponent())
    {
        constexpr auto max_shift {detail::make_positive_unsigned(detail::precision_v<decimal32> + 1)};
        const auto shift {detail::make_positive_unsigned(lhs_exp - rhs.biased_exponent())};

        if (shift > max_shift)
        {
            return lhs.full_significand() != 0U && (lhs_exp > rhs.biased_exponent()) ?
                ReturnType{lhs.full_significand(), lhs.biased_exponent(), lhs.isneg()} :
                ReturnType{rhs.full_significand(), rhs.biased_exponent(), rhs.isneg()};
        }
        else if (lhs_exp < rhs.biased_exponent())
        {
            big_rhs *= detail::pow10<promoted_sig_type>(shift);
            lhs_exp = rhs.biased_exponent() - static_cast<decimal32_components::biased_exponent_type>(shift);
        }
        else
        {
            big_lhs *= detail::pow10<promoted_sig_type>(shift);
            lhs_exp -= static_cast<decimal32_components::biased_exponent_type>(shift);
        }
    }

    // Perform signed addition with overflow protection
    const auto signed_lhs {detail::make_signed_value<add_type>(static_cast<add_type>(big_lhs), lhs.isneg())};
    const auto signed_rhs {detail::make_signed_value<add_type>(static_cast<add_type>(big_rhs), rhs.isneg())};

    const auto new_sig {signed_lhs + signed_rhs};

    return ReturnType{new_sig, lhs_exp};
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
        constexpr auto max_shift {detail::make_positive_unsigned(detail::precision_v<decimal32> + 1)};
        const auto shift {detail::make_positive_unsigned(lhs_exp - rhs_exp)};

        if (shift > max_shift)
        {
            return lhs_sig != 0U && (lhs_exp > rhs_exp) ? ReturnType{lhs_sig, lhs_exp, lhs_sign} : ReturnType{rhs_sig, rhs_exp, rhs_sign};
        }
        else if (lhs_exp < rhs_exp)
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

template <typename ReturnType, typename T, typename U>
BOOST_DECIMAL_FORCE_INLINE constexpr auto add_impl(T lhs_sig, U lhs_exp, bool lhs_sign,
                                                   T rhs_sig, U rhs_exp, bool rhs_sign) noexcept -> ReturnType
{
    const bool sign {lhs_sign};

    auto delta_exp {lhs_exp > rhs_exp ? lhs_exp - rhs_exp : rhs_exp - lhs_exp};

    #ifdef BOOST_DECIMAL_DEBUG_ADD
    std::cerr << "Starting sig lhs: " << lhs_sig
              << "\nStarting exp lhs: " << lhs_exp
              << "\nStarting sig rhs: " << rhs_sig
              << "\nStarting exp rhs: " << rhs_exp << std::endl;
    #endif

    if (delta_exp > detail::precision + 1)
    {
        // If the difference in exponents is more than the digits of accuracy
        // we return the larger of the two
        //
        // e.g. 1e20 + 1e-20 = 1e20

        #ifdef BOOST_DECIMAL_DEBUG_ADD
        std::cerr << "New sig: " << lhs_sig
                  << "\nNew exp: " << lhs_exp
                  << "\nNew neg: " << lhs_sign << std::endl;
        #endif

        return {static_cast<std::uint32_t>(lhs_sig), lhs_exp, lhs_sign};
    }

    // The two numbers can be added together without special handling
    //
    // If we can add to the lhs sig rather than dividing we can save some precision
    // 32-bit signed int can have 9 digits and our normalized significand has 7
    if (delta_exp <= 2)
    {
        lhs_sig *= pow10(static_cast<T>(delta_exp));
        lhs_exp -= delta_exp;
        delta_exp = 0;
    }
    else
    {
        lhs_sig *= 100;
        delta_exp -= 2;
        lhs_exp -=2;

        if (delta_exp > 1)
        {
            rhs_sig /= pow10(static_cast<T>(delta_exp - 1));
            delta_exp = 1;
        }

        if (delta_exp == 1)
        {
            detail::fenv_round(rhs_sig, rhs_sign);
        }
    }

    // Cast the results to signed types so that we can apply a sign at the end if necessary
    // Both of the significands are maximally 24 bits, so they fit into a 32-bit signed type just fine
    const auto new_sig {static_cast<typename ReturnType::significand_type>(lhs_sig + rhs_sig)};
    const auto new_exp {lhs_exp};

    #ifdef BOOST_DECIMAL_DEBUG_ADD
    std::cerr << "Final sig lhs: " << lhs_sig
              << "\nFinal sig rhs: " << rhs_sig
              << "\nResult sig: " << new_sig << std::endl;
    #endif

    return {new_sig, new_exp, sign};
}

template <typename ReturnType, typename T>
constexpr auto d64_add_impl(const T& lhs, const T& rhs) noexcept -> ReturnType
{
    using add_type = std::int_fast64_t;
    using significand_type = typename T::significand_type;

    auto sig_lhs {lhs.full_significand()};
    auto sig_rhs {rhs.full_significand()};
    auto lhs_exp {lhs.biased_exponent()};
    const auto rhs_exp {rhs.biased_exponent()};

    // Align to the larger exponent
    if (lhs_exp != rhs_exp)
    {
        constexpr auto max_shift {detail::make_positive_unsigned(detail::precision_v<decimal64> + 1)};
        auto shift {detail::make_positive_unsigned(lhs_exp - rhs_exp)};

        if (shift > max_shift)
        {
            return lhs.full_significand() != 0U && (lhs_exp > rhs.biased_exponent()) ?
                ReturnType{lhs.full_significand(), lhs.biased_exponent(), lhs.isneg()} :
                ReturnType{rhs.full_significand(), rhs.biased_exponent(), rhs.isneg()};
        }

        // Unlike 32 bit addition we will need to both expand one significand and shrink the other
        // If we did not do this we would end up doing 128-bit maths which requires a 128 bit division on construction
        const auto expand_shift {(std::min)(2U, shift)};
        const auto shrink_shift {shift - expand_shift};

        if (lhs_exp < rhs.biased_exponent())
        {
            sig_rhs *= detail::pow10<significand_type>(expand_shift);
            sig_lhs /= detail::pow10<significand_type>(shrink_shift);
            lhs_exp = rhs_exp - static_cast<typename T::biased_exponent_type>(expand_shift);
            BOOST_DECIMAL_ASSERT(sig_rhs >= rhs.full_significand() && sig_rhs <= static_cast<significand_type>(std::numeric_limits<add_type>::max()));
            BOOST_DECIMAL_ASSERT(sig_lhs <= lhs.full_significand());
        }
        else
        {
            sig_lhs *= detail::pow10<significand_type>(expand_shift);
            sig_rhs /= detail::pow10<significand_type>(shrink_shift);
            lhs_exp -= static_cast<typename T::biased_exponent_type>(expand_shift);
            BOOST_DECIMAL_ASSERT(sig_rhs <= rhs.full_significand());
            BOOST_DECIMAL_ASSERT(sig_lhs >= lhs.full_significand() && sig_lhs <= static_cast<significand_type>(std::numeric_limits<add_type>::max()));
        }
    }

    // Perform signed addition with overflow protection
    const auto signed_lhs {detail::make_signed_value<add_type>(static_cast<add_type>(sig_lhs), lhs.isneg())};
    const auto signed_rhs {detail::make_signed_value<add_type>(static_cast<add_type>(sig_rhs), rhs.isneg())};

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

    if (delta_exp > detail::precision_v<decimal64> + 1)
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
            detail::fenv_round<decimal64>(sig_smaller, sign_smaller);
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

    if (delta_exp > detail::precision_v<decimal128> + 1)
    {
        // If the difference in exponents is more than the digits of accuracy
        // we return the larger of the two
        //
        // e.g. 1e20 + 1e-20 = 1e20

        return {lhs_sig, lhs_exp, lhs_sign};
    }
    else if (delta_exp == detail::precision_v<decimal128> + 1)
    {
        // Only need to see if we need to add one to the
        // significand of the bigger value
        //
        // e.g. 1.234567e5 + 9.876543e-2 = 1.234568e5

        BOOST_DECIMAL_IF_CONSTEXPR (std::numeric_limits<T2>::digits10 > std::numeric_limits<std::uint64_t>::digits10)
        {
            if (rhs_sig >= detail::uint128 {UINT64_C(0xF684DF56C3E0), UINT64_C(0x1BC6C73200000000)})
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
        lhs_sig *= detail::pow10(static_cast<detail::uint128>(delta_exp));
        lhs_exp -= delta_exp;
    }
    else
    {
        lhs_sig *= 1000;
        delta_exp -= 3;
        lhs_exp -= 3;

        if (delta_exp > 1)
        {
            rhs_sig /= pow10(static_cast<uint128>(delta_exp - 1));
            delta_exp = 1;
        }

        if (delta_exp == 1)
        {
            detail::fenv_round<decimal128>(rhs_sig, rhs_sign);
        }
    }

    const auto new_sig {static_cast<typename ReturnType::significand_type>(lhs_sig) +
                        static_cast<typename ReturnType::significand_type>(rhs_sig)};
    const auto new_exp {lhs_exp};

    #ifdef BOOST_DECIMAL_DEBUG_ADD_128
    std::cerr << "Res Sig: " << static_cast<detail::uint128_t>(new_sig)
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

    if (delta_exp > detail::precision_v<decimal128> + 1)
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
        sig_bigger *= pow10(static_cast<uint128>(delta_exp));
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
            sig_smaller /= pow10(static_cast<uint128>(delta_exp - 1));
            delta_exp = 1;
        }

        if (delta_exp == 1)
        {
            detail::fenv_round<decimal128>(sig_smaller, sign_smaller);
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
