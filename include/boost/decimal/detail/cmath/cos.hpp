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

    T u = T(1.7209676144094619e-05);
    u = u * x + T(2.948681946150653e-05);
    u = u * x + T(-0.0014412503094103166);
    u = u * x + T(5.1536622967173333e-05);
    u = u * x + T(0.041637504302165618);
    u = u * x + T(9.1699203194461092e-06);
    u = u * x + T(-0.50000143607979219);
    u = u * x + T(8.6335229817523757e-08);
    return u * x + T(0.99999999914639126);
}

}} // Namespaces

#endif // BOOST_DECIMAL_DETAIL_CMATH_COS_HPP
