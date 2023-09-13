// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_COS_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_COS_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/cmath/fmin.hpp>
#include <boost/decimal/detail/cmath/fmax.hpp>
#include <boost/decimal/numbers.hpp>
#include <type_traits>
#include <cstdint>

namespace boost { namespace decimal {

template<typename T, std::enable_if_t<detail::is_decimal_floating_point_v<T>, bool> = true>
constexpr auto cos_impl(T x) noexcept -> T
{
    constexpr T zero {0, 0};
    if (x < zero)
    {
        return cos(-x);
    }

    T u = T(2.2805960529562646e-05);
    u = u * x + T(3.9171880037888081e-06);
    u = u * x + T(-0.001392392773950284);
    u = u * x + T(1.7339629614857501e-06);
    u = u * x + T(0.041666173896377827);
    u = u * x + T(7.7764646000512304e-08);
    u = u * x + T(-0.50000000610949535);
    u = u * x + T(1.8421494272283811e-10);
    return u * x + T(0.99999999999908662);
}

}} // Namespaces

#endif // BOOST_DECIMAL_DETAIL_CMATH_COS_HPP
