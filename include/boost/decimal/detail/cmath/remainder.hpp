// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_REMAINDER_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_REMAINDER_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <type_traits>
#include <limits>
#include <cmath>

namespace boost { namespace decimal {

template<typename T, std::enable_if_t<detail::is_decimal_floating_point_v<T>, bool> = true>
constexpr auto remainder(T x, T y) noexcept -> T
{
    constexpr T zero {0, 0};
    constexpr T half {5, -1};

    if ((isinf(x) && !isinf(y)) ||
        (abs(y) == zero && !isnan(x)))
    {
        return std::numeric_limits<T>::quiet_NaN();
    }
    else if (isnan(x))
    {
        return x;
    }
    else if (isnan(y))
    {
        return y;
    }

    T n {};
    const T frac {modf(x / y, &n)};
    if (frac > half)
    {
        ++n;
    }
    else if (frac < -half)
    {
        --n;
    }

    return x - n*y;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_REMAINDER_HPP
