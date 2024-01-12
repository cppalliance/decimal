// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_ASIN_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_ASIN_IMPL_HPP

#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/emulated128.hpp>
#include <boost/decimal/detail/cmath/fma.hpp>
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

template <>
constexpr auto asin_impl<decimal128>(decimal128 x) noexcept
{
    decimal128 result {};

    // 10th degree remez polynomial calculated from 0, 0.5
    // Estimated max error: 7.36516188600087509860029869041014e-11
    constexpr decimal128 a0  {uint128{UINT64_C(143010115336237), UINT64_C(516409510824292249)}, -34};
    constexpr decimal128 a1  {uint128{UINT64_C(235767315028868), UINT64_C(18335361300665096842)}, -34, true};
    constexpr decimal128 a2  {uint128{UINT64_C(165661130571303), UINT64_C(2696222372090233442)}, -34};
    constexpr decimal128 a3  {uint128{UINT64_C(75770177096626), UINT64_C(1136186724396506434)}, -34, true};
    constexpr decimal128 a4  {uint128{UINT64_C(295841983277201), UINT64_C(3737404419660904404)}, -35};
    constexpr decimal128 a5  {uint128{UINT64_C(347225199319988), UINT64_C(12011146057825411692)}, -35};
    constexpr decimal128 a6  {uint128{UINT64_C(62920056131937), UINT64_C(2964129682239744913)}, -36};
    constexpr decimal128 a7  {uint128{UINT64_C(90265192507970), UINT64_C(4360833744321461000)}, -34};
    constexpr decimal128 a8  {uint128{UINT64_C(140437214828213), UINT64_C(2679609872435210522)}, -39};
    constexpr decimal128 a9  {uint128{UINT64_C(542101067814679), UINT64_C(9115059183029878256)}, -34};
    constexpr decimal128 a10 {uint128{UINT64_C(399266221538670), UINT64_C(9343090405564050290)}, -44};

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
