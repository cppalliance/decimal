// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_HYPOT_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_HYPOT_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/cmath/abs.hpp>
#include <boost/decimal/detail/cmath/sqrt.hpp>
#include <boost/decimal/detail/utilities.hpp>
#include <type_traits>
#include <cmath>

namespace boost { namespace decimal {

template <typename T>
constexpr auto hypot(T x, T y) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>
{
    constexpr T zero {0, 0};

    if (abs(x) == zero)
    {
        return y;
    }
    else if (abs(y) == zero)
    {
        return x;
    }
    else if (isinf(x) || isinf(y))
    {
        // Return +inf even if the other value is nan
        return std::numeric_limits<T>::infinity();
    }
    else if (isnan(x))
    {
        return x;
    }
    else if (isnan(y))
    {
        return y;
    }

    x = abs(x);
    y = abs(y);

    if (y > x)
    {
        detail::swap(x, y);
    }

    if (x * std::numeric_limits<T>::epsilon() >= y)
    {
        return x;
    }

    const auto rat {y / x};
    return x * sqrt(1 + rat * rat);
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_HYPOT_HPP
