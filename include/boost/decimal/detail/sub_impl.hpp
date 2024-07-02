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

template <typename ReturnType, typename T, typename U>
BOOST_DECIMAL_FORCE_INLINE constexpr auto d32_sub_impl(T lhs_sig, U lhs_exp, bool lhs_sign,
                                                       T rhs_sig, U rhs_exp, bool rhs_sign,
                                                       bool abs_lhs_bigger) noexcept -> ReturnType
{
    using sub_type = std::int_fast32_t;

    auto delta_exp {lhs_exp > rhs_exp ? lhs_exp - rhs_exp : rhs_exp - lhs_exp};
    auto signed_sig_lhs {detail::make_signed_value(static_cast<sub_type>(lhs_sig), lhs_sign)};
    auto signed_sig_rhs {detail::make_signed_value(static_cast<sub_type>(rhs_sig), rhs_sign)};

    if (delta_exp > detail::precision + 1)
    {
        // If the difference in exponents is more than the digits of accuracy
        // we return the larger of the two
        //
        // e.g. 1e20 - 1e-20 = 1e20
        return abs_lhs_bigger ? ReturnType{lhs_sig, lhs_exp, false} :
                                ReturnType{rhs_sig, rhs_exp, true};
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
            sig_smaller /= pow10(delta_exp - 1);
            delta_exp = 1;
        }

        if (delta_exp == 1)
        {
            detail::fenv_round(sig_smaller, smaller_sign);
        }
    }

    // Both of the significands are less than 9'999'999, so we can safely
    // cast them to signed 32-bit ints to calculate the new significand
    const auto new_sig {signed_sig_lhs - signed_sig_rhs};

    const auto new_exp {abs_lhs_bigger ? lhs_exp : rhs_exp};
    const auto new_sign {new_sig < 0};
    const auto res_sig {detail::make_positive_unsigned(new_sig)};

    return {res_sig, new_exp, new_sign};
}

template <typename ReturnType, BOOST_DECIMAL_INTEGRAL T, BOOST_DECIMAL_INTEGRAL U>
constexpr auto d64_sub_impl(T lhs_sig, U lhs_exp, bool lhs_sign,
                            T rhs_sig, U rhs_exp, bool rhs_sign,
                            bool abs_lhs_bigger) noexcept -> ReturnType
{
    using sub_type = std::int_fast64_t;

    auto delta_exp {lhs_exp > rhs_exp ? lhs_exp - rhs_exp : rhs_exp - lhs_exp};
    auto signed_sig_lhs {static_cast<sub_type>(detail::make_signed_value(lhs_sig, lhs_sign))};
    auto signed_sig_rhs {static_cast<sub_type>(detail::make_signed_value(rhs_sig, rhs_sign))};

    if (delta_exp > detail::precision_v<decimal64> + 1)
    {
        // If the difference in exponents is more than the digits of accuracy
        // we return the larger of the two
        //
        // e.g. 1e20 - 1e-20 = 1e20
        return abs_lhs_bigger ? ReturnType{lhs_sig, lhs_exp, false} :
                                ReturnType{rhs_sig, rhs_exp, true};
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
            sig_smaller /= pow10<sub_type>(delta_exp - 1);
            delta_exp = 1;
        }

        if (delta_exp == 1)
        {
            detail::fenv_round<decimal64>(sig_smaller, smaller_sign);
        }
    }

    // Both of the significands are less than 9'999'999'999'999'999, so we can safely
    // cast them to signed 64-bit ints to calculate the new significand
    const auto new_sig {signed_sig_lhs - signed_sig_rhs};

    const auto new_exp {abs_lhs_bigger ? lhs_exp : rhs_exp};
    const auto new_sign {new_sig < 0};
    const auto res_sig {detail::make_positive_unsigned(new_sig)};

    return {res_sig, new_exp, new_sign};
}

template <typename ReturnType, BOOST_DECIMAL_INTEGRAL T, BOOST_DECIMAL_INTEGRAL U>
constexpr auto new_d128_sub_impl(T lhs_sig, U lhs_exp, bool lhs_sign,
                             T rhs_sig, U rhs_exp, bool rhs_sign,
                             bool abs_lhs_bigger) noexcept -> ReturnType
{
    #if defined(BOOST_DECIMAL_HAS_INT128) && (!defined(__clang_major__) || __clang_major__ > 13)
    using sub_type = detail::int128_t;
    #else
    using sub_type = detail::int128;
    #endif

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

    const auto signed_sig_lhs {detail::make_signed_value(static_cast<unsigned_sub_type>(lhs_sig), lhs_sign)};
    const auto signed_sig_rhs {detail::make_signed_value(static_cast<unsigned_sub_type>(rhs_sig), rhs_sign)};

    const auto new_sig {signed_sig_lhs - signed_sig_rhs};

    const auto new_exp {abs_lhs_bigger ? lhs_exp : rhs_exp};
    const auto new_sign {new_sig < 0};
    const auto res_sig {detail::make_positive_unsigned(new_sig)};

    return {res_sig, new_exp, new_sign};
}

template <typename ReturnType, BOOST_DECIMAL_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL U1,
                               BOOST_DECIMAL_INTEGRAL T2, BOOST_DECIMAL_INTEGRAL U2>
constexpr auto d128_sub_impl(T1 lhs_sig, U1 lhs_exp, bool lhs_sign,
                             T2 rhs_sig, U2 rhs_exp, bool rhs_sign,
                             bool abs_lhs_bigger) noexcept -> ReturnType
{
    #if defined(BOOST_DECIMAL_HAS_INT128) && (!defined(__clang_major__) || __clang_major__ > 13)
    using sub_type = detail::int128_t;
    #else
    using sub_type = detail::int128;
    #endif

    using unsigned_sub_type = detail::make_unsigned_t<sub_type>;

    auto delta_exp {lhs_exp > rhs_exp ? lhs_exp - rhs_exp : rhs_exp - lhs_exp};

    if (delta_exp > detail::precision_v<decimal128> + 1)
    {
        // If the difference in exponents is more than the digits of accuracy
        // we return the larger of the two
        //
        // e.g. 1e20 - 1e-20 = 1e20
        return abs_lhs_bigger ? ReturnType{detail::shrink_significand<detail::uint128>(lhs_sig, lhs_exp), lhs_exp, false} :
                                ReturnType{detail::shrink_significand<detail::uint128>(rhs_sig, rhs_exp), rhs_exp, true};
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

    const auto signed_sig_lhs {detail::make_signed_value(static_cast<unsigned_sub_type>(lhs_sig), lhs_sign)};
    const auto signed_sig_rhs {detail::make_signed_value(static_cast<unsigned_sub_type>(rhs_sig), rhs_sign)};

    const auto new_sig {rhs_sign && !lhs_sign ? signed_sig_lhs + signed_sig_rhs : signed_sig_lhs - signed_sig_rhs};

    const auto new_exp {abs_lhs_bigger ? lhs_exp : rhs_exp};
    const auto new_sign {new_sig < 0};
    const auto res_sig {detail::make_positive_unsigned(new_sig)};

    return {res_sig, new_exp, new_sign};
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif //BOOST_DECIMAL_DETAIL_SUB_IMPL_HPP
