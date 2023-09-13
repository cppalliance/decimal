// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_SIN_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_SIN_IMPL_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <type_traits>
#include <cstdint>

namespace boost { namespace decimal { namespace detail {

template<typename T, std::enable_if_t<detail::is_decimal_floating_point_v<T>, bool> = true>
constexpr auto sin_impl(T x) noexcept -> T
{
    constexpr T zero {0, 0};
    if (x < zero)
    {
        return -sin_impl(-x);
    }

    T u = T(9.4465293626777997e-06);
    u = u * x + T(-0.0002121051143997598);
    u = u * x + T(1.0766679285155848e-05);
    u = u * x + T(0.0083283844849017102);
    u = u * x + T(1.3384154242642468e-06);
    u = u * x + T(-0.16666687019704884);
    u = u * x + T(1.5512908179440871e-08);
    u = u * x + T(0.99999999954464358);
    return u * x + T(2.2004194322650135e-12);
}

}}} // Namespaces

#endif
