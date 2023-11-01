// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_SIN_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_SIN_IMPL_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <type_traits>
#include <cstdint>

namespace boost { namespace decimal { namespace detail {

template<typename T>
constexpr auto sin_impl(T x) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T> // NOLINT(misc-no-recursion)
{
    T result { };

    if (signbit(x))
    {
        result = -sin_impl(-x);
    }
    else
    {
        // Constants calculated for [0, pi/4]
        constexpr T a0 {UINT64_C(94465293626777997), -22};
        constexpr T a1 {UINT64_C(2121051143997598), -19, true};
        constexpr T a2 {UINT64_C(10766679285155848), -21};
        constexpr T a3 {UINT64_C(83283844849017102), -19};
        constexpr T a4 {UINT64_C(13384154242642468), -22};
        constexpr T a5 {UINT64_C(16666687019704884), -17, true};
        constexpr T a6 {UINT64_C(15512908179440871), -24};
        constexpr T a7 {UINT64_C(99999999954464358), -17};
        constexpr T a8 {UINT64_C(22004194322650135), -28};

        result = a0;
        result = fma(result, x, a1);
        result = fma(result, x, a2);
        result = fma(result, x, a3);
        result = fma(result, x, a4);
        result = fma(result, x, a5);
        result = fma(result, x, a6);
        result = fma(result, x, a7);
        result = fma(result, x, a8);
    }

    return result;
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif
