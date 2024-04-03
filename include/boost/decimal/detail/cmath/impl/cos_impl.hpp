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

    // 20th Degree Remez Polynomial from 0 to pi / 4
    // Estimated max error: 2.1310510195548626186539810165524781e-35
    static constexpr std::array<decimal128, 21> d128_coeffs =
    {{
        decimal128 {uint128{UINT64_C(205464805604747), UINT64_C(1103437048276783858)}, -52},
        decimal128 {uint128{UINT64_C(88395173126016), UINT64_C(16033763930510860544)}, -52},
        decimal128 {uint128{UINT64_C(84906215169376), UINT64_C(15713701775139901874)}, -49, true},
        decimal128 {uint128{UINT64_C(406502864446184), UINT64_C(12823522983377384156)}, -52},
        decimal128 {uint128{UINT64_C(259090944919015), UINT64_C(2392820714740683920)}, -47},
        decimal128 {uint128{UINT64_C(467445711741371), UINT64_C(4983292921389624904)}, -52},
        decimal128 {uint128{UINT64_C(62183039812775), UINT64_C(15652271656899615679)}, -44, true},
        decimal128 {uint128{UINT64_C(184508989294410), UINT64_C(1375137197219348330)}, -52},
        decimal128 {uint128{UINT64_C(113173126395461), UINT64_C(16339984202390313234)}, -42},
        decimal128 {uint128{UINT64_C(274884181093086), UINT64_C(12747689940557963034)}, -53},
        decimal128 {uint128{UINT64_C(149388526852617), UINT64_C(12302422570283469338)}, -40, true},
        decimal128 {uint128{UINT64_C(154088679459876), UINT64_C(3924311363127714460)}, -54},
        decimal128 {uint128{UINT64_C(134449674167349), UINT64_C(4753674936935436426)}, -33},
        decimal128 {uint128{UINT64_C(299007263162206), UINT64_C(7798573768093066264)}, -56},
        decimal128 {uint128{UINT64_C(75291817533715), UINT64_C(10804169962871218270)}, -36, true},
        decimal128 {uint128{UINT64_C(166117873118141), UINT64_C(15619656560639581524)}, -58},
        decimal128 {uint128{UINT64_C(225875452601146), UINT64_C(13965751132838711524)}, -35},
        decimal128 {uint128{UINT64_C(177440011694387), UINT64_C(4853507633156477618)}, -61},
        decimal128 {uint128{UINT64_C(271050543121376), UINT64_C(2001506101975100694)}, -34, true},
        decimal128 {uint128{UINT64_C(129186594797812), UINT64_C(16941950919815074018)}, -65},
        decimal128 {uint128{UINT64_C(54210108624275), UINT64_C(4089650035136921600)}, -33}
    }};
};

#if !(defined(__cpp_inline_variables) && __cpp_inline_variables >= 201606L) && (!defined(_MSC_VER) || _MSC_VER != 1900)

template <bool b>
constexpr std::array<decimal32, 9> cos_table_imp<b>::d32_coeffs;

template <bool b>
constexpr std::array<decimal64, 13> cos_table_imp<b>::d64_coeffs;

template <bool b>
constexpr std::array<decimal128, 21> cos_table_imp<b>::d128_coeffs;

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
constexpr auto cos_series_expansion<decimal64>(decimal64 x) noexcept
{
    return remez_series_result(x, cos_detail::cos_table::d64_coeffs);
}

template <>
constexpr auto cos_series_expansion<decimal128>(decimal128 x) noexcept
{
    return remez_series_result(x, cos_detail::cos_table::d128_coeffs);
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_IMPL_COS_IMPL_HPP
