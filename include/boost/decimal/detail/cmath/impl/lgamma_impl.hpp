// Copyright 2024 Matt Borland
// Copyright 2023 - 2024 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_LGAMMA_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_LGAMMA_IMPL_HPP

#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/cmath/impl/taylor_series_result.hpp>

#include <array>
#include <cstddef>
#include <cstdint>

namespace boost {
namespace decimal {
namespace detail {

namespace lgamma_detail {

template <bool b>
struct lgamma_taylor_series_imp
{

    // TODO: Decrease table for d32_coeffs
    static constexpr std::array<decimal32, 25> d32_coeffs =
    {{
         // Use a Taylor series expansion of the logarithm of the gamma function.
         // N[Series[Log[Gamma[x]], {x, 0, 26}], 32]
         //         log(1/x)
         //        -EulerGamma                             // * x
           decimal32 { UINT64_C(822'467'033'424'113'218), -18 },   // x^2
         - decimal32 { UINT64_C(400'685'634'386'531'428), -18 },   // x^3
         + decimal32 { UINT64_C(270'580'808'427'784'548), -18 },   // x^4
         - decimal32 { UINT64_C(207'385'551'028'673'985), -18 },   // x^5
         + decimal32 { UINT64_C(169'557'176'997'408'190), -18 },   // x^6
         - decimal32 { UINT64_C(144'049'896'768'846'118), -18 },   // x^7
         + decimal32 { UINT64_C(125'509'669'524'743'042), -18 },   // x^8
         - decimal32 { UINT64_C(111'334'265'869'564'690), -18 },   // x^9
         + decimal32 { UINT64_C(100'099'457'512'781'809), -18 },   // x^10
         - decimal32 { UINT64_C(909'540'171'458'290'422), -19 },   // x^11
         + decimal32 { UINT64_C(833'538'405'461'090'040), -19 },   // x^12
         - decimal32 { UINT64_C(769'325'164'113'521'915), -19 },   // x^13
         + decimal32 { UINT64_C(714'329'462'953'613'361), -19 },   // x^14
         - decimal32 { UINT64_C(666'687'058'824'204'680), -19 },   // x^15
         + decimal32 { UINT64_C(625'009'551'412'130'407), -19 },   // x^16
         - decimal32 { UINT64_C(588'239'786'586'845'823), -19 },   // x^17
         + decimal32 { UINT64_C(555'557'676'274'036'111), -19 },   // x^18
         - decimal32 { UINT64_C(526'316'793'796'166'607), -19 },   // x^19
         + decimal32 { UINT64_C(500'000'476'981'016'936), -19 },   // x^20
         - decimal32 { UINT64_C(476'190'703'301'422'280), -19 },   // x^21
         + decimal32 { UINT64_C(454'545'562'932'046'694), -19 },   // x^22
         - decimal32 { UINT64_C(434'782'660'530'402'594), -19 },   // x^23
         + decimal32 { UINT64_C(416'666'691'503'412'105), -19 },   // x^24
         - decimal32 { UINT64_C(400'000'011'921'401'406), -19 },   // x^25
         + decimal32 { UINT64_C(384'615'390'346'751'857), -19 },   // x^26
     }};

    static constexpr std::array<decimal64, 25> d64_coeffs =
    {{
         // Use a Taylor series expansion of the logarithm of the gamma function.
         // N[Series[Log[Gamma[x]], {x, 0, 26}], 32]
         //         log(1/x)
         //        -EulerGamma                             // * x
           decimal64 { UINT64_C(822'467'033'424'113'218), -18 },   // x^2
         - decimal64 { UINT64_C(400'685'634'386'531'428), -18 },   // x^3
         + decimal64 { UINT64_C(270'580'808'427'784'548), -18 },   // x^4
         - decimal64 { UINT64_C(207'385'551'028'673'985), -18 },   // x^5
         + decimal64 { UINT64_C(169'557'176'997'408'190), -18 },   // x^6
         - decimal64 { UINT64_C(144'049'896'768'846'118), -18 },   // x^7
         + decimal64 { UINT64_C(125'509'669'524'743'042), -18 },   // x^8
         - decimal64 { UINT64_C(111'334'265'869'564'690), -18 },   // x^9
         + decimal64 { UINT64_C(100'099'457'512'781'809), -18 },   // x^10
         - decimal64 { UINT64_C(909'540'171'458'290'422), -19 },   // x^11
         + decimal64 { UINT64_C(833'538'405'461'090'040), -19 },   // x^12
         - decimal64 { UINT64_C(769'325'164'113'521'915), -19 },   // x^13
         + decimal64 { UINT64_C(714'329'462'953'613'361), -19 },   // x^14
         - decimal64 { UINT64_C(666'687'058'824'204'680), -19 },   // x^15
         + decimal64 { UINT64_C(625'009'551'412'130'407), -19 },   // x^16
         - decimal64 { UINT64_C(588'239'786'586'845'823), -19 },   // x^17
         + decimal64 { UINT64_C(555'557'676'274'036'111), -19 },   // x^18
         - decimal64 { UINT64_C(526'316'793'796'166'607), -19 },   // x^19
         + decimal64 { UINT64_C(500'000'476'981'016'936), -19 },   // x^20
         - decimal64 { UINT64_C(476'190'703'301'422'280), -19 },   // x^21
         + decimal64 { UINT64_C(454'545'562'932'046'694), -19 },   // x^22
         - decimal64 { UINT64_C(434'782'660'530'402'594), -19 },   // x^23
         + decimal64 { UINT64_C(416'666'691'503'412'105), -19 },   // x^24
         - decimal64 { UINT64_C(400'000'011'921'401'406), -19 },   // x^25
         + decimal64 { UINT64_C(384'615'390'346'751'857), -19 },   // x^26
    }};

    // TODO: add d128_coeffs
};

#if !(defined(__cpp_inline_variables) && __cpp_inline_variables >= 201606L) && (!defined(_MSC_VER) || _MSC_VER != 1900)

template <bool b>
constexpr std::array<decimal32, 25> lgamma_taylor_series_imp<b>::d32_coeffs;

template <bool b>
constexpr std::array<decimal64, 25> lgamma_taylor_series_imp<b>::d64_coeffs;

#endif

} //namespace lgamma_detail

using lgamma_taylor_series_table = lgamma_detail::lgamma_taylor_series_imp<true>;

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto lgamma_taylor_series_expansion(T x) noexcept
{
    // LCOV_EXCL_START
    static_assert(!std::is_same<T, decimal128>::value, "Decimal128 has not been implemented");
    static_cast<void>(x);
    return T{1,0,true};
    // LCOV_EXCL_STOP
}

template <>
constexpr auto lgamma_taylor_series_expansion<decimal32>(decimal32 x) noexcept
{
    return taylor_series_result(x, lgamma_taylor_series_table::d32_coeffs);
}

template <>
constexpr auto lgamma_taylor_series_expansion<decimal64>(decimal64 x) noexcept
{
    return taylor_series_result(x, lgamma_taylor_series_table::d64_coeffs);
}

} //namespace detail
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_IMPL_TGAMMA_IMPL_HPP
