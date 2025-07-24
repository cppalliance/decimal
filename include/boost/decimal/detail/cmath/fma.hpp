// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_FMA_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_FMA_HPP

#include <boost/decimal/decimal32_t.hpp>
#include <boost/decimal/decimal_fast32_t.hpp>
#include <boost/decimal/decimal64_t.hpp>
#include <boost/decimal/decimal128_t.hpp>
#include <boost/decimal/decimal_fast128_t.hpp>
#include <boost/decimal/detail/config.hpp>

namespace boost {
namespace decimal {

namespace detail {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4127)
#endif

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Dec>
using components_type = std::conditional_t<std::is_same<Dec, decimal32_t>::value, decimal32_t_components,
                        std::conditional_t<std::is_same<Dec, decimal_fast32_t>::value, decimal_fast32_t_components,
                        std::conditional_t<std::is_same<Dec, decimal64_t>::value, decimal64_t_components,
                        std::conditional_t<std::is_same<Dec, decimal_fast64_t>::value, decimal_fast64_t_components,
                        std::conditional_t<std::is_same<Dec, decimal128_t>::value, decimal128_t_components, decimal_fast128_t_components
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

    int exp_z {};
    auto sig_z = frexp10(z, &exp_z);
    detail::normalize<T>(first_res.sig, first_res.exp);

    return detail::d32_add_impl<T>(first_res.sig, first_res.exp, first_res.sign,
                                   sig_z, static_cast<exp_type>(exp_z), z < 0);
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

template <bool, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto d128_fma_impl(T x, T y, T z) noexcept -> T
{
    return x * y + z;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

constexpr auto unchecked_fma(decimal32_t x, decimal32_t y, decimal32_t z) noexcept -> decimal32_t
{
    return detail::d32_fma_impl<false>(x, y, z);
}

constexpr auto unchecked_fma(decimal_fast32_t x, decimal_fast32_t y, decimal_fast32_t z) noexcept -> decimal_fast32_t
{
    return detail::d32_fma_impl<false>(x, y, z);
}

constexpr auto unchecked_fma(decimal64_t x, decimal64_t y, decimal64_t z) noexcept -> decimal64_t
{
    return detail::d64_fma_impl<false>(x, y, z);
}

constexpr auto unchecked_fma(decimal_fast64_t x, decimal_fast64_t y, decimal_fast64_t z) noexcept -> decimal_fast64_t
{
    return detail::d64_fma_impl<false>(x, y, z);
}

constexpr auto unchecked_fma(decimal128_t x, decimal128_t y, decimal128_t z) noexcept -> decimal128_t
{
    return detail::d128_fma_impl<false>(x, y, z);
}

constexpr auto unchecked_fma(decimal_fast128_t x, decimal_fast128_t y, decimal_fast128_t z) noexcept -> decimal_fast128_t
{
    return detail::d128_fma_impl<false>(x, y, z);
}

} // Namespace detail

BOOST_DECIMAL_EXPORT constexpr auto fma(decimal32_t x, decimal32_t y, decimal32_t z) noexcept -> decimal32_t
{
    return detail::d32_fma_impl<true>(x, y, z);
}

BOOST_DECIMAL_EXPORT constexpr auto fma(decimal64_t x, decimal64_t y, decimal64_t z) noexcept -> decimal64_t
{
    return detail::d64_fma_impl<true>(x, y, z);
}

BOOST_DECIMAL_EXPORT constexpr auto fma(decimal128_t x, decimal128_t y, decimal128_t z) noexcept -> decimal128_t
{
    return detail::d128_fma_impl<true>(x, y, z);
}

BOOST_DECIMAL_EXPORT constexpr auto fma(decimal_fast32_t x, decimal_fast32_t y, decimal_fast32_t z) noexcept -> decimal_fast32_t
{
    return detail::d32_fma_impl<true>(x, y, z);
}

BOOST_DECIMAL_EXPORT constexpr auto fma(decimal_fast64_t x, decimal_fast64_t y, decimal_fast64_t z) noexcept -> decimal_fast64_t
{
    return detail::d64_fma_impl<true>(x, y, z);
}

BOOST_DECIMAL_EXPORT constexpr auto fma(decimal_fast128_t x, decimal_fast128_t y, decimal_fast128_t z) noexcept -> decimal_fast128_t
{
    return detail::d128_fma_impl<true>(x, y, z);
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_IMPL_FMA_HPP
