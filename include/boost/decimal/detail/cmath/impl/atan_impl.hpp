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
    static constexpr std::array<::boost::decimal::decimal32, 3> d32_atan_values =
    {{
        ::boost::decimal::decimal32 { UINT64_C(4636476090008061162), -19 }, // atan_half
        ::boost::decimal::decimal32 { UINT64_C(7853981633974483096), -19 }, // atan_one
        ::boost::decimal::decimal32 { UINT64_C(9827937232473290679), -19 }, // atan_three_halves
    }};

    static constexpr std::array<::boost::decimal::decimal64, 3> d64_atan_values =
    {{
        ::boost::decimal::decimal64 { UINT64_C(4636476090008061162), -19 }, // atan_half
        ::boost::decimal::decimal64 { UINT64_C(7853981633974483096), -19 }, // atan_one
        ::boost::decimal::decimal64 { UINT64_C(9827937232473290679), -19 }, // atan_three_halves
    }};

    static constexpr std::array<::boost::decimal::decimal128, 3> d128_atan_values =
    {{
        ::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(251343872473191), UINT64_C(15780610568723885484) }, -34 }, // atan_half
        ::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(425765197510819), UINT64_C(5970600460659265246)  }, -34 }, // atan_one
        ::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(532773544924935), UINT64_C(16408933314882201700) }, -34 }, // atan_three_halves
    }};

    // 10th degree remez polynomial calculated from 0, 0.4375
    // Estimated max error: 2.3032664387910605e-12
    static constexpr std::array<::boost::decimal::decimal32, 11> d32_coeffs =
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

    static constexpr std::array<::boost::decimal::decimal64, 11> d64_coeffs =
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
};

#if !(defined(__cpp_inline_variables) && __cpp_inline_variables >= 201606L) && (!defined(_MSC_VER) || _MSC_VER != 1900)

template <bool b> constexpr std::array<decimal32, 11> atan_table_imp<b>::d32_coeffs;
template <bool b> constexpr std::array<decimal64, 11> atan_table_imp<b>::d64_coeffs;

template <bool b> constexpr std::array<decimal32,  3> atan_table_imp<b>::d32_atan_values;
template <bool b> constexpr std::array<decimal64,  3> atan_table_imp<b>::d64_atan_values;
template <bool b> constexpr std::array<decimal128,  3> atan_table_imp<b>::d128_atan_values;

#endif

using atan_table = atan_table_imp<true>;

} //namespace atan_detail

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto atan_series(T x) noexcept;

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto atan_values(std::size_t idx) noexcept -> T;

template <> constexpr auto atan_series<decimal32> (decimal32 x) noexcept { return remez_series_result(x, atan_detail::atan_table::d32_coeffs); }
template <> constexpr auto atan_series<decimal64> (decimal64 x) noexcept { return remez_series_result(x, atan_detail::atan_table::d64_coeffs); }

template <>
constexpr auto atan_series<decimal128>(decimal128 x) noexcept
{
    // PadeApproximant[ArcTan[x]/x, {x, 0, {18, 18}}]
    // FullSimplify[%]
    // HornerForm[Numerator[Out[2]]]
    // HornerForm[Denominator[Out[2]]]

    const decimal128 x2 { x * x };

    const decimal128
        top
        {
                    decimal128 { UINT64_C(21427381364263875) }
            + x2 * (decimal128 { UINT64_C(91886788553059500) }
            + x2 * (decimal128 { UINT64_C(163675410390191700) }
            + x2 * (decimal128 { UINT64_C(156671838074852100) }
            + x2 * (decimal128 { UINT64_C(87054123957610810) }
            + x2 * (decimal128 { UINT64_C(28283323008669300) }
            + x2 * (decimal128 { UINT64_C(5134145876036100) }
            + x2 * (decimal128 { UINT64_C(463911017673180) }
            + x2 * (decimal128 { UINT64_C(16016872057515) }
            + x2 *  decimal128 { UINT64_C(90194313216) }))))))))
        };

    const decimal128
        bot
        {
                    decimal128 { UINT64_C(21427381364263875) }
            + x2 * (decimal128 { UINT64_C(99029249007814125) }
            + x2 * (decimal128 { UINT64_C(192399683786610300) }
            + x2 * (decimal128 { UINT64_C(204060270682768500) }
            + x2 * (decimal128 { UINT64_C(128360492848838250) }
            + x2 * (decimal128 { UINT64_C(48688462804731750) }
            + x2 * (decimal128 { UINT64_C(10819658401051500) }
            + x2 * (decimal128 { UINT64_C(1298359008126180) }
            + x2 * (decimal128 { UINT64_C(70562989572075) }
            + x2 *  decimal128 { UINT64_C(1120047453525) }))))))))
        };

    return (x * top) / bot;
}

template <> constexpr auto atan_values<decimal32> (std::size_t idx) noexcept -> decimal32  { return atan_detail::atan_table::d32_atan_values [idx]; }
template <> constexpr auto atan_values<decimal64> (std::size_t idx) noexcept -> decimal64  { return atan_detail::atan_table::d64_atan_values [idx]; }
template <> constexpr auto atan_values<decimal128>(std::size_t idx) noexcept -> decimal128 { return atan_detail::atan_table::d128_atan_values[idx]; }

} //namespace detail
} //namespace decimal
} //namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_IMPL_ATAN_IMPL_HPP
