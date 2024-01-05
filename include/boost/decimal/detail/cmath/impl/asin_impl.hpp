// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_ASIN_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_ASIN_IMPL_HPP

#include <boost/decimal/detail/concepts.hpp>
#include <cstdint>

namespace boost {
namespace decimal {
namespace detail {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto asin_impl(T x) noexcept
{
    T result {};

    // 10th degree remez polynomial calculated from 0, 0.5
    // Estimated max error: 7.3651618860008751e-11
    constexpr T a0  {UINT64_C(263887099755925), -15};
    constexpr T a1  {UINT64_C(43491393212832818), -17, true};
    constexpr T a2  {UINT64_C(38559884786102105), -17};
    constexpr T a3  {UINT64_C(13977130653211101), -17, true};
    constexpr T a4  {UINT64_C(54573213517731915), -18};
    constexpr T a5  {UINT64_C(64851743877986187), -18};
    constexpr T a6  {UINT64_C(11606701725692841), -19};
    constexpr T a7  {UINT64_C(16658989049586517), -17};
    constexpr T a8  {UINT64_C(25906093603686159), -22};
    constexpr T a9  {UINT64_C(99999996600828589), -17};
    constexpr T a10 {UINT64_C(73651618860008751), -27};

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
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_IMPL_ASIN_IMPL_HPP
