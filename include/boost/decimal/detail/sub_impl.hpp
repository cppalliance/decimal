// Copyright 2023 - 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_SUB_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_SUB_IMPL_HPP

#include <boost/decimal/detail/shrink_significand.hpp>
#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/fenv_rounding.hpp>
#include <boost/int128/int128.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <cstdint>
#endif

namespace boost {
namespace decimal {
namespace detail {

template <typename ReturnType, BOOST_DECIMAL_INTEGRAL T, BOOST_DECIMAL_INTEGRAL U>
constexpr auto d128_sub_impl(T lhs_sig, U lhs_exp, bool lhs_sign,
                             T rhs_sig, U rhs_exp, bool rhs_sign,
                             bool abs_lhs_bigger) noexcept -> ReturnType
{
    using sub_type = boost::int128::int128_t;
    using unsigned_sub_type = detail::make_unsigned_t<sub_type>;

    auto delta_exp {lhs_exp > rhs_exp ? lhs_exp - rhs_exp : rhs_exp - lhs_exp};

    if (delta_exp > detail::precision_v<decimal128> + 1)
    {
        // If the difference in exponents is more than the digits of accuracy
        // we return the larger of the two
        //
        // e.g. 1e20 - 1e-20 = 1e20
        return abs_lhs_bigger ? ReturnType{lhs_sig, lhs_exp, false} :
                                ReturnType{rhs_sig, rhs_exp, true};
    }

    // The two numbers can be subtracted together without special handling

    auto& sig_bigger {abs_lhs_bigger ? lhs_sig : rhs_sig};
    auto& exp_bigger {abs_lhs_bigger ? lhs_exp : rhs_exp};
    auto& sig_smaller {abs_lhs_bigger ? rhs_sig : lhs_sig};
    auto& smaller_sign {abs_lhs_bigger ? rhs_sign : lhs_sign};

    if (delta_exp == 1)
    {
        sig_bigger *= 10;
        --delta_exp;
        --exp_bigger;
    }
    else
    {
        if (delta_exp >= 2)
        {
            sig_bigger *= 100;
            delta_exp -= 2;
            exp_bigger -= 2;
        }

        if (delta_exp > 1)
        {
            sig_smaller /= pow10<std::remove_reference_t<decltype(sig_smaller)>>(delta_exp - 1);
            delta_exp = 1;
        }

        if (delta_exp == 1)
        {
            detail::fenv_round<decimal128>(sig_smaller, smaller_sign);
        }
    }

    static_assert(!std::is_same<T, unsigned_sub_type>::value, "TODO(mborland): update the following two lines!");
    const auto signed_sig_lhs {detail::make_signed_value(unsigned_sub_type{lhs_sig.high, lhs_sig.low}, lhs_sign)};
    const auto signed_sig_rhs {detail::make_signed_value(unsigned_sub_type{rhs_sig.high, rhs_sig.low}, rhs_sign)};

    const auto new_sig {signed_sig_lhs - signed_sig_rhs};

    const auto new_exp {abs_lhs_bigger ? lhs_exp : rhs_exp};
    const auto new_sign {new_sig < 0};
    const auto res_sig {detail::make_positive_unsigned(new_sig)};

    return {T{res_sig.high, res_sig.low}, new_exp, new_sign};
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif //BOOST_DECIMAL_DETAIL_SUB_IMPL_HPP
