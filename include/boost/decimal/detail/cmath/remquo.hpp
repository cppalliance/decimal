// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_REMQUO_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_REMQUO_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <type_traits>
#include <limits>
#include <cmath>

namespace boost { namespace decimal {

template<typename T>
constexpr auto remquo(T x, T y, int* quo) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>
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

    // Compute quo
    auto div {x / y};
    T n {};
    const T frac {modf(div, &n)};
    auto sig {static_cast<std::int64_t>(n)};
    auto usig {static_cast<std::uint64_t>(sig < 0 ? -sig : sig)};

    // Apple clang and MSVC use the last byte.
    // Everyone else uses 3 bits.
    // Standard only specifies at least 3, so they are both correct
    #if defined(__APPLE__) || defined(_MSC_VER)
    *quo = static_cast<int>(usig & 0b1111);
    #else
    *quo = static_cast<int>(usig & 0b111);
    #endif
    *quo = sig < 0 ? -*quo : *quo;

    // Now compute the remainder
    if (frac > half)
    {
        ++n;
        ++*quo;
    }
    else if (frac < -half)
    {
        --n;
    }

    return x - n*y;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_REMQUO_HPP
