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
};

#if !(defined(__cpp_inline_variables) && __cpp_inline_variables >= 201606L) && (!defined(_MSC_VER) || _MSC_VER != 1900)

template <bool b>
constexpr std::array<decimal32, 6> sin_table_imp<b>::d32_coeffs;

template <bool b>
constexpr std::array<decimal64, 12> sin_table_imp<b>::d64_coeffs;

template <bool b>
constexpr std::array<decimal32_fast, 6> sin_table_imp<b>::d32_fast_coeffs;

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
constexpr auto sin_series_expansion<decimal128>(decimal128 x) noexcept
{
    const bool b_neg { signbit(x) };

    x = abs(x);

    // PadeApproximant[Sin[x], {x, 0, {14, 13}}]
    // FullSimplify[%]
    // HornerForm[Numerator[Out[2]]]
    // HornerForm[Denominator[Out[2]]]

    const decimal128 c0 { boost::decimal::detail::uint128 { UINT64_C(72470724512963),  UINT64_C(12010094287581601792) },  -1 };
    const decimal128 c1 { boost::decimal::detail::uint128 { UINT64_C(111100426260665), UINT64_C(12001293056709775360) },  -2, true };
    const decimal128 c2 { boost::decimal::detail::uint128 { UINT64_C(448976101608303), UINT64_C(8651619847551332352)  },  -4 };
    const decimal128 c3 { boost::decimal::detail::uint128 { UINT64_C(73569920121966),  UINT64_C(7922026052315602944)  },  -5, true };
    const decimal128 c4 { boost::decimal::detail::uint128 { UINT64_C(56791565109495),  UINT64_C(18025512837605806080) },  -7 };
    const decimal128 c5 { boost::decimal::detail::uint128 { UINT64_C(208944907042123), UINT64_C(1905626912845279232)  }, -10, true };
    const decimal128 c6 { boost::decimal::detail::uint128 { UINT64_C(301324799882787), UINT64_C(8861120840873566208)  }, -13 };

    const decimal128 d1 { boost::decimal::detail::uint128 { UINT64_C(96841145942737),  UINT64_C(12517245955660587008) },  -3 };
    const decimal128 d2 { boost::decimal::detail::uint128 { UINT64_C(64553072381691),  UINT64_C(13718792646062137344) },  -5 };
    const decimal128 d3 { boost::decimal::detail::uint128 { UINT64_C(279090388104865), UINT64_C(5072548100861788160)  },  -8 };
    const decimal128 d4 { boost::decimal::detail::uint128 { UINT64_C(84086452204639),  UINT64_C(9046779044634853376)  }, -10 };
    const decimal128 d5 { boost::decimal::detail::uint128 { UINT64_C(171178955723736), UINT64_C(18053324302671642624) }, -13 };
    const decimal128 d6 { boost::decimal::detail::uint128 { UINT64_C(189091057352841), UINT64_C(2258222749986258944)  }, -16 };

    const decimal128 x2 { x * x };

    const decimal128 top { x * (c0 + x2 * (c1 + x2 * (c2 + x2 * (c3 + x2 * (c4 + x2 * (c5 + x2 *  c6)))))) };
    const decimal128 bot { c0 + x2 * (d1 + x2 * (d2 + x2 * (d3 + x2 * (d4 + x2 * (d5 + x2 *  d6))))) };

    const decimal128 result { top / bot };

    return b_neg ? -result : result;
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif
