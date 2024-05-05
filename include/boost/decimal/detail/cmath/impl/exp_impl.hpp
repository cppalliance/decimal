// Copyright 2024 Matt Borland
// Copyright 2024 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_EXP_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_EXP_IMPL_HPP

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

namespace exp_detail {

template <bool b>
struct exp_table_imp
{
private:
    using d128_coeffs_t = std::array<decimal128, 17>;

public:
    static constexpr d128_coeffs_t d128_coeffs =
    {{
         // Series[Exp[x] - 1, {x, 0, 18}]
         //            (1),                                                                                                                   // * x
         ::boost::decimal::decimal128 { 5, -1 },                                                                                              // * x^2
         ::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(90350181040458),  UINT64_C(12964998083131386532) }, -34 },
         ::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(225875452601146), UINT64_C(13965751134118914724) }, -35 },
         ::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(451750905202293), UINT64_C(9484758194528277842)  }, -36 },
         ::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(75291817533715),  UINT64_C(10804165069276155440) }, -36 },
         ::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(107559739333879), UINT64_C(7528774067376128516)  }, -37 },
         ::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(134449674167349), UINT64_C(4799281565792772746)  }, -38 },
         ::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(149388526852610), UINT64_C(5332535073103080820)  }, -39 },
         ::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(149388526852610), UINT64_C(5332535073103080820)  }, -40 },
         ::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(135807751684191), UINT64_C(3170782423392841514)  }, -41 },
         ::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(113173126403492), UINT64_C(11865690723015477068) }, -42 },
         ::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(87056251079609),  UINT64_C(13384395342406417346) }, -43 },
         ::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(62183036485435),  UINT64_C(9560282387433155251)  }, -44 },
         ::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(414553576569570), UINT64_C(2246069003855862950)  }, -46 },
         ::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(259095985355981), UINT64_C(6015479145837302244)  }, -47 },
         ::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(152409403150577), UINT64_C(4623619737181327888)  }, -48 },
         ::boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(84671890639209),  UINT64_C(10767230553416093986) }, -49 },
    }};
};

#if !(defined(__cpp_inline_variables) && __cpp_inline_variables >= 201606L) && (!defined(_MSC_VER) || _MSC_VER != 1900)

template <bool b>
constexpr typename exp_table_imp<b>::d128_coeffs_t exp_table_imp<b>::d128_coeffs;

#endif

} //namespace exp_detail

using exp_table = exp_detail::exp_table_imp<true>;

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto exp_pade_appxroximant_or_series(T x) noexcept;

template <>
constexpr auto exp_pade_appxroximant_or_series<decimal32>(decimal32 x) noexcept
{
    using local_float_t = decimal32;

    // PadeApproximant[Exp[x] - 1, {x, 0, {3, 4}}]
    // FullSimplify[%]
    //   (40 x (42 + x^2))
    // / (1680 + x (-840 + x (180 + (-20 + x) x)))

    const auto x2 = x * x;

    // Use the small-argument Pade approximation having coefficients shown above.
    const local_float_t top { local_float_t { UINT8_C(40), 0 } * x * (local_float_t { UINT8_C(42), 0 } + x2) };
    const local_float_t bot { local_float_t { UINT16_C(1680), 0 } + x * (local_float_t { INT16_C(-840), 0 } + x * (local_float_t { UINT8_C(180), 0 } + (local_float_t { INT8_C(-20), 0 } + x) * x)) };

    constexpr local_float_t one { 1 };

    return one + (top / bot);
}

template <>
constexpr auto exp_pade_appxroximant_or_series<decimal64>(decimal64 x) noexcept
{
    using local_float_t = decimal64;

    // PadeApproximant[Exp[x] - 1, {x, 0, {6, 6}}]
    // FullSimplify[%]
    //   (84 x (7920 + 240 x^2 + x^4))
    // / (665280 + x (-332640 + x (75600 + x (-10080 + x (840 + (-42 + x) x)))))

    const auto x2 = x * x;

    // Use the small-argument Pade approximation having coefficients shown above.
    const local_float_t top = local_float_t { UINT8_C(84), 0 } * x * ( local_float_t { UINT16_C(7920), 0 } + ( local_float_t { UINT8_C(240), 0 } + x2) * x2);
    const local_float_t bot = local_float_t { UINT32_C(665280), 0 } + x * (local_float_t { INT32_C(-332640), 0 } + x * (local_float_t { UINT32_C(75600), 0 } + x * (local_float_t { INT16_C(-10080), 0 } + x * (local_float_t { UINT16_C(840), 0 } + (local_float_t { INT8_C(-42), 0 } + x) * x))));

    constexpr local_float_t one { 1 };

    return one + (top / bot);
}

template <>
constexpr auto exp_pade_appxroximant_or_series<decimal128>(decimal128 x) noexcept
{
    // Compute exp(x) - 1 for x small.
    // Use argument scaling in combination with a Taylor series expansion to order-18.

    using local_float_t = decimal128;

    // Rescale the argument even further (and note the three squarings below).
    x /= 8;

    constexpr local_float_t one { 1, 0 };

    // Note: The Taylor series expansion begins with coefficients of order-2.
    // So we need to multiply by x^2 and add the two skipped terms (1 + x).

    local_float_t
        result
        {
            one + (x * (one + (x * taylor_series_result(x, exp_table::d128_coeffs))))
        };

    // Scale up with three squarings in order to obtain the result.
    result *= result;
    result *= result;

    return result *= result;
}

} //namespace detail
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_IMPL_EXP_IMPL_HPP
