// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_FDIM_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_FDIM_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <type_traits>
#include <limits>
#include <cmath>

namespace boost { namespace decimal {

template<typename T, std::enable_if_t<detail::is_decimal_floating_point_v<T>, bool> = true>
constexpr auto fdim(T x, T y) noexcept -> T
{
    constexpr T zero {0, 0};

    if (isnan(x) || isinf(x))
    {
        return x;
    }
    else if (isnan(y))
    {
        return y;
    }

    if (y >= x)
    {
        return zero;
    }

    return x - y;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_FDIM_HPP
