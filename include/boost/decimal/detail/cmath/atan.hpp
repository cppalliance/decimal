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
constexpr auto atan_small_impl(T x) noexcept
{
    T result {};

    // 10th degree remez polynomial calculated from 0, 0.4375
    // Estimated max error: 2.3032664387910605e-12
    constexpr T a0  {UINT64_C(61037779951304161), -18, true};
    constexpr T a1  {UINT64_C(10723099589331457), -17};
    constexpr T a2  {UINT64_C(22515613909953665), -18};
    constexpr T a3  {UINT64_C(15540713402718176), -17, true};
    constexpr T a4  {UINT64_C(35999727706986597), -19};
    constexpr T a5  {UINT64_C(19938867353282852), -17};
    constexpr T a6  {UINT64_C(62252075283915644), -22};
    constexpr T a7  {UINT64_C(33333695504913247), -17, true};
    constexpr T a8  {UINT64_C(10680927642397763), -24};
    constexpr T a9  {UINT64_C(99999999877886492), -17};
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

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto atan_med_impl(T x) noexcept
{
    T result {};

    // 10th degree remez polynomial from 2.4375, 6
    // Estimated max error: 2.6239664084435361e-9
    constexpr T a0  {UINT64_C(35895331641408534), -24, true};
    constexpr T a1  {UINT64_C(1734850544519432), -21};
    constexpr T a2  {UINT64_C(38064221484608425), -21, true};
    constexpr T a3  {UINT64_C(50135011697517902), -20};
    constexpr T a4  {UINT64_C(44154446962804779), -19, true};
    constexpr T a5  {UINT64_C(27400572833763747), -18};
    constexpr T a6  {UINT64_C(12289830364128736), -17, true};
    constexpr T a7  {UINT64_C(40157034119189716), -17};
    constexpr T a8  {UINT64_C(94842703533437844), -17, true};
    constexpr T a9  {UINT64_C(15738722141839421), -16};
    constexpr T a10 {UINT64_C(1425850153011925), -16, true};

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

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto atan_large_impl(T x) noexcept
{
    T result {};

    // 10th degree remez polynomial from 6, 12
    // Estimated max error: 6.2743529396040705e-10
    constexpr T a0  {UINT64_C(33711825733904967), -27, true};
    constexpr T a1  {UINT64_C(33373728162443893), -25};
    constexpr T a2  {UINT64_C(14947421096171872), -23, true};
    constexpr T a3  {UINT64_C(39987662607208282), -22};
    constexpr T a4  {UINT64_C(7102051362837618), -20, true};
    constexpr T a5  {UINT64_C(87975481286276403), -20};
    constexpr T a6  {UINT64_C(77627729565466572), -19, true};
    constexpr T a7  {UINT64_C(48865382040050205), -18};
    constexpr T a8  {UINT64_C(21563130284459425), -17, true};
    constexpr T a9  {UINT64_C(63862756812924015), -17};
    constexpr T a10 {UINT64_C(41486607456081259), -17};

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

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto atan_huge_impl(T x) noexcept
{
    T result {};

    // 10th degree remez polynomial from 12, 24
    // Estimated max error: 4.1947468509456082e-10
    constexpr T a0  {UINT64_C(21000582873393547), -30, true};
    constexpr T a1  {UINT64_C(41414963913943078), -28};
    constexpr T a2  {UINT64_C(36923801494687764), -26, true};
    constexpr T a3  {UINT64_C(19644631420406287), -24};
    constexpr T a4  {UINT64_C(69300227486259428), -23, true};
    constexpr T a5  {UINT64_C(17021586749644597), -21};
    constexpr T a6  {UINT64_C(29708833721842521), -20, true};
    constexpr T a7  {UINT64_C(36858037393285633), -19};
    constexpr T a8  {UINT64_C(31874262996720311), -18, true};
    constexpr T a9  {UINT64_C(18322547884211479), -17};
    constexpr T a10 {UINT64_C(9387703660037886), -16};

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

template <typename T>
constexpr auto atan_impl(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
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

    if (absx <= std::numeric_limits<T>::epsilon())
    {
        return x;
    }
    else if (absx <= T{4375, -4})
    {
        result = detail::atan_small_impl(absx);
    }
    else if (absx <= T{6875, -4})
    {
        constexpr T atan_half {UINT64_C(4636476090008061162), -19};
        result = atan_half + detail::atan_small_impl((x - T{5, -1}) / (1 + x / 2));
    }
    else if (absx <= T{11875, -4})
    {
        constexpr T atan_one {UINT64_C(7853981633974483096), -19};
        result = atan_one + detail::atan_small_impl((x - 1) / (x + 1));
    }
    else if (absx <= T{24375, -4})
    {
        constexpr T atan_three_halves {UINT64_C(9827937232473290679), -19};
        result = atan_three_halves + detail::atan_small_impl((x - T{15, -1}) / (1 + T{15, -1} * x));
    }
    else if (absx <= T{6})
    {
        result = detail::atan_med_impl(x);
    }
    else if (absx <= T{12})
    {
        result = detail::atan_large_impl(x);
    }
    else if (absx <= T{24})
    {
        result = detail::atan_huge_impl(x);
    }
    else
    {
        constexpr T atan_inf {numbers::pi_v<T> / 2};
        result = atan_inf - detail::atan_small_impl(1 / x);
    }

    // arctan(-x) == -arctan(x)
    if (isneg)
    {
        result = -result;
    }

    return result;
}

} //namespace detail

template <typename T>
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
