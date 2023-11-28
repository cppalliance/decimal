// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_NUMBERS_HPP
#define BOOST_DECIMAL_NUMBERS_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/literals.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/emulated128.hpp>
#include <cstdint>

namespace boost { namespace decimal { namespace numbers {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec e_v = Dec{UINT64_C(2718281828459045235), -18};

template <>
constexpr decimal128 e_v<decimal128> = decimal128{detail::uint128{UINT64_C(147358353192158),
                                                                  UINT64_C(5661142159003925334)}, -33};

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec log2e_v = Dec{UINT64_C(1442695040888963407), -18};

template <>
constexpr decimal128 log2e_v<decimal128> = decimal128{detail::uint128{UINT64_C(78208654878293),
                                                                      UINT64_C(16395798456599530402)}, -33};

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec log10e_v = Dec{UINT64_C(4342944819032518277), -19};

template <>
constexpr decimal128 log10e_v<decimal128> = decimal128{detail::uint128{UINT64_C(235431510388986),
                                                                       UINT64_C(2047877485384264674)}, -34};

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec pi_v = Dec{UINT64_C(3141592653589793238), -18};

template <>
constexpr decimal128 pi_v<decimal128> = decimal128{detail::uint128{UINT64_C(170306079004327),
                                                                   UINT64_C(13456286628489437068)}, -33};

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec inv_pi_v = Dec{UINT64_C(3183098861837906715), -19};

template <>
constexpr decimal128 inv_pi_v<decimal128> = decimal128{detail::uint128{UINT64_C(172556135062039),
                                                                       UINT64_C(13820348844234745256)}, -34};

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec inv_sqrtpi_v = Dec{UINT64_C(5641895835477562869), -19};

template <>
constexpr decimal128 inv_sqrtpi_v<decimal128> = decimal128{detail::uint128{UINT64_C(305847786088084),
                                                                           UINT64_C(12695685840195063976)}, -34};

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec ln2_v = Dec{UINT64_C(6931471805599453094), -19};

template <>
constexpr decimal128 ln2_v<decimal128> = decimal128{detail::uint128{UINT64_C(375755839507647),
                                                                    UINT64_C(8395602002641374208)}, -34};

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec ln10_v = Dec{UINT64_C(2302585092994045684), -18};

template <>
constexpr decimal128 ln10_v<decimal128> = decimal128{detail::uint128{UINT64_C(124823388007844),
                                                                     UINT64_C(1462833818723808456)}, -33};

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec sqrt2_v = Dec{UINT64_C(1414213562373095049), -18};

template <>
constexpr decimal128 sqrt2_v<decimal128> = decimal128{detail::uint128{UINT64_C(76664670834168),
                                                                      UINT64_C(12987834932751794202)}, -33};

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec sqrt3_v = Dec{UINT64_C(1732050807568877294), -18};

template <>
constexpr decimal128 sqrt3_v<decimal128> = decimal128{detail::uint128{UINT64_C(93894662421072),
                                                                      UINT64_C(8437766544231453518)}, -33};

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec inv_sqrt2_v = Dec{UINT64_C(7071067811865475244), -19};

template <>
constexpr decimal128 inv_sqrt2_v<decimal128> = decimal128{detail::uint128{UINT64_C(383323354170843),
                                                                          UINT64_C(9598942442630316202)}, -34};

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec inv_sqrt3_v = Dec{UINT64_C(5773502691896257645), -19};

template <>
constexpr decimal128 inv_sqrt3_v<decimal128> = decimal128{detail::uint128{UINT64_C(312982208070241),
                                                                          UINT64_C(9679144407061960114)}, -34};

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec egamma_v = Dec{UINT64_C(5772156649015328606), -19};

template <>
constexpr decimal128 egamma_v<decimal128> = decimal128{detail::uint128{UINT64_C(312909238939453),
                                                                       UINT64_C(7916302232898517972)}, -34};

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec phi_v = Dec{UINT64_C(1618033988749894848), -18};

template <>
constexpr decimal128 phi_v<decimal128> = decimal128{detail::uint128{UINT64_C(87713798287901),
                                                                    UINT64_C(2061523135646567614)}, -33};

static constexpr auto e {e_v<decimal64>};
static constexpr auto log2e {log2e_v<decimal64>};
static constexpr auto log10e {log10e_v<decimal64>};
static constexpr auto pi {pi_v<decimal64>};
static constexpr auto inv_pi {inv_pi_v<decimal64>};
static constexpr auto inv_sqrtpi {inv_sqrtpi_v<decimal64>};
static constexpr auto ln2 {ln2_v<decimal64>};
static constexpr auto ln10 {ln10_v<decimal64>};
static constexpr auto sqrt2 {sqrt2_v<decimal64>};
static constexpr auto sqrt3 {sqrt3_v<decimal64>};
static constexpr auto inv_sqrt2 {inv_sqrt2_v<decimal64>};
static constexpr auto inv_sqrt3 {inv_sqrt3_v<decimal64>};
static constexpr auto egamma {egamma_v<decimal64>};
static constexpr auto phi {phi_v<decimal64>};

} // namespace numbers
} // namespace decimal
} // namespace boost

#endif //BOOST_DECIMAL_NUMBERS_HPP
