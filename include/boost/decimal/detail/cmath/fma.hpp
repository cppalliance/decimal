// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_FMA_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_FMA_HPP

#include <boost/decimal/decimal32.hpp>
#include <boost/decimal/decimal32_fast.hpp>
#include <boost/decimal/decimal64.hpp>
#include <boost/decimal/decimal128.hpp>
#include <boost/decimal/decimal128_fast.hpp>
#include <boost/decimal/detail/config.hpp>

namespace boost {
namespace decimal {

namespace detail {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4127)
#endif

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Dec>
using components_type = std::conditional_t<std::is_same<Dec, decimal32>::value, decimal32_components,
                        std::conditional_t<std::is_same<Dec, decimal32_fast>::value, decimal32_fast_components,
                        std::conditional_t<std::is_same<Dec, decimal64>::value, decimal64_components,
                        std::conditional_t<std::is_same<Dec, decimal64_fast>::value, decimal64_fast_components,
                        std::conditional_t<std::is_same<Dec, decimal128>::value, decimal128_components, decimal128_fast_components
                        >>>>>;

template <bool checked, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto d32_fma_impl(T x, T y, T z) noexcept -> T
{
    using T_components_type = components_type<T>;
    using exp_type = typename T::biased_exponent_type;

    // Apply the add
    #ifndef BOOST_DECIMAL_FAST_MATH
    BOOST_DECIMAL_IF_CONSTEXPR (checked)
    {
        if (!isfinite(x) || !isfinite(y))
        {
            return detail::check_non_finite(x, y);
        }
    }
    #endif

    int exp_lhs {};
    auto sig_lhs = frexp10(x, &exp_lhs);

    int exp_rhs {};
    auto sig_rhs = frexp10(y, &exp_rhs);

    auto first_res = detail::mul_impl<T_components_type>(sig_lhs, static_cast<exp_type>(exp_lhs), x < 0,
                                                         sig_rhs, static_cast<exp_type>(exp_rhs), y < 0);

    // Apply the mul on the carried components
    // We still create the result as a decimal type to check for non-finite values and comparisons,
    // but we do not use it for the resultant calculation
    const T complete_lhs {first_res.sig, first_res.exp, first_res.sign};

    #ifndef BOOST_DECIMAL_FAST_MATH
    BOOST_DECIMAL_IF_CONSTEXPR (checked)
    {
        if (!isfinite(complete_lhs) || !isfinite(z))
        {
            return detail::check_non_finite(complete_lhs, z);
        }
    }
    #endif

    const bool abs_lhs_bigger {abs(complete_lhs) > abs(z)};

    int exp_z {};
    auto sig_z = frexp10(z, &exp_z);
    detail::normalize<T>(first_res.sig, first_res.exp);

    return detail::d32_add_impl<T>(first_res.sig, first_res.exp, first_res.sign,
                                   sig_z, static_cast<exp_type>(exp_z), z < 0,
                                   abs_lhs_bigger);
}

template <bool checked, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto d64_fma_impl(T x, T y, T z) noexcept -> T
{
    using T_components_type = components_type<T>;
    using exp_type = typename T::biased_exponent_type;

    // Apply the add
    #ifndef BOOST_DECIMAL_FAST_MATH
    BOOST_DECIMAL_IF_CONSTEXPR (checked)
    {
        if (!isfinite(x) || !isfinite(y))
        {
            return detail::check_non_finite(x, y);
        }
    }
    #endif

    int exp_lhs {};
    auto sig_lhs = frexp10(x, &exp_lhs);

    int exp_rhs {};
    auto sig_rhs = frexp10(y, &exp_rhs);

    auto first_res = detail::d64_mul_impl<T_components_type>(sig_lhs, static_cast<exp_type>(exp_lhs), x < 0,
                                                             sig_rhs, static_cast<exp_type>(exp_rhs), y < 0);

    // Apply the mul on the carried components
    // We still create the result as a decimal type to check for non-finite values and comparisons,
    // but we do not use it for the resultant calculation
    const T complete_lhs {first_res.sig, first_res.exp, first_res.sign};

    #ifndef BOOST_DECIMAL_FAST_MATH
    BOOST_DECIMAL_IF_CONSTEXPR (checked)
    {
        if (!isfinite(complete_lhs) || !isfinite(z))
        {
            return detail::check_non_finite(complete_lhs, z);
        }
    }
    #endif

    const bool abs_lhs_bigger {abs(complete_lhs) > abs(z)};

    int exp_z {};
    auto sig_z = frexp10(z, &exp_z);
    detail::normalize<T>(first_res.sig, first_res.exp);

    return detail::d64_add_impl<T>(first_res.sig, first_res.exp, first_res.sign,
                                   sig_z, static_cast<exp_type>(exp_z), z < 0,
                                   abs_lhs_bigger);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

constexpr auto unchecked_fma(decimal32 x, decimal32 y, decimal32 z) noexcept -> decimal32
{
    return detail::d32_fma_impl<false>(x, y, z);
}

constexpr auto unchecked_fma(decimal32_fast x, decimal32_fast y, decimal32_fast z) noexcept -> decimal32_fast
{
    return detail::d32_fma_impl<false>(x, y, z);
}

constexpr auto unchecked_fma(decimal64 x, decimal64 y, decimal64 z) noexcept -> decimal64
{
    return detail::d64_fma_impl<false>(x, y, z);
}

constexpr auto unchecked_fma(decimal64_fast x, decimal64_fast y, decimal64_fast z) noexcept -> decimal64_fast
{
    return detail::d64_fma_impl<false>(x, y, z);
}

} // Namespace detail

BOOST_DECIMAL_EXPORT constexpr auto fma(decimal32 x, decimal32 y, decimal32 z) noexcept -> decimal32
{
    return detail::d32_fma_impl<true>(x, y, z);
}

BOOST_DECIMAL_EXPORT constexpr auto fma(decimal64 x, decimal64 y, decimal64 z) noexcept -> decimal64
{
    return detail::d64_fma_impl<true>(x, y, z);
}

BOOST_DECIMAL_EXPORT constexpr auto fma(decimal128 x, decimal128 y, decimal128 z) noexcept -> decimal128
{
    return x * y + z;
}

BOOST_DECIMAL_EXPORT constexpr auto fma(decimal32_fast x, decimal32_fast y, decimal32_fast z) noexcept -> decimal32_fast
{
    return detail::d32_fma_impl<true>(x, y, z);
}

BOOST_DECIMAL_EXPORT constexpr auto fma(decimal64_fast x, decimal64_fast y, decimal64_fast z) noexcept -> decimal64_fast
{
    return detail::d64_fma_impl<true>(x, y, z);
}

BOOST_DECIMAL_EXPORT constexpr auto fma(decimal128_fast x, decimal128_fast y, decimal128_fast z) noexcept -> decimal128_fast
{
    return x * y + z;
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_IMPL_FMA_HPP
