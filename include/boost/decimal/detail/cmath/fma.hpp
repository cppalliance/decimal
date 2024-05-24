// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_FMA_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_FMA_HPP

#include <boost/decimal/decimal32.hpp>
#include <boost/decimal/decimal32_fast.hpp>
#include <boost/decimal/decimal64.hpp>
#include <boost/decimal/decimal128.hpp>
#include <boost/decimal/detail/config.hpp>

namespace boost {
namespace decimal {

constexpr auto fmad32(decimal32 x, decimal32 y, decimal32 z) noexcept -> decimal32
{
    // First calculate x * y without rounding
    constexpr decimal32 zero {0, 0};

    const auto res {detail::check_non_finite(x, y)};
    if (res != zero)
    {
        return res;
    }

    auto sig_lhs {x.full_significand()};
    auto exp_lhs {x.biased_exponent()};
    detail::normalize(sig_lhs, exp_lhs);

    auto sig_rhs {y.full_significand()};
    auto exp_rhs {y.biased_exponent()};
    detail::normalize(sig_rhs, exp_rhs);

    auto mul_result {detail::mul_impl<detail::decimal32_components>(sig_lhs, exp_lhs, x.isneg(), sig_rhs, exp_rhs, y.isneg())};
    const decimal32 dec_result {mul_result.sig, mul_result.exp, mul_result.sign};

    const auto res_add {detail::check_non_finite(dec_result, z)};
    if (res_add != zero)
    {
        return res_add;
    }

    bool lhs_bigger {dec_result > z};
    if (dec_result.isneg() && z.isneg())
    {
        lhs_bigger = !lhs_bigger;
    }
    bool abs_lhs_bigger {abs(dec_result) > abs(z)};

    // To avoid the rounding step we promote the constituent pieces to the next higher type
    detail::decimal64_components promoted_mul_result {static_cast<std::uint64_t>(mul_result.sig),
                                                      mul_result.exp, mul_result.sign};

    detail::normalize<decimal64>(promoted_mul_result.sig, promoted_mul_result.exp);

    auto sig_z {static_cast<std::uint64_t>(z.full_significand())};
    auto exp_z {z.biased_exponent()};
    detail::normalize<decimal64>(sig_z, exp_z);
    detail::decimal64_components z_components {sig_z, exp_z, z.isneg()};

    if (!lhs_bigger)
    {
        detail::swap(promoted_mul_result, z_components);
        abs_lhs_bigger = !abs_lhs_bigger;
    }

    detail::decimal64_components result {};

    if (!promoted_mul_result.sign && z_components.sign)
    {
        result = d64_sub_impl(promoted_mul_result.sig, promoted_mul_result.exp, promoted_mul_result.sign,
                              z_components.sig, z_components.exp, z_components.sign,
                              abs_lhs_bigger);
    }
    else
    {
        result = d64_add_impl(promoted_mul_result.sig, promoted_mul_result.exp, promoted_mul_result.sign,
                              z_components.sig, z_components.exp, z_components.sign);
    }

    return {result.sig, result.exp, result.sign};
}

constexpr auto fmad64(decimal64 x, decimal64 y, decimal64 z) noexcept -> decimal64
{
    // First calculate x * y without rounding
    constexpr decimal64 zero {0, 0};

    const auto res {detail::check_non_finite(x, y)};
    if (res != zero)
    {
        return res;
    }

    auto sig_lhs {x.full_significand()};
    auto exp_lhs {x.biased_exponent()};
    detail::normalize<decimal64>(sig_lhs, exp_lhs);

    auto sig_rhs {y.full_significand()};
    auto exp_rhs {y.biased_exponent()};
    detail::normalize<decimal64>(sig_rhs, exp_rhs);

    auto mul_result {d64_mul_impl(sig_lhs, exp_lhs, x.isneg(), sig_rhs, exp_rhs, y.isneg())};
    const decimal64 dec_result {mul_result.sig, mul_result.exp, mul_result.sign};

    const auto res_add {detail::check_non_finite(dec_result, z)};
    if (res_add != zero)
    {
        return res_add;
    }

    bool lhs_bigger {dec_result > z};
    if (dec_result.isneg() && z.isneg())
    {
        lhs_bigger = !lhs_bigger;
    }
    bool abs_lhs_bigger {abs(dec_result) > abs(z)};

    // To avoid the rounding step we promote the constituent pieces to the next higher type
    detail::decimal128_components promoted_mul_result {static_cast<detail::uint128>(mul_result.sig),
                                                       mul_result.exp, mul_result.sign};

    detail::normalize<decimal128>(promoted_mul_result.sig, promoted_mul_result.exp);

    auto sig_z {static_cast<detail::uint128>(z.full_significand())};
    auto exp_z {z.biased_exponent()};
    detail::normalize<decimal128>(sig_z, exp_z);
    detail::decimal128_components z_components {sig_z, exp_z, z.isneg()};

    if (!lhs_bigger)
    {
        detail::swap(promoted_mul_result, z_components);
        abs_lhs_bigger = !abs_lhs_bigger;
    }

    detail::decimal128_components result {};

    if (!promoted_mul_result.sign && z_components.sign)
    {
        result = d128_sub_impl(promoted_mul_result.sig, promoted_mul_result.exp, promoted_mul_result.sign,
                               z_components.sig, z_components.exp, z_components.sign,
                               abs_lhs_bigger);
    }
    else
    {
        result = d128_add_impl(promoted_mul_result.sig, promoted_mul_result.exp, promoted_mul_result.sign,
                               z_components.sig, z_components.exp, z_components.sign);
    }

    return {result.sig, result.exp, result.sign};
}

constexpr auto fmad128(decimal128 x, decimal128 y, decimal128 z) noexcept -> decimal128
{
    return x * y + z;
}

// TODO(mborland): promote to decimal64_fast instead of regular decimal64 once it is available
constexpr auto fmad32f(decimal32_fast x, decimal32_fast y, decimal32_fast z) noexcept -> decimal32_fast
{
    // First calculate x * y without rounding
    constexpr decimal32_fast zero {0, 0};

    const auto res {detail::check_non_finite(x, y)};
    if (res != zero)
    {
        return res;
    }

    auto sig_lhs {x.full_significand()};
    auto exp_lhs {x.biased_exponent()};
    detail::normalize(sig_lhs, exp_lhs);

    auto sig_rhs {y.full_significand()};
    auto exp_rhs {y.biased_exponent()};
    detail::normalize(sig_rhs, exp_rhs);

    auto mul_result {detail::mul_impl<detail::decimal32_fast_components>(sig_lhs, exp_lhs, x.isneg(), sig_rhs, exp_rhs, y.isneg())};
    const decimal32_fast dec_result {mul_result.sig, mul_result.exp, mul_result.sign};

    const auto res_add {detail::check_non_finite(dec_result, z)};
    if (res_add != zero)
    {
        return res_add;
    }

    bool lhs_bigger {dec_result > z};
    if (dec_result.isneg() && z.isneg())
    {
        lhs_bigger = !lhs_bigger;
    }
    bool abs_lhs_bigger {abs(dec_result) > abs(z)};

    // To avoid the rounding step we promote the constituent pieces to the next higher type
    detail::decimal64_components promoted_mul_result {static_cast<std::uint64_t>(mul_result.sig),
                                                      mul_result.exp, mul_result.sign};

    detail::normalize<decimal64>(promoted_mul_result.sig, promoted_mul_result.exp);

    auto sig_z {static_cast<std::uint64_t>(z.full_significand())};
    auto exp_z {z.biased_exponent()};
    detail::normalize<decimal64>(sig_z, exp_z);
    detail::decimal64_components z_components {sig_z, exp_z, z.isneg()};

    if (!lhs_bigger)
    {
        detail::swap(promoted_mul_result, z_components);
        abs_lhs_bigger = !abs_lhs_bigger;
    }

    detail::decimal64_components result {};

    if (!promoted_mul_result.sign && z_components.sign)
    {
        result = d64_sub_impl(promoted_mul_result.sig, promoted_mul_result.exp, promoted_mul_result.sign,
                              z_components.sig, z_components.exp, z_components.sign,
                              abs_lhs_bigger);
    }
    else
    {
        result = d64_add_impl(promoted_mul_result.sig, promoted_mul_result.exp, promoted_mul_result.sign,
                              z_components.sig, z_components.exp, z_components.sign);
    }

    return {result.sig, result.exp, result.sign};
}

BOOST_DECIMAL_EXPORT constexpr auto fma(decimal32 x, decimal32 y, decimal32 z) noexcept -> decimal32
{
    return fmad32(x, y, z);
}

BOOST_DECIMAL_EXPORT constexpr auto fma(decimal64 x, decimal64 y, decimal64 z) noexcept -> decimal64
{
    return fmad64(x, y, z);
}

BOOST_DECIMAL_EXPORT constexpr auto fma(decimal128 x, decimal128 y, decimal128 z) noexcept -> decimal128
{
    return fmad128(x, y, z);
}

BOOST_DECIMAL_EXPORT constexpr auto fma(decimal32_fast x, decimal32_fast y, decimal32_fast z) noexcept -> decimal32_fast
{
    return fmad32f(x, y, z);
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_IMPL_FMA_HPP
