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

template <bool b>
struct lgamma_laurent_series_imp
{
    static constexpr std::array<decimal32, 26> d32_coeffs = {{
        // Use a Laurent infinite-series expansion of the logarithm of the
        // gamma function divided by the square root.
        // N[Series[Log[Gamma[x]], {x, Infinity, 26}], 32]
        // log( e^-x * x^x * sqrt(x) * [Series below] )
        // See also Wolfram Alpha(R): https://www.wolframalpha.com/input?i=Series%5BLog%5BGamma%5Bx%5D%5D%2C+%7Bx%2C+Infinity%2C+4%7D%5D
          decimal32 { UINT64_C(250'662'827'463'100'050), + 1 - 18 }, // * x^-1
        + decimal32 { UINT64_C(208'885'689'552'583'375), - 0 - 18 }, // * x^-2
        + decimal32 { UINT64_C(870'357'039'802'430'730), - 2 - 18 }, // * x^-3
        - decimal32 { UINT64_C(672'109'047'402'988'175), - 2 - 18 }, // * x^-4
        - decimal32 { UINT64_C(575'201'238'110'171'235), - 3 - 18 }, // * x^-5
        + decimal32 { UINT64_C(196'529'488'158'320'306), - 2 - 18 }, // * x^-6
        + decimal32 { UINT64_C(174'782'521'204'559'121), - 3 - 18 }, // * x^-7
        - decimal32 { UINT64_C(148'434'113'515'827'614), - 2 - 18 }, // * x^-8
        - decimal32 { UINT64_C(129'637'573'211'255'432), - 3 - 18 }, // * x^-9
        + decimal32 { UINT64_C(210'431'122'975'320'637), - 2 - 18 }, // * x^-10
        + decimal32 { UINT64_C(180'599'945'655'550'436), - 3 - 18 }, // * x^-11
        - decimal32 { UINT64_C(479'878'567'054'634'606), - 2 - 18 }, // * x^-12
        - decimal32 { UINT64_C(407'367'859'381'525'183), - 3 - 18 }, // * x^-13
        + decimal32 { UINT64_C(160'508'503'319'445'960), - 1 - 18 }, // * x^-14
        + decimal32 { UINT64_C(135'399'228'015'909'411), - 2 - 18 }, // * x^-15
        - decimal32 { UINT64_C(740'154'212'684'273'819), - 1 - 18 }, // * x^-16
        - decimal32 { UINT64_C(622'080'867'880'877'866), - 2 - 18 }, // * x^-17
        + decimal32 { UINT64_C(450'040'333'856'250'984), - 0 - 18 }, // * x^-18
        + decimal32 { UINT64_C(377'400'786'521'707'440), - 1 - 18 }, // * x^-19
        - decimal32 { UINT64_C(348'872'797'304'123'310), + 1 - 18 }, // * x^-20
        - decimal32 { UINT64_C(292'138'192'227'179'792), - 0 - 18 }, // * x^-21
        + decimal32 { UINT64_C(335'837'691'649'553'084), + 2 - 18 }, // * x^-22
        + decimal32 { UINT64_C(280'944'016'052'174'395), + 1 - 18 }, // * x^-23
        - decimal32 { UINT64_C(393'042'854'585'987'930), + 3 - 18 }, // * x^-24
        - decimal32 { UINT64_C(328'565'400'725'242'178), + 2 - 18 }, // * x^-25
        + decimal32 { UINT64_C(549'592'170'046'323'711), + 4 - 18 }, // * x^-26
    }};
    
    static constexpr std::array<decimal64, 26> d64_coeffs = {{
        // Use a Laurent infinite-series expansion of the logarithm of the
        // gamma function divided by the square root.
        // N[Series[Log[Gamma[x]], {x, Infinity, 26}], 32]
        // log( e^-x * x^x * sqrt(x) * [Series below] )
        // See also Wolfram Alpha(R): https://www.wolframalpha.com/input?i=Series%5BLog%5BGamma%5Bx%5D%5D%2C+%7Bx%2C+Infinity%2C+4%7D%5D
          decimal64 { UINT64_C(250'662'827'463'100'050), + 1 - 18 }, // * x^-1
        + decimal64 { UINT64_C(208'885'689'552'583'375), - 0 - 18 }, // * x^-2
        + decimal64 { UINT64_C(870'357'039'802'430'730), - 2 - 18 }, // * x^-3
        - decimal64 { UINT64_C(672'109'047'402'988'175), - 2 - 18 }, // * x^-4
        - decimal64 { UINT64_C(575'201'238'110'171'235), - 3 - 18 }, // * x^-5
        + decimal64 { UINT64_C(196'529'488'158'320'306), - 2 - 18 }, // * x^-6
        + decimal64 { UINT64_C(174'782'521'204'559'121), - 3 - 18 }, // * x^-7
        - decimal64 { UINT64_C(148'434'113'515'827'614), - 2 - 18 }, // * x^-8
        - decimal64 { UINT64_C(129'637'573'211'255'432), - 3 - 18 }, // * x^-9
        + decimal64 { UINT64_C(210'431'122'975'320'637), - 2 - 18 }, // * x^-10
        + decimal64 { UINT64_C(180'599'945'655'550'436), - 3 - 18 }, // * x^-11
        - decimal64 { UINT64_C(479'878'567'054'634'606), - 2 - 18 }, // * x^-12
        - decimal64 { UINT64_C(407'367'859'381'525'183), - 3 - 18 }, // * x^-13
        + decimal64 { UINT64_C(160'508'503'319'445'960), - 1 - 18 }, // * x^-14
        + decimal64 { UINT64_C(135'399'228'015'909'411), - 2 - 18 }, // * x^-15
        - decimal64 { UINT64_C(740'154'212'684'273'819), - 1 - 18 }, // * x^-16
        - decimal64 { UINT64_C(622'080'867'880'877'866), - 2 - 18 }, // * x^-17
        + decimal64 { UINT64_C(450'040'333'856'250'984), - 0 - 18 }, // * x^-18
        + decimal64 { UINT64_C(377'400'786'521'707'440), - 1 - 18 }, // * x^-19
        - decimal64 { UINT64_C(348'872'797'304'123'310), + 1 - 18 }, // * x^-20
        - decimal64 { UINT64_C(292'138'192'227'179'792), - 0 - 18 }, // * x^-21
        + decimal64 { UINT64_C(335'837'691'649'553'084), + 2 - 18 }, // * x^-22
        + decimal64 { UINT64_C(280'944'016'052'174'395), + 1 - 18 }, // * x^-23
        - decimal64 { UINT64_C(393'042'854'585'987'930), + 3 - 18 }, // * x^-24
        - decimal64 { UINT64_C(328'565'400'725'242'178), + 2 - 18 }, // * x^-25
        + decimal64 { UINT64_C(549'592'170'046'323'711), + 4 - 18 }, // * x^-26
    }};
};

#if !(defined(__cpp_inline_variables) && __cpp_inline_variables >= 201606L) && (!defined(_MSC_VER) || _MSC_VER != 1900)

template <bool b>
constexpr std::array<decimal32, 25> lgamma_taylor_series_imp<b>::d32_coeffs;

template <bool b>
constexpr std::array<decimal64, 25> lgamma_taylor_series_imp<b>::d64_coeffs;

template <bool b>
constexpr std::array<decimal32, 26> lgamma_laurent_series_imp<b>::d32_coeffs;

template <bool b>
constexpr std::array<decimal64, 26> lgamma_laurent_series_imp<b>::d64_coeffs;

#endif

} //namespace lgamma_detail

using lgamma_taylor_series_table = lgamma_detail::lgamma_taylor_series_imp<true>;
using lgamma_laurent_series_table = lgamma_detail::lgamma_laurent_series_imp<true>;

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

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto lgamma_laurent_series_expansion(T x) noexcept
{
    // LCOV_EXCL_START
    static_assert(!std::is_same<T, decimal128>::value, "Decimal128 has not been implemented");
    static_cast<void>(x);
    return T{1,0,true};
    // LCOV_EXCL_STOP
}

template <>
constexpr auto lgamma_laurent_series_expansion<decimal32>(decimal32 x) noexcept
{
    return taylor_series_result(x, lgamma_laurent_series_table::d32_coeffs);
}

template <>
constexpr auto lgamma_laurent_series_expansion<decimal64>(decimal64 x) noexcept
{
    return taylor_series_result(x, lgamma_laurent_series_table::d64_coeffs);
}

} //namespace detail
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_IMPL_TGAMMA_IMPL_HPP
