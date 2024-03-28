// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_REMQUO_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_REMQUO_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/emulated128.hpp>
#include <type_traits>
#include <limits>
#include <cmath>

namespace boost {
namespace decimal {

template <typename T>
constexpr auto remquo(T x, T y, int* quo) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    using unsigned_significand_type = std::conditional_t<std::is_same<T, decimal128>::value, detail::uint128, std::uint64_t>;

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
    auto usig {static_cast<unsigned_significand_type>(n < 0 ? -n : n)};

    // Apple clang and MSVC use the last nibble.
    // Everyone else uses 3 bits.
    // Standard only specifies at least 3, so they are both correct
    #if defined(__APPLE__) || defined(_MSC_VER)
    constexpr auto unsigned_value_mask {0b1111U};
    #else
    constexpr auto unsigned_value_mask {0b111U};
    #endif

    *quo = static_cast<int>(usig & unsigned_value_mask);
    *quo = n < 0 ? -*quo : *quo;

    // Now compute the remainder
    if (frac > half)
    {
        ++n;
        *quo += 1;
        if (static_cast<unsigned>(*quo) > unsigned_value_mask)
        {
            *quo = 0;
        }
    }
    else if (frac < -half)
    {
        --n;
        *quo -= 1;
        if (static_cast<unsigned>(-*quo) > unsigned_value_mask)
        {
            *quo = 0;
        }
    }

    return x - n*y;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_REMQUO_HPP
