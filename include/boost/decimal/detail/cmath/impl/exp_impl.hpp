// Copyright 2024 Matt Borland
// Copyright 2024 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_EXP_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_EXP_IMPL_HPP

#include <boost/decimal/detail/concepts.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <array>
#include <cstddef>
#include <cstdint>
#endif

namespace boost {
namespace decimal {
namespace detail {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto exp_pade_appxroximant(T x) noexcept;

template <>
constexpr auto exp_pade_appxroximant<decimal32>(decimal32 x) noexcept
{
    // TODO: Chris: At 32-bit, reduce the number of coefficients in the Pade appxorimant of the exp() function.

    using local_float_t = decimal32;

    // PadeApproximant[Exp[x] - 1, {x, 0, {6, 6}}]
    // FullSimplify[%]
    //   (84 x (7920 + 240 x^2 + x^4))
    // / (665280 + x (-332640 + x (75600 + x (-10080 + x (840 + (-42 + x) x)))))

    const auto x2 = x * x;

    // Use the small-argument Pade approximation having coefficients shown above.
    const local_float_t top = local_float_t { UINT8_C(84), 0 } * x * ( local_float_t { UINT16_C(7920), 0 } + ( local_float_t { UINT8_C(240), 0 } + x2) * x2);
    const local_float_t bot = local_float_t { UINT32_C(665280), 0 } + x * (local_float_t { INT32_C(-332640), 0 } + x * (local_float_t { UINT32_C(75600), 0 } + x * (local_float_t { INT16_C(-10080), 0 } + x * (local_float_t { UINT16_C(840), 0 } + (local_float_t { INT8_C(-42), 0 } + x) * x))));

    return local_float_t { 1 } + (top / bot);
}

template <>
constexpr auto exp_pade_appxroximant<decimal64>(decimal64 x) noexcept
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

    return local_float_t { 1 } + (top / bot);
}

template <>
constexpr auto exp_pade_appxroximant<decimal128>(decimal128 x) noexcept
{
    // Compute exp(x) - 1 for x small.

    // Use an order-12 Pade approximation of the exponential function.
    // PadeApproximant[Exp[x] - 1, {x, 0, 12, 12}].

    using local_float_t = decimal128;

    // Rescale the argument even further (and note the three squarings below).
    x /= 8;

    const local_float_t x2 = (x * x);

    const local_float_t top = (((((  local_float_t { boost::decimal::detail::uint128 { UINT64_C(130576843339991), UINT64_C(2348781707059460614)  }, -46 }   * x2
                                   + local_float_t { boost::decimal::detail::uint128 { UINT64_C(502720846858965), UINT64_C(15499169997977266440) }, -43 } ) * x2
                                   + local_float_t { boost::decimal::detail::uint128 { UINT64_C(492264253244299), UINT64_C(6469924059228430936)  }, -40 } ) * x2
                                   + local_float_t { boost::decimal::detail::uint128 { UINT64_C(168354374609550), UINT64_C(6971973999273187690)  }, -37 } ) * x2
                                   + local_float_t { boost::decimal::detail::uint128 { UINT64_C(196413437044475), UINT64_C(8133969665818718980)  }, -35 } ) * x2
                                   + local_float_t { boost::decimal::detail::uint128 { UINT64_C(54210108624275),  UINT64_C(4089650035136921600)  }, -33 } )
                                   ;

    const local_float_t bot = ((((((((((((  local_float_t( +boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(418515523525612), UINT64_C(10839100561497421498) }, -49 } )  * x
                                          + local_float_t( -boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(65288421669995),  UINT64_C(10397762890384506116) }, -46 } )) * x
                                          + local_float_t( +boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(502720846858965), UINT64_C(15499169997977266440) }, -45 } )) * x
                                          + local_float_t( -boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(251360423429482), UINT64_C(16972957035843409028) }, -43 } )) * x
                                          + local_float_t( +boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(90489752434613),  UINT64_C(15702571451232594082) }, -41 } )) * x
                                          + local_float_t( -boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(246132126622149), UINT64_C(12458334066468991276) }, -40 } )) * x
                                          + local_float_t( +boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(516877465906514), UINT64_C(5871083058504374896)  }, -39 } )) * x
                                          + local_float_t( -boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(84177187304775),  UINT64_C(3485986999636593840)  }, -37 } )) * x
                                          + local_float_t( +boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(105221484130968), UINT64_C(18192541804827906022) }, -36 } )) * x
                                          + local_float_t( -boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(98206718522237),  UINT64_C(13290356869764135298) }, -35 } )) * x
                                          + local_float_t( +boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(64816434224676),  UINT64_C(16519268045002340975) }, -34 } )) * x
                                          + local_float_t( -boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(271050543121376), UINT64_C(2001506101975056384)  }, -34 } )) * x
                                          + local_float_t( +boost::decimal::decimal128 { boost::decimal::detail::uint128 { UINT64_C(54210108624275),  UINT64_C(4089650035136921600)  }, -33 } ))
                                          ;

    local_float_t result { local_float_t { 1 } + ((x * top) / bot) };

    result *= result;
    result *= result;
    result *= result;

    return result;
}

} //namespace detail
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_IMPL_EXP_IMPL_HPP
