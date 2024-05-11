// Copyright 2023 - 2024 Matt Borland
// Copyright 2023 - 2024 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_ATAN_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_ATAN_HPP

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/cmath/impl/atan_impl.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/numbers.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <type_traits>
#include <cstdint>
#endif

namespace boost {
namespace decimal {

namespace detail {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto atan_small_impl(T x) noexcept
{
    return detail::atan_series_small(x);
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto atan_med_impl(T x) noexcept
{
    return detail::atan_series_med(x);
}

template <typename T>
constexpr auto atan_impl_cases(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    if (x <= std::numeric_limits<T>::epsilon())
    {
        return x;
    }
    else if (x <= T { 4375, -4 })
    {
        return detail::atan_small_impl(x);
    }
    else if (x <= T { 6875, -4 })
    {
        constexpr T atan_half { UINT64_C(4636476090008061162), -19 };

        return atan_half + detail::atan_small_impl((x - T{5, -1}) / (1 + x / 2));
    }
    else if (x <= T{11875, -4})
    {
        constexpr T atan_one {UINT64_C(7853981633974483096), -19};

        return atan_one + detail::atan_small_impl((x - 1) / (x + 1));
    }
    else if (x <= T { 24375, -4 })
    {
        constexpr T atan_three_halves {UINT64_C(9827937232473290679), -19};

        return atan_three_halves + detail::atan_small_impl((x - T{15, -1}) / (1 + T{15, -1} * x));
    }
    else
    {
        // x <= T { 6 }
        return detail::atan_med_impl(x);
    }
}

template <typename T>
constexpr auto atan_impl(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    const auto fpc { fpclassify(x) };

    T result { };

    constexpr T my_pi_half { numbers::pi_v<T> / 2 };

    if (fpc == FP_ZERO || fpc == FP_NAN)
    {
        result = x;
    }
    else if (signbit(x))
    {
        result = -atan_impl(-x);
    }
    else if (fpc == FP_INFINITE)
    {
        result = my_pi_half;
    }
    else
    {
        if (x <= T { 6 })
        {
            result = atan_impl_cases(x);
        }
        else if (x <= T { 24 })
        {
            // The algorithm for arc-tangent of large-valued argument is based
            // on Chapter 11, page 194 of Cody and Waite, "Software Manual
            // for the Elementary Functions", Prentice Hall, 1980.

            const T f { ((x * numbers::sqrt3_v<T>) - T { 1 }) / (numbers::sqrt3_v<T> + x) };

            result = (numbers::pi_v<T> / static_cast<int>(INT8_C(6))) + atan_impl_cases(f);
        }
        else
        {
            result = my_pi_half - detail::atan_small_impl(1 / x);
        }
    }

    return result;
}

} //namespace detail

BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto atan(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    #if BOOST_DECIMAL_DEC_EVAL_METHOD == 0

    using evaluation_type = T;

    #elif BOOST_DECIMAL_DEC_EVAL_METHOD == 1

    using evaluation_type = detail::promote_args_t<T, decimal64>;

    #else // BOOST_DECIMAL_DEC_EVAL_METHOD == 2

    using evaluation_type = detail::promote_args_t<T, decimal128>;

    #endif

    return static_cast<T>(detail::atan_impl(static_cast<evaluation_type>(x)));
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_ATAN_HPP
