// Copyright 2024 Matt Borland
// Copyright 2023 - 2024 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_TGAMMA_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_TGAMMA_IMPL_HPP

#include <boost/decimal/detail/concepts.hpp>

#include <array>
#include <cstddef>
#include <cstdint>

namespace boost {
namespace decimal {

namespace detail {
namespace tgamma_detail {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
struct tgamma_table_imp { };

template <>
struct tgamma_table_imp<::boost::decimal::decimal32>
{
    static constexpr auto my_size = static_cast<std::size_t>(UINT8_C(15));

    using my_array_type = std::array<::boost::decimal::decimal32, my_size>;

    static constexpr my_array_type table =
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
};

template <>
struct tgamma_table_imp<::boost::decimal::decimal64>
{
    static constexpr auto my_size = static_cast<std::size_t>(UINT8_C(26));

    using my_array_type = std::array<::boost::decimal::decimal64, my_size>;

    static constexpr my_array_type table =
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
};

} //namespace tgamma_detail

template<typename T>
using tgamma_table_struct_type = detail::tgamma_detail::tgamma_table_imp<T>;

} //namespace detail
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_IMPL_TGAMMA_IMPL_HPP
