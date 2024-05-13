// Copyright 2024 Matt Borland
// Copyright 2023 - 2024 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_LGAMMA_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_LGAMMA_IMPL_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/cmath/impl/taylor_series_result.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <array>
#include <cstddef>
#include <cstdint>
#endif

namespace boost {
namespace decimal {
namespace detail {

namespace lgamma_detail {

template <bool b>
struct lgamma_taylor_series_imp
{
    using d32_coeffs_t  = std::array<decimal32,  17>;
    using d64_coeffs_t  = std::array<decimal64,  27>;
    using d128_coeffs_t = std::array<decimal128, 25>;

    static constexpr d32_coeffs_t d32_coeffs =
    {{
         // Use a Taylor series expansion of the logarithm of the gamma function.
         // N[Series[Log[Gamma[x]], {x, 0, 18}], 19]
         //         log(1/x)
         //        -EulerGamma                                      // * x
         + decimal32 { UINT64_C(8224670334241132182), - 19 - 0 },   // x^2
         - decimal32 { UINT64_C(4006856343865314285), - 19 - 0 },   // x^3
         + decimal32 { UINT64_C(2705808084277845479), - 19 - 0 },   // x^4
         - decimal32 { UINT64_C(2073855510286739853), - 19 - 0 },   // x^5
         + decimal32 { UINT64_C(1695571769974081900), - 19 - 0 },   // x^6
         - decimal32 { UINT64_C(1440498967688461181), - 19 - 0 },   // x^7
         + decimal32 { UINT64_C(1255096695247430424), - 19 - 0 },   // x^8
         - decimal32 { UINT64_C(1113342658695646905), - 19 - 0 },   // x^9
         + decimal32 { UINT64_C(1000994575127818085), - 19 - 0 },   // x^10
         - decimal32 { UINT64_C(9095401714582904223), - 19 - 1 },   // x^11
         + decimal32 { UINT64_C(8335384054610900402), - 19 - 1 },   // x^12
         - decimal32 { UINT64_C(7693251641135219147), - 19 - 1 },   // x^13
         + decimal32 { UINT64_C(7143294629536133606), - 19 - 1 },   // x^14
         - decimal32 { UINT64_C(6666870588242046803), - 19 - 1 },   // x^15
         + decimal32 { UINT64_C(6250095514121304074), - 19 - 1 },   // x^16
         - decimal32 { UINT64_C(5882397865868458234), - 19 - 1 },   // x^17
         + decimal32 { UINT64_C(5555576762740361110), - 19 - 1 },   // x^18
     }};

    static constexpr d64_coeffs_t d64_coeffs =
    {{
         // Use a Taylor series expansion of the logarithm of the gamma function.
         // N[Series[Log[Gamma[x]], {x, 0, 28}], 19]
         //         log(1/x)
         //        -EulerGamma                                     // * x
           decimal64 { UINT64_C(8224670334241132182), -18 },   // x^2
         - decimal64 { UINT64_C(4006856343865314285), -18 },   // x^3
         + decimal64 { UINT64_C(2705808084277845479), -18 },   // x^4
         - decimal64 { UINT64_C(2073855510286739853), -18 },   // x^5
         + decimal64 { UINT64_C(1695571769974081900), -18 },   // x^6
         - decimal64 { UINT64_C(1440498967688461181), -18 },   // x^7
         + decimal64 { UINT64_C(1255096695247430424), -18 },   // x^8
         - decimal64 { UINT64_C(1113342658695646905), -18 },   // x^9
         + decimal64 { UINT64_C(1000994575127818085), -18 },   // x^10
         - decimal64 { UINT64_C(9095401714582904223), -19 },   // x^11
         + decimal64 { UINT64_C(8335384054610900402), -19 },   // x^12
         - decimal64 { UINT64_C(7693251641135219147), -19 },   // x^13
         + decimal64 { UINT64_C(7143294629536133606), -19 },   // x^14
         - decimal64 { UINT64_C(6666870588242046803), -19 },   // x^15
         + decimal64 { UINT64_C(6250095514121304074), -19 },   // x^16
         - decimal64 { UINT64_C(5882397865868458234), -19 },   // x^17
         + decimal64 { UINT64_C(5555576762740361110), -19 },   // x^18
         - decimal64 { UINT64_C(5263167937961666073), -19 },   // x^19
         + decimal64 { UINT64_C(5000004769810169364), -19 },   // x^20
         - decimal64 { UINT64_C(4761907033014222799), -19 },   // x^21
         + decimal64 { UINT64_C(4545455629320466944), -19 },   // x^22
         - decimal64 { UINT64_C(4347826605304025936), -19 },   // x^23
         + decimal64 { UINT64_C(4166666915034121047), -19 },   // x^24
         - decimal64 { UINT64_C(4000000119214014059), -19 },   // x^25
         + decimal64 { UINT64_C(3846153903467518571), -19 },   // x^26
         - decimal64 { UINT64_C(3703703731298932555), -19 },   // x^27
         + decimal64 { UINT64_C(3571428584733335803), -19 },   // x^28
    }};

    static constexpr d128_coeffs_t d128_coeffs =
    {{
         // Use a Taylor series expansion of the logarithm of the gamma function.
         // N[Series[Log[Gamma[x]], {x, 0, 26}], 32]
         //         log(1/x)
         //        -EulerGamma                             // * x
           decimal128 { UINT64_C(822'467'033'424'113'218), -18 },   // x^2
         - decimal128 { UINT64_C(400'685'634'386'531'428), -18 },   // x^3
         + decimal128 { UINT64_C(270'580'808'427'784'548), -18 },   // x^4
         - decimal128 { UINT64_C(207'385'551'028'673'985), -18 },   // x^5
         + decimal128 { UINT64_C(169'557'176'997'408'190), -18 },   // x^6
         - decimal128 { UINT64_C(144'049'896'768'846'118), -18 },   // x^7
         + decimal128 { UINT64_C(125'509'669'524'743'042), -18 },   // x^8
         - decimal128 { UINT64_C(111'334'265'869'564'690), -18 },   // x^9
         + decimal128 { UINT64_C(100'099'457'512'781'809), -18 },   // x^10
         - decimal128 { UINT64_C(909'540'171'458'290'422), -19 },   // x^11
         + decimal128 { UINT64_C(833'538'405'461'090'040), -19 },   // x^12
         - decimal128 { UINT64_C(769'325'164'113'521'915), -19 },   // x^13
         + decimal128 { UINT64_C(714'329'462'953'613'361), -19 },   // x^14
         - decimal128 { UINT64_C(666'687'058'824'204'680), -19 },   // x^15
         + decimal128 { UINT64_C(625'009'551'412'130'407), -19 },   // x^16
         - decimal128 { UINT64_C(588'239'786'586'845'823), -19 },   // x^17
         + decimal128 { UINT64_C(555'557'676'274'036'111), -19 },   // x^18
         - decimal128 { UINT64_C(526'316'793'796'166'607), -19 },   // x^19
         + decimal128 { UINT64_C(500'000'476'981'016'936), -19 },   // x^20
         - decimal128 { UINT64_C(476'190'703'301'422'280), -19 },   // x^21
         + decimal128 { UINT64_C(454'545'562'932'046'694), -19 },   // x^22
         - decimal128 { UINT64_C(434'782'660'530'402'594), -19 },   // x^23
         + decimal128 { UINT64_C(416'666'691'503'412'105), -19 },   // x^24
         - decimal128 { UINT64_C(400'000'011'921'401'406), -19 },   // x^25
         + decimal128 { UINT64_C(384'615'390'346'751'857), -19 },   // x^26
     }};
};

#if !(defined(__cpp_inline_variables) && __cpp_inline_variables >= 201606L) && (!defined(_MSC_VER) || _MSC_VER != 1900)

template <bool b>
constexpr typename lgamma_taylor_series_imp<b>::d32_coeffs_t lgamma_taylor_series_imp<b>::d32_coeffs;

template <bool b>
constexpr typename lgamma_taylor_series_imp<b>::d64_coeffs_t lgamma_taylor_series_imp<b>::d64_coeffs;

template <bool b>
constexpr typename lgamma_taylor_series_imp<b>::d128_coeffs_t lgamma_taylor_series_imp<b>::d128_coeffs;

#endif

} //namespace lgamma_detail

using lgamma_taylor_series_table = lgamma_detail::lgamma_taylor_series_imp<true>;

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto lgamma_taylor_series_expansion(T x) noexcept;

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

template <>
constexpr auto lgamma_taylor_series_expansion<decimal128>(decimal128 x) noexcept
{
    return taylor_series_result(x, lgamma_taylor_series_table::d128_coeffs);
}

} //namespace detail
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_IMPL_TGAMMA_IMPL_HPP
