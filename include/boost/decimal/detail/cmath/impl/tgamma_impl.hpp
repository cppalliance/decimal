// Copyright 2024 Matt Borland
// Copyright 2023 - 2024 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_TGAMMA_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_TGAMMA_IMPL_HPP

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

namespace tgamma_detail {

template <bool b>
struct tgamma_table_imp
{
    static constexpr std::array<decimal32, 15> d32_coeffs =
    {{
        // N[Series[1/Gamma[z], {z, 0, 16}], 24]
        +::boost::decimal::decimal32 { UINT64_C(5'772'156'649'015'328'606), - 19 - 0 }, // * z^2
        -::boost::decimal::decimal32 { UINT64_C(6'558'780'715'202'538'811), - 19 - 0 }, // * z^3
        -::boost::decimal::decimal32 { UINT64_C(4'200'263'503'409'523'553), - 19 - 1 }, // * z^4
        +::boost::decimal::decimal32 { UINT64_C(1'665'386'113'822'914'895), - 19 - 0 }, // * z^5
        -::boost::decimal::decimal32 { UINT64_C(4'219'773'455'554'433'675), - 19 - 1 }, // * z^6
        -::boost::decimal::decimal32 { UINT64_C(9'621'971'527'876'973'562), - 19 - 2 }, // * z^7
        +::boost::decimal::decimal32 { UINT64_C(7'218'943'246'663'099'542), - 19 - 2 }, // * z^8
        -::boost::decimal::decimal32 { UINT64_C(1'165'167'591'859'065'112), - 19 - 2 }, // * z^9
        -::boost::decimal::decimal32 { UINT64_C(2'152'416'741'149'509'728), - 19 - 3 }, // * z^10
        +::boost::decimal::decimal32 { UINT64_C(1'280'502'823'881'161'862), - 19 - 3 }, // * z^11
        -::boost::decimal::decimal32 { UINT64_C(2'013'485'478'078'823'866), - 19 - 4 }, // * z^12
        -::boost::decimal::decimal32 { UINT64_C(1'250'493'482'142'670'657), - 19 - 5 }, // * z^13
        +::boost::decimal::decimal32 { UINT64_C(1'133'027'231'981'695'882), - 19 - 5 }, // * z^14
        -::boost::decimal::decimal32 { UINT64_C(2'056'338'416'977'607'103), - 19 - 6 }, // * z^15
        +::boost::decimal::decimal32 { UINT64_C(6'116'095'104'481'415'818), - 19 - 8 }, // * z^16
    }};

    static constexpr std::array<decimal64, 36> d64_coeffs =
    {{
         // N[Series[1/Gamma[z], {z, 0, 27}], 24]
         +::boost::decimal::decimal64 { UINT64_C(5'772'156'649'015'328'606), - 19 - 0  }, // * z^2
         -::boost::decimal::decimal64 { UINT64_C(6'558'780'715'202'538'811), - 19 - 0  }, // * z^3
         -::boost::decimal::decimal64 { UINT64_C(4'200'263'503'409'523'553), - 19 - 1  }, // * z^4
         +::boost::decimal::decimal64 { UINT64_C(1'665'386'113'822'914'895), - 19 - 0  }, // * z^5
         -::boost::decimal::decimal64 { UINT64_C(4'219'773'455'554'433'675), - 19 - 1  }, // * z^6
         -::boost::decimal::decimal64 { UINT64_C(9'621'971'527'876'973'562), - 19 - 2  }, // * z^7
         +::boost::decimal::decimal64 { UINT64_C(7'218'943'246'663'099'542), - 19 - 2  }, // * z^8
         -::boost::decimal::decimal64 { UINT64_C(1'165'167'591'859'065'112), - 19 - 2  }, // * z^9
         -::boost::decimal::decimal64 { UINT64_C(2'152'416'741'149'509'728), - 19 - 3  }, // * z^10
         +::boost::decimal::decimal64 { UINT64_C(1'280'502'823'881'161'862), - 19 - 3  }, // * z^11
         -::boost::decimal::decimal64 { UINT64_C(2'013'485'478'078'823'866), - 19 - 4  }, // * z^12
         -::boost::decimal::decimal64 { UINT64_C(1'250'493'482'142'670'657), - 19 - 5  }, // * z^13
         +::boost::decimal::decimal64 { UINT64_C(1'133'027'231'981'695'882), - 19 - 5  }, // * z^14
         -::boost::decimal::decimal64 { UINT64_C(2'056'338'416'977'607'103), - 19 - 6  }, // * z^15
         +::boost::decimal::decimal64 { UINT64_C(6'116'095'104'481'415'818), - 19 - 8  }, // * z^16
         +::boost::decimal::decimal64 { UINT64_C(5'002'007'644'469'222'930), - 19 - 8  }, // * z^17
         -::boost::decimal::decimal64 { UINT64_C(1'181'274'570'487'020'145), - 19 - 8  }, // * z^18
         +::boost::decimal::decimal64 { UINT64_C(1'043'426'711'691'100'510), - 19 - 9  }, // * z^19
         +::boost::decimal::decimal64 { UINT64_C(7'782'263'439'905'071'254), - 19 - 11 }, // * z^20
         -::boost::decimal::decimal64 { UINT64_C(3'696'805'618'642'205'708), - 19 - 11 }, // * z^21
         +::boost::decimal::decimal64 { UINT64_C(5'100'370'287'454'475'979), - 19 - 12 }, // * z^22
         -::boost::decimal::decimal64 { UINT64_C(2'058'326'053'566'506'783), - 19 - 13 }, // * z^23
         -::boost::decimal::decimal64 { UINT64_C(5'348'122'539'423'017'982), - 19 - 14 }, // * z^24
         +::boost::decimal::decimal64 { UINT64_C(1'226'778'628'238'260'790), - 19 - 14 }, // * z^25
         -::boost::decimal::decimal64 { UINT64_C(1'181'259'301'697'458'770), - 19 - 15 }, // * z^26
         +::boost::decimal::decimal64 { UINT64_C(1'186'692'254'751'600'333), - 19 - 17 }, // * z^27
     }};

    static constexpr std::array<decimal128, 44> d128_coeffs =
    {{
         // N[Series[1/Gamma[z], {z, 0, 27}], 24]
         +::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(312909238939453), UINT64_C(7916302232898517972)  }, -34 },   // * z^2
         -::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(355552215013931), UINT64_C(2875353717947891404)  }, -34 },   // * z^3
         -::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(227696740770409), UINT64_C(1287992959696612036)  }, -35 },   // * z^4
         +::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(90280762131699),  UINT64_C(14660682722320745466) }, -34 },   // * z^5
         -::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(228754377395439), UINT64_C(1086189775515439306)  }, -35 },   // * z^6
         -::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(521608121705894), UINT64_C(2882773517907923486)  }, -36 },   // * z^7
         +::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(391339697554084), UINT64_C(12203646426790846826) }, -36 },   // * z^8
         -::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(63163861720165),  UINT64_C(1793625582468481749)  }, -36 },   // * z^9
         -::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(116682745342423), UINT64_C(7466931387917530902)  }, -37 },   // * z^10
         +::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(69416197176288),  UINT64_C(17486507952476000235) }, -37 },   // * z^11
         -::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(109151266480053), UINT64_C(14157573701904186532) }, -38 },   // * z^12
         -::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(67789387500902),  UINT64_C(6337242598258275460)  }, -39 },   // * z^13
         +::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(61421529319989),  UINT64_C(11330812743044278521) }, -39 },   // * z^14
         -::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(111474328952626), UINT64_C(4349913604764276954)  }, -40 },   // * z^15
         +::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(331554179970335), UINT64_C(8536598537651543980)  }, -42 },   // * z^16
         +::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(271159377746131), UINT64_C(11232450780359262294) }, -42 },   // * z^17
         -::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(64037022781195),  UINT64_C(7729482665838775386)  }, -42 },   // * z^18
         +::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(56564275382244),  UINT64_C(15921046388084405946) }, -43 },   // * z^19
         +::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(421877346419979), UINT64_C(12114109382397224706) }, -45 },   // * z^20
         -::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(200404234149424), UINT64_C(17191629897693416576) }, -45 },   // * z^21
         +::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(276491627306932), UINT64_C(18075235341994261118) }, -46 },   // * z^22
         -::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(111582078948016), UINT64_C(1315679057212061374)  }, -47 },   // * z^23
         -::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(289922303798056), UINT64_C(8236273575746269444)  }, -48 },   // * z^24
         +::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(66503802694735),  UINT64_C(8619931044472680662)  }, -48 },   // * z^25
         -::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(64036195058454),  UINT64_C(13570784405336680634) }, -49 },   // * z^26
         +::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(64330716033670),  UINT64_C(6228121739584017954)  }, -51 },   // * z^27
         +::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(76565308743615),  UINT64_C(9665163337994634860)  }, -51 },
         -::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(124615253252825), UINT64_C(5713012462345318490)  }, -52 },
         +::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(92938152937825),  UINT64_C(2160517649493992050)  }, -53 },
         +::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(72497982578925),  UINT64_C(10055707640313829460) }, -55 },
         -::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(111360223980902), UINT64_C(528747408384118098)   }, -55 },
         +::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(148320486134320), UINT64_C(12662323637555269860) }, -56 },
         -::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(93911231108772),  UINT64_C(8663955293807189228)  }, -57 },
         -::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(127969413738636), UINT64_C(17978922200959991754) }, -59 },
         +::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(101100927852914), UINT64_C(16158702556622869636) }, -59 },
         -::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(120243204727301), UINT64_C(13141135468649758444) }, -60 },
         +::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(70352901832557),  UINT64_C(2975454173305568482)  }, -61 },
         +::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(64005738370342),  UINT64_C(18063645830042937300) }, -63 },
         -::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(60963839731470),  UINT64_C(14965217315129705920) }, -63 },
         +::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(69230926066837),  UINT64_C(16656915204960392533) }, -64 },
         -::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(400691370795862), UINT64_C(16972369904241895558) }, -66 },
         +::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(61514934723438),  UINT64_C(5918930041313493498)  }, -68 },
         +::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(251487992814431), UINT64_C(6680121266003781724)  }, -68 },
         -::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(289879709778175), UINT64_C(4432551928123929090)  }, -69 },
     }};
};

#if !(defined(__cpp_inline_variables) && __cpp_inline_variables >= 201606L) && (!defined(_MSC_VER) || _MSC_VER != 1900)

template <bool b>
constexpr std::array<decimal32, 15> tgamma_table_imp<b>::d32_coeffs;

template <bool b>
constexpr std::array<decimal64, 36> tgamma_table_imp<b>::d64_coeffs;

template <bool b>
constexpr std::array<decimal128, 44> tgamma_table_imp<b>::d128_coeffs;

#endif

} //namespace tgamma_detail

using tgamma_table = tgamma_detail::tgamma_table_imp<true>;

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto tgamma_series_expansion(T z) noexcept;

template <>
constexpr auto tgamma_series_expansion<decimal32>(decimal32 z) noexcept
{
    return taylor_series_result(z, tgamma_table::d32_coeffs);
}

template <>
constexpr auto tgamma_series_expansion<decimal64>(decimal64 z) noexcept
{
    return taylor_series_result(z, tgamma_table::d64_coeffs);
}

template <>
constexpr auto tgamma_series_expansion<decimal128>(decimal128 z) noexcept
{
    return taylor_series_result(z, tgamma_table::d128_coeffs);
}

} //namespace detail
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_IMPL_TGAMMA_IMPL_HPP
