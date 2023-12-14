// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_ATAN_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_ATAN_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/numbers.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/cmath/fpclassify.hpp>
#include <boost/decimal/detail/cmath/fabs.hpp>
#include <type_traits>
#include <cstdint>

namespace boost {
namespace decimal {

namespace detail {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto atan_impl(T x) noexcept
{
    T result {};

    // 10th degree remez polynomial calculated from 0, 0.4375
    constexpr T a0 {UINT64_C(61037779951304161), -18, true};
    constexpr T a1 {UINT64_C(10723099589331457), -17};
    constexpr T a2 {UINT64_C(22515613909953665), -18};
    constexpr T a3 {UINT64_C(15540713402718176), -17, true};
    constexpr T a4 {UINT64_C(35999727706986597), -19};
    constexpr T a5 {UINT64_C(19938867353282852), -17};
    constexpr T a6 {UINT64_C(62252075283915644), -22};
    constexpr T a7 {UINT64_C(33333695504913247), -17, true};
    constexpr T a8 {UINT64_C(10680927642397763), -24};
    constexpr T a9 {UINT64_C(99999999877886492), -17};
    constexpr T a10 {UINT64_C(23032664387910606), -29};

    result = a0;
    result = fma(result, x, a1);
    result = fma(result, x, a2);
    result = fma(result, x, a3);
    result = fma(result, x, a4);
    result = fma(result, x, a5);
    result = fma(result, x, a6);
    result = fma(result, x, a7);
    result = fma(result, x, a8);
    result = fma(result, x, a9);
    result = fma(result, x, a10);

    return result;
}

} //namespace detail

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto atan(T x) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T> // NOLINT(misc-no-recursion)
{
    const auto fpc {fpclassify(x)};
    const auto isneg {signbit(x)};

    if (fpc == FP_ZERO || fpc == FP_NAN)
    {
        return x;
    }
    else if (fpc == FP_INFINITE)
    {
        constexpr auto half_pi {numbers::pi_v<T> / 2};
        return isneg ? -half_pi : half_pi;
    }

    // Small angles
    const auto absx {fabs(x)};
    T result {};

    if (absx < std::numeric_limits<T>::epsilon())
    {
        return x;
    }
    else if (absx <= T{4375, -4})
    {
        result = detail::atan_impl(absx);
    }
    else if (absx <= T{6875, -4})
    {
        constexpr T atan_half {UINT64_C(4636476090008061162), -19};
        result = atan_half + detail::atan_impl((x - T{5, -1}) / (1 + x / 2));
    }
    else if (absx <= T{11875, -4})
    {
        constexpr T atan_one {UINT64_C(7853981633974483096), -19};
        result = atan_one + detail::atan_impl((x - 1) / (x + 1));
    }
    else if (absx <= T{24375, -4})
    {
        constexpr T atan_three_halves {UINT64_C(9827937232473290679), -19};
        result = atan_three_halves + detail::atan_impl((x - T{15, -1}) / (1 + T{15, -1} * x));
    }
    else
    {
        constexpr T atan_inf {numbers::pi_v<T> / 2};
        result = atan_inf - detail::atan_impl(1 / x);
    }

    // arctan(-x) == -arctan(x)
    if (isneg)
    {
        result = -result;
    }

    return result;
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_ATAN_HPP
