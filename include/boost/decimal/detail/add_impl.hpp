// Copyright 2023 - 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_ADD_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_ADD_IMPL_HPP

#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/fenv_rounding.hpp>
#include <cstdint>

namespace boost {
namespace decimal {
namespace detail {

template <typename ReturnType, typename T, typename T2>
constexpr auto add_impl(T lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                        T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign) noexcept -> ReturnType
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
    else if (delta_exp == detail::precision + 1)
    {
        // Only need to see if we need to add one to the
        // significand of the bigger value
        //
        // e.g. 1.234567e5 + 9.876543e-2 = 1.234568e5

        if (rhs_sig >= UINT32_C(5'000'000))
        {
            ++lhs_sig;
        }

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
        while (delta_exp > 0)
        {
            lhs_sig *= 10;
            --delta_exp;
            --lhs_exp;
        }
    }
    else
    {
        lhs_sig *= 100;
        delta_exp -= 2;
        lhs_exp -=2;
    }

    while (delta_exp > 1)
    {
        rhs_sig /= 10;
        --delta_exp;
    }


    if (delta_exp == 1)
    {
        detail::fenv_round(rhs_sig, rhs_sign);
    }

    // Cast the results to signed types so that we can apply a sign at the end if necessary
    // Both of the significands are maximally 24 bits, so they fit into a 32-bit signed type just fine
    const auto new_sig {static_cast<std::int32_t>(lhs_sig + rhs_sig)};
    const auto new_exp {lhs_exp};
    const auto res_sig {detail::make_positive_unsigned(new_sig)};

    #ifdef BOOST_DECIMAL_DEBUG_ADD
    std::cerr << "Final sig lhs: " << lhs_sig
              << "\nFinal sig rhs: " << rhs_sig
              << "\nResult sig: " << new_sig << std::endl;
    #endif

    return {res_sig, new_exp, sign};
}

}
}
}

#endif //BOOST_DECIMAL_DETAIL_ADD_IMPL_HPP
