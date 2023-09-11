// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_SIN_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_SIN_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/cmath/fmin.hpp>
#include <boost/decimal/detail/cmath/fmax.hpp>
#include <boost/decimal/numbers.hpp>
#include <type_traits>
#include <cstdint>

namespace boost { namespace decimal {

template<typename T, std::enable_if_t<detail::is_decimal_floating_point_v<T>, bool> = true>
constexpr auto sin(T x) noexcept -> T
{
    constexpr T zero {0, 0};

    // First check non-finite values
    if (abs(x) == zero || isinf(x) || isnan(x))
    {
        return x;
    }

    // Use argument reduction to get val in the range [-pi/2, pi/2]
    x = fmin(x, numbers::pi_v<T> - x);
    x = fmax(x, -numbers::pi_v<T> - x);
    x = fmin(x, numbers::pi_v<T> - x);

    // Pre-calculated coefficients from the minmax polynomial
    constexpr T a0 {UINT64_C(1), 0};
    constexpr T a1 {UINT64_C(1666666666666580809), -19, true};
    constexpr T a2 {UINT64_C(8333333333262716094), -21};
    constexpr T a3 {UINT64_C(1984126982005911439), -22, true};
    constexpr T a4 {UINT64_C(2755731607338689220), -24};
    constexpr T a5 {UINT64_C(2505185130214293595), -26, true};
    constexpr T a6 {UINT64_C(1604729591825977403), -28};
    constexpr T a7 {UINT64_C(7364589573262279913), -31, true};

    const T x2 = x * x;
    const T x3 = x2 * x;
    const T x4 = x2 * x2;
    const T x8 = x4 * x4;
    const T x9 = x8 * x;

    const T A = x3 * (fma(fma(x2, a3, a2), x2, a1));
    const T B = fma(fma(fma(x2, a7, a6), x2, a5), x2, a4);
    const T C = a0 * x;

    return fma(x9, B, C) + A;
}

}} // Namespaces

#endif // BOOST_DECIMAL_DETAIL_CMATH_SIN_HPP
