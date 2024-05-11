// Copyright 2024 Matt Borland
// Copyright 2024 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_ATAN_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_ATAN_IMPL_HPP

#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/cmath/impl/remez_series_result.hpp>
#include <boost/decimal/detail/cmath/impl/taylor_series_result.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <array>
#include <cstddef>
#include <cstdint>
#endif

namespace boost {
namespace decimal {
namespace detail {

namespace atan_detail {

template <bool b>
struct atan_table_imp
{
    // 10th degree remez polynomial calculated from 0, 2.4375
    // Estimated max error: 2.3032664387910605e-12
    static constexpr std::array<::boost::decimal::decimal32, 11> d32_coeffs_small =
    {{
        ::boost::decimal::decimal32 { UINT64_C(61037779951304161), -18, true },
        ::boost::decimal::decimal32 { UINT64_C(10723099589331457), -17 },
        ::boost::decimal::decimal32 { UINT64_C(22515613909953665), -18 },
        ::boost::decimal::decimal32 { UINT64_C(15540713402718176), -17, true },
        ::boost::decimal::decimal32 { UINT64_C(35999727706986597), -19 },
        ::boost::decimal::decimal32 { UINT64_C(19938867353282852), -17 },
        ::boost::decimal::decimal32 { UINT64_C(62252075283915644), -22 },
        ::boost::decimal::decimal32 { UINT64_C(33333695504913247), -17, true },
        ::boost::decimal::decimal32 { UINT64_C(10680927642397763), -24 },
        ::boost::decimal::decimal32 { UINT64_C(99999999877886492), -17 },
        ::boost::decimal::decimal32 { UINT64_C(23032664387910606), -29 },
    }};

    static constexpr std::array<::boost::decimal::decimal64, 11> d64_coeffs_small =
    {{
        ::boost::decimal::decimal64 { UINT64_C(61037779951304161), -18, true },
        ::boost::decimal::decimal64 { UINT64_C(10723099589331457), -17 },
        ::boost::decimal::decimal64 { UINT64_C(22515613909953665), -18 },
        ::boost::decimal::decimal64 { UINT64_C(15540713402718176), -17, true },
        ::boost::decimal::decimal64 { UINT64_C(35999727706986597), -19 },
        ::boost::decimal::decimal64 { UINT64_C(19938867353282852), -17 },
        ::boost::decimal::decimal64 { UINT64_C(62252075283915644), -22 },
        ::boost::decimal::decimal64 { UINT64_C(33333695504913247), -17, true },
        ::boost::decimal::decimal64 { UINT64_C(10680927642397763), -24 },
        ::boost::decimal::decimal64 { UINT64_C(99999999877886492), -17 },
        ::boost::decimal::decimal64 { UINT64_C(23032664387910606), -29 },
    }};

    // 10th degree remez polynomial from 2.4375, 6
    // Estimated max error: 2.6239664084435361e-9
    static constexpr std::array<::boost::decimal::decimal32, 11> d32_coeffs_med =
    {{
        ::boost::decimal::decimal32 { UINT64_C(35895331641408534), -24, true },
        ::boost::decimal::decimal32 { UINT64_C(1734850544519432),  -21 },
        ::boost::decimal::decimal32 { UINT64_C(38064221484608425), -21, true },
        ::boost::decimal::decimal32 { UINT64_C(50135011697517902), -20 },
        ::boost::decimal::decimal32 { UINT64_C(44154446962804779), -19, true },
        ::boost::decimal::decimal32 { UINT64_C(27400572833763747), -18 },
        ::boost::decimal::decimal32 { UINT64_C(12289830364128736), -17, true },
        ::boost::decimal::decimal32 { UINT64_C(40157034119189716), -17 },
        ::boost::decimal::decimal32 { UINT64_C(94842703533437844), -17, true },
        ::boost::decimal::decimal32 { UINT64_C(15738722141839421), -16 },
        ::boost::decimal::decimal32 { UINT64_C(1425850153011925),  -16, true },
    }};

    static constexpr std::array<::boost::decimal::decimal64, 11> d64_coeffs_med =
    {{
        ::boost::decimal::decimal64 { UINT64_C(35895331641408534), -24, true },
        ::boost::decimal::decimal64 { UINT64_C(1734850544519432),  -21 },
        ::boost::decimal::decimal64 { UINT64_C(38064221484608425), -21, true },
        ::boost::decimal::decimal64 { UINT64_C(50135011697517902), -20 },
        ::boost::decimal::decimal64 { UINT64_C(44154446962804779), -19, true },
        ::boost::decimal::decimal64 { UINT64_C(27400572833763747), -18 },
        ::boost::decimal::decimal64 { UINT64_C(12289830364128736), -17, true },
        ::boost::decimal::decimal64 { UINT64_C(40157034119189716), -17 },
        ::boost::decimal::decimal64 { UINT64_C(94842703533437844), -17, true },
        ::boost::decimal::decimal64 { UINT64_C(15738722141839421), -16 },
        ::boost::decimal::decimal64 { UINT64_C(1425850153011925),  -16, true },
    }};
};

#if !(defined(__cpp_inline_variables) && __cpp_inline_variables >= 201606L) && (!defined(_MSC_VER) || _MSC_VER != 1900)

template <bool b> constexpr std::array<decimal32, 11> atan_table_imp<b>::d32_coeffs_small;
template <bool b> constexpr std::array<decimal32, 11> atan_table_imp<b>::d32_coeffs_med;

template <bool b> constexpr std::array<decimal64, 11> atan_table_imp<b>::d64_coeffs_small;
template <bool b> constexpr std::array<decimal64, 11> atan_table_imp<b>::d64_coeffs_med;

#endif

using atan_table = atan_table_imp<true>;

} //namespace atan_detail

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto atan_series_small(T x) noexcept;

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto atan_series_med  (T x) noexcept;

template <> constexpr auto atan_series_small<decimal32>(decimal32 x) noexcept { return remez_series_result(x, atan_detail::atan_table::d32_coeffs_small); }
template <> constexpr auto atan_series_med  <decimal32>(decimal32 x) noexcept { return remez_series_result(x, atan_detail::atan_table::d32_coeffs_med  ); }

template <> constexpr auto atan_series_small<decimal64>(decimal64 x) noexcept { return remez_series_result(x, atan_detail::atan_table::d64_coeffs_small); }
template <> constexpr auto atan_series_med  <decimal64>(decimal64 x) noexcept { return remez_series_result(x, atan_detail::atan_table::d64_coeffs_med  ); }

} //namespace detail
} //namespace decimal
} //namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_IMPL_ATAN_IMPL_HPP
