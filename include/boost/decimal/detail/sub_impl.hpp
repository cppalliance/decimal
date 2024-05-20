// Copyright 2023 - 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_SUB_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_SUB_IMPL_HPP

#include <boost/decimal/detail/shrink_significand.hpp>
#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/fenv_rounding.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <cstdint>
#endif

namespace boost {
namespace decimal {
namespace detail {

template <typename ReturnType, typename T1, typename T2>
constexpr auto sub_impl(T1 lhs_sig, std::int32_t lhs_exp, bool lhs_sign,
                        T2 rhs_sig, std::int32_t rhs_exp, bool rhs_sign,
                        bool abs_lhs_bigger) noexcept -> ReturnType
{
    auto delta_exp {lhs_exp > rhs_exp ? lhs_exp - rhs_exp : rhs_exp - lhs_exp};
    auto signed_sig_lhs {detail::make_signed_value(lhs_sig, lhs_sign)};
    auto signed_sig_rhs {detail::make_signed_value(rhs_sig, rhs_sign)};

    if (delta_exp > detail::precision + 1)
    {
        // If the difference in exponents is more than the digits of accuracy
        // we return the larger of the two
        //
        // e.g. 1e20 - 1e-20 = 1e20
        return abs_lhs_bigger ? ReturnType{detail::shrink_significand(lhs_sig, lhs_exp), lhs_exp, false} :
                                ReturnType{detail::shrink_significand(rhs_sig, rhs_exp), rhs_exp, true};
    }

    // The two numbers can be subtracted together without special handling

    auto& sig_bigger {abs_lhs_bigger ? signed_sig_lhs : signed_sig_rhs};
    auto& exp_bigger {abs_lhs_bigger ? lhs_exp : rhs_exp};
    auto& sig_smaller {abs_lhs_bigger ? signed_sig_rhs : signed_sig_lhs};
    auto& smaller_sign {abs_lhs_bigger ? rhs_sign : lhs_sign};

    if (delta_exp == 1)
    {
        sig_bigger *= 10;
        --delta_exp;
        --exp_bigger;
    }
    else if (delta_exp >= 2)
    {
        sig_bigger *= 100;
        delta_exp -= 2;
        exp_bigger -= 2;
    }

    if (delta_exp > 1)
    {
        sig_smaller /= pow10(delta_exp - 1);
        delta_exp = 1;
    }

    if (delta_exp == 1)
    {
        detail::fenv_round(sig_smaller, smaller_sign);
    }

    // Both of the significands are less than 9'999'999, so we can safely
    // cast them to signed 32-bit ints to calculate the new significand
    std::int32_t new_sig = (rhs_sign && !lhs_sign) ?
            static_cast<std::int32_t>(signed_sig_lhs) + static_cast<std::int32_t>(signed_sig_rhs) :
            static_cast<std::int32_t>(signed_sig_lhs) - static_cast<std::int32_t>(signed_sig_rhs);

    const auto new_exp {abs_lhs_bigger ? lhs_exp : rhs_exp};
    const auto new_sign {new_sig < 0};
    const auto res_sig {detail::make_positive_unsigned(new_sig)};

    return {res_sig, new_exp, new_sign};
}

}
}
}

#endif //BOOST_DECIMAL_DETAIL_SUB_IMPL_HPP
