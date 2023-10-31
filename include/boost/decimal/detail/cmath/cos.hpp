// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_COS_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_COS_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/numbers.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/cmath/cos.hpp>
#include <boost/decimal/detail/cmath/remquo.hpp>
#include <boost/decimal/detail/cmath/impl/sin_impl.hpp>
#include <boost/decimal/detail/cmath/impl/cos_impl.hpp>
#include <type_traits>
#include <cstdint>

namespace boost { namespace decimal {

template<typename T, std::enable_if_t<detail::is_decimal_floating_point_v<T>, bool> = true>
constexpr auto cos(T x) noexcept -> T
{
    constexpr T zero {0, 0};
    constexpr T one {1, 0};

    T result { };

    // First check non-finite values and small angles
    if (isinf(x) || isnan(x))
    {
        result = x;
    }
    else if (abs(x) < std::numeric_limits<T>::epsilon())
    {
        result = one;
    }
    else if (x < zero)
    {
        result = cos(-x);
    }
    else
    {
        // Perform argument reduction and subsequent computation of the result.

        // Given x = k * (pi/2) + r, compute n = (k % 4).

        // | n |  sin(x) |  cos(x) |  sin(x)/cos(x) |
        // |----------------------------------------|
        // | 0 |  sin(r) |  cos(r) |  sin(r)/cos(r) |
        // | 1 |  cos(r) | -sin(r) | -cos(r)/sin(r) |
        // | 2 | -sin(r) | -cos(r) |  sin(r)/cos(r) |
        // | 3 | -cos(r) |  sin(r) | -cos(r)/sin(r) |

        constexpr auto my_pi_half = numbers::pi_v<T> / 2;

        int k {};
        auto r { remquo(x, my_pi_half, &k) };

        const auto n = static_cast<unsigned>(k % 4U);

        result = (((n == 1U) || (n == 3U)) ? detail::sin_impl(r) : detail::cos_impl(r));

        if ((n == 1U) || (n == 2U))
        {
          result = -result;
        }
    }

    return result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_COS_HPP
