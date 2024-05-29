// Copyright 2023-2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_SIN_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_SIN_IMPL_HPP

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

namespace sin_detail {

template <bool b>
struct sin_table_imp {

    // 5th Degree Remez Polynomial
    // Estimated max error: 6.0855992690454531e-8
    static constexpr std::array<decimal32, 6> d32_coeffs =
    {{
        decimal32 {UINT64_C(76426704684128569), -19},
        decimal32 {UINT64_C(8163484279370784), -19},
        decimal32 {UINT64_C(16704305092800237), -17, true},
        decimal32 {UINT64_C(74622903795259856), -21},
        decimal32 {UINT64_C(9999946918542727), -16},
        decimal32 {UINT64_C(60055992690454536), -24}
     }};

    static constexpr std::array<decimal32_fast, 6> d32_fast_coeffs =
    {{
         decimal32_fast {UINT64_C(76426704684128569), -19},
         decimal32_fast {UINT64_C(8163484279370784), -19},
         decimal32_fast {UINT64_C(16704305092800237), -17, true},
         decimal32_fast {UINT64_C(74622903795259856), -21},
         decimal32_fast {UINT64_C(9999946918542727), -16},
         decimal32_fast {UINT64_C(60055992690454536), -24}
     }};

    // 11th Degree Remez Polynomial
    // Estimated max error: 5.2301715421592162270336342660001217e-18
    static constexpr std::array<decimal64, 12> d64_coeffs =
    {{
        decimal64 {UINT64_C(2306518628003855678), -26, true},
        decimal64 {UINT64_C(5453073257634027470), -27, true},
        decimal64 {UINT64_C(2762996699568163845), -24},
        decimal64 {UINT64_C(5023027013521532307), -27, true},
        decimal64 {UINT64_C(1984096861383546182), -22, true},
        decimal64 {UINT64_C(1026912296061211491), -27, true},
        decimal64 {UINT64_C(8333333562151404340), -21},
        decimal64 {UINT64_C(3217043986646625014), -29, true},
        decimal64 {UINT64_C(1666666666640042905), -19, true},
        decimal64 {UINT64_C(1135995742940218051), -31, true},
        decimal64 {UINT64_C(1000000000000001896), -18},
        decimal64 {UINT64_C(5230171542159216227), -36, true}
    }};

    static constexpr std::array<decimal64_fast, 12> d64_fast_coeffs =
    {{
         decimal64_fast {UINT64_C(2306518628003855678), -26, true},
         decimal64_fast {UINT64_C(5453073257634027470), -27, true},
         decimal64_fast {UINT64_C(2762996699568163845), -24},
         decimal64_fast {UINT64_C(5023027013521532307), -27, true},
         decimal64_fast {UINT64_C(1984096861383546182), -22, true},
         decimal64_fast {UINT64_C(1026912296061211491), -27, true},
         decimal64_fast {UINT64_C(8333333562151404340), -21},
         decimal64_fast {UINT64_C(3217043986646625014), -29, true},
         decimal64_fast {UINT64_C(1666666666640042905), -19, true},
         decimal64_fast {UINT64_C(1135995742940218051), -31, true},
         decimal64_fast {UINT64_C(1000000000000001896), -18},
         decimal64_fast {UINT64_C(5230171542159216227), -36, true}
     }};

    // 20th Degree Remez Polynomial
    // Estimated max error: 5.1424960359035132189835410157248994e-35
    static constexpr std::array<decimal128, 21> d128_coeffs =
    {{
        decimal128 {uint128{UINT64_C(85106305874239), UINT64_C(16929064868128953896)}, -52},
        decimal128 {uint128{UINT64_C(477768502693008), UINT64_C(6230918648367889942)}, -51, true},
        decimal128 {uint128{UINT64_C(75154315253822), UINT64_C(13833706134005544038)}, -51},
        decimal128 {uint128{UINT64_C(152287788904364), UINT64_C(1676311666321267536)}, -48},
        decimal128 {uint128{UINT64_C(144214752508825), UINT64_C(2528999524738537100)}, -51},
        decimal128 {uint128{UINT64_C(414554872884779), UINT64_C(15931857976032858760)}, -46, true},
        decimal128 {uint128{UINT64_C(90156974414685), UINT64_C(14279793832049340120)}, -51},
        decimal128 {uint128{UINT64_C(87056250588597), UINT64_C(16057379721599586648)}, -43},
        decimal128 {uint128{UINT64_C(210637815468175), UINT64_C(7636003443272702110)}, -52},
        decimal128 {uint128{UINT64_C(135807751684903), UINT64_C(10512681453991690152)}, -41, true},
        decimal128 {uint128{UINT64_C(189273977706970), UINT64_C(1683985612936918840)}, -53},
        decimal128 {uint128{UINT64_C(149388526852609), UINT64_C(16550971142245619806)}, -39},
        decimal128 {uint128{UINT64_C(62386708229102), UINT64_C(17615400106141663882)}, -54},
        decimal128 {uint128{UINT64_C(107559739333879), UINT64_C(7530156268905159646)}, -37, true},
        decimal128 {uint128{UINT64_C(66059193820724), UINT64_C(9642511815583692046)}, -56},
        decimal128 {uint128{UINT64_C(451750905202293), UINT64_C(9484757435910730332)}, -36},
        decimal128 {uint128{UINT64_C(170869449273575), UINT64_C(3295407555488151196)}, -59},
        decimal128 {uint128{UINT64_C(90350181040458), UINT64_C(12964998083139403502)}, -34, true},
        decimal128 {uint128{UINT64_C(58541029533765), UINT64_C(17525845691359836026)}, -62},
        decimal128 {uint128{UINT64_C(542101086242752), UINT64_C(4003012203950106990)}, -34},
        decimal128 {uint128{UINT64_C(278775268706234), UINT64_C(3358921116451750765)}, -68}
    }};
};

#if !(defined(__cpp_inline_variables) && __cpp_inline_variables >= 201606L) && (!defined(_MSC_VER) || _MSC_VER != 1900)

template <bool b>
constexpr std::array<decimal32, 6> sin_table_imp<b>::d32_coeffs;

template <bool b>
constexpr std::array<decimal64, 12> sin_table_imp<b>::d64_coeffs;

template <bool b>
constexpr std::array<decimal128, 21> sin_table_imp<b>::d128_coeffs;

template <bool b>
constexpr std::array<decimal32_fast, 6> sin_table_imp<b>::d32_fast_coeffs;

template <bool b>
constexpr std::array<decimal64_fast, 12> sin_table_imp<b>::d64_fast_coeffs;

#endif

using sin_table = sin_table_imp<true>;

} //namespace sin_detail

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto sin_series_expansion(T x) noexcept;

template <>
constexpr auto sin_series_expansion<decimal32>(decimal32 x) noexcept
{
    const auto b_neg = signbit(x);
    x = abs(x);
    auto result = remez_series_result(x, sin_detail::sin_table::d32_coeffs);
    return b_neg ? -result : result;
}

template <>
constexpr auto sin_series_expansion<decimal32_fast>(decimal32_fast x) noexcept
{
    const auto b_neg = signbit(x);
    x = abs(x);
    auto result = remez_series_result(x, sin_detail::sin_table::d32_fast_coeffs);
    return b_neg ? -result : result;
}

template <>
constexpr auto sin_series_expansion<decimal64>(decimal64 x) noexcept
{
    const auto b_neg = signbit(x);
    x = abs(x);
    auto result = remez_series_result(x, sin_detail::sin_table::d64_coeffs);
    return b_neg ? -result : result;
}

template <>
constexpr auto sin_series_expansion<decimal64_fast>(decimal64_fast x) noexcept
{
    const auto b_neg = signbit(x);
    x = abs(x);
    auto result = remez_series_result(x, sin_detail::sin_table::d64_fast_coeffs);
    return b_neg ? -result : result;
}

template <>
constexpr auto sin_series_expansion<decimal128>(decimal128 x) noexcept
{
    const auto b_neg = signbit(x);
    x = abs(x);
    auto result = remez_series_result(x, sin_detail::sin_table::d128_coeffs);
    return b_neg ? -result : result;
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif
