// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_COS_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_COS_IMPL_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/cmath/impl/remez_series_result.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/config.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <type_traits>
#include <cstdint>
#endif

namespace boost {
namespace decimal {
namespace detail {

namespace cos_detail {

template <bool b>
struct cos_table_imp
{
    // 8th Degree Remez Polynomial from 0 to pi / 4
    // Estimated max error: 4.321978891364628e-14
    static constexpr std::array<decimal32, 9> d32_coeffs =
    {{
        decimal32 {UINT64_C(22805960529562646), -21},
        decimal32 {UINT64_C(39171880037888081), -22},
        decimal32 {UINT64_C(1392392773950284), -18, true},
        decimal32 {UINT64_C(17339629614857501), -22},
        decimal32 {UINT64_C(41666173896377827), -18},
        decimal32 {UINT64_C(77764646000512304), -24},
        decimal32 {UINT64_C(50000000610949535), -17, true},
        decimal32 {UINT64_C(18421494272283811), -26},
        decimal32 {UINT64_C(99999999999908662), -17}
    }};

    static constexpr std::array<decimal32_fast, 9> d32_fast_coeffs =
    {{
         decimal32_fast {UINT64_C(22805960529562646), -21},
         decimal32_fast {UINT64_C(39171880037888081), -22},
         decimal32_fast {UINT64_C(1392392773950284), -18, true},
         decimal32_fast {UINT64_C(17339629614857501), -22},
         decimal32_fast {UINT64_C(41666173896377827), -18},
         decimal32_fast {UINT64_C(77764646000512304), -24},
         decimal32_fast {UINT64_C(50000000610949535), -17, true},
         decimal32_fast {UINT64_C(18421494272283811), -26},
         decimal32_fast {UINT64_C(99999999999908662), -17}
     }};

    // 12th Degree Remez Polynomial from 0 to pi / 4
    // Estimated max error: 7.911867233315355155595617164843665e-20
    static constexpr std::array<decimal64, 13> d64_coeffs =
    {{
        decimal64 {UINT64_C(1922641020040661424), -27},
        decimal64 {UINT64_C(4960385936049718134), -28},
        decimal64 {UINT64_C(2763064713566851512), -25, true},
        decimal64 {UINT64_C(6633276621376137827), -28},
        decimal64 {UINT64_C(2480119161297283187), -23},
        decimal64 {UINT64_C(1600210781837650114), -28},
        decimal64 {UINT64_C(1388888932852646133), -21, true},
        decimal64 {UINT64_C(8054772849254568869), -30},
        decimal64 {UINT64_C(4166666666572238908), -20},
        decimal64 {UINT64_C(6574164404618517322), -32},
        decimal64 {UINT64_C(5000000000000023748), -19, true},
        decimal64 {UINT64_C(3367952043014273196), -35},
        decimal64 {UINT64_C(9999999999999999999), -19}
    }};

    static constexpr std::array<decimal64_fast, 13> d64_fast_coeffs =
    {{
         decimal64_fast {UINT64_C(1922641020040661424), -27},
         decimal64_fast {UINT64_C(4960385936049718134), -28},
         decimal64_fast {UINT64_C(2763064713566851512), -25, true},
         decimal64_fast {UINT64_C(6633276621376137827), -28},
         decimal64_fast {UINT64_C(2480119161297283187), -23},
         decimal64_fast {UINT64_C(1600210781837650114), -28},
         decimal64_fast {UINT64_C(1388888932852646133), -21, true},
         decimal64_fast {UINT64_C(8054772849254568869), -30},
         decimal64_fast {UINT64_C(4166666666572238908), -20},
         decimal64_fast {UINT64_C(6574164404618517322), -32},
         decimal64_fast {UINT64_C(5000000000000023748), -19, true},
         decimal64_fast {UINT64_C(3367952043014273196), -35},
         decimal64_fast {UINT64_C(9999999999999999999), -19}
     }};
};

#if !(defined(__cpp_inline_variables) && __cpp_inline_variables >= 201606L) && (!defined(_MSC_VER) || _MSC_VER != 1900)

template <bool b>
constexpr std::array<decimal32, 9> cos_table_imp<b>::d32_coeffs;

template <bool b>
constexpr std::array<decimal64, 13> cos_table_imp<b>::d64_coeffs;

template <bool b>
constexpr std::array<decimal32_fast, 9> cos_table_imp<b>::d32_fast_coeffs;

template <bool b>
constexpr std::array<decimal64_fast, 13> cos_table_imp<b>::d64_fast_coeffs;

#endif

using cos_table = cos_table_imp<true>;

} // namespace cos_detail

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto cos_series_expansion(T x) noexcept;

template <>
constexpr auto cos_series_expansion<decimal32>(decimal32 x) noexcept
{
    return remez_series_result(x, cos_detail::cos_table::d32_coeffs);
}

template <>
constexpr auto cos_series_expansion<decimal32_fast>(decimal32_fast x) noexcept
{
    return remez_series_result(x, cos_detail::cos_table::d32_fast_coeffs);
}

template <>
constexpr auto cos_series_expansion<decimal64>(decimal64 x) noexcept
{
    return remez_series_result(x, cos_detail::cos_table::d64_coeffs);
}

template <>
constexpr auto cos_series_expansion<decimal64_fast>(decimal64_fast x) noexcept
{
    return remez_series_result(x, cos_detail::cos_table::d64_fast_coeffs);
}

template <>
constexpr auto cos_series_expansion<decimal128>(decimal128 x) noexcept
{
    // PadeApproximant[Cos[x], {x, 0, {14, 14}}]
    // FullSimplify[%]
    // HornerForm[Numerator[Out[2]]]
    // HornerForm[Denominator[Out[2]]]

    const decimal128 c0 { boost::decimal::detail::uint128 { UINT64_C(307807346375396), UINT64_C(9191352932158695424)  },  3 };
    const decimal128 c1 { boost::decimal::detail::uint128 { UINT64_C(149996550055690), UINT64_C(222763958071016960)   },  3, true };
    const decimal128 c2 { boost::decimal::detail::uint128 { UINT64_C(108967212479807), UINT64_C(3937477076487471608)  },  2 };
    const decimal128 c3 { boost::decimal::detail::uint128 { UINT64_C(277096228519262), UINT64_C(6277888927557284608)  },  0, true };
    const decimal128 c4 { boost::decimal::detail::uint128 { UINT64_C(319580269604048), UINT64_C(10708241405247058432) }, -2 };
    const decimal128 c5 { boost::decimal::detail::uint128 { UINT64_C(183739194803716), UINT64_C(9003931728965394944)  }, -4, true };
    const decimal128 c6 { boost::decimal::detail::uint128 { UINT64_C(518817586019902), UINT64_C(14598542072727738368) }, -7 };
    const decimal128 c7 { boost::decimal::detail::uint128 { UINT64_C(58205916937364),  UINT64_C(13388002334603019776) }, -9, true };

    const decimal128 d1 { boost::decimal::detail::uint128 { UINT64_C(390712313200823), UINT64_C(13016137105513388032) },   1 };
    const decimal128 d2 { boost::decimal::detail::uint128 { UINT64_C(249767150099857), UINT64_C(14534865724066009088) },  -1 };
    const decimal128 d3 { boost::decimal::detail::uint128 { UINT64_C(105535117882474), UINT64_C(16245151810017622016) },  -3 };
    const decimal128 d4 { boost::decimal::detail::uint128 { UINT64_C(322928599993793), UINT64_C(8055050913586880512)  },  -6 };
    const decimal128 d5 { boost::decimal::detail::uint128 { UINT64_C(72777849685460),  UINT64_C(10172723920765296640) },  -8 };
    const decimal128 d6 { boost::decimal::detail::uint128 { UINT64_C(114133059907344), UINT64_C(3036923607254532096)  }, -11 };
    const decimal128 d7 { boost::decimal::detail::uint128 { UINT64_C(98470690251347),  UINT64_C(1521187190289973248)  }, -14 };

    const decimal128 x2 { x * x };

    const decimal128 top { c0 + x2 * (c1 + x2 * (c2 + x2 * (c3 + x2 * (c4 + x2 * (c5 + x2 * (c6 + x2 *  c7)))))) };
    const decimal128 bot { c0 + x2 * (d1 + x2 * (d2 + x2 * (d3 + x2 * (d4 + x2 * (d5 + x2 * (d6 + x2 *  d7)))))) };

    return decimal128 { top / bot };
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_IMPL_COS_IMPL_HPP
