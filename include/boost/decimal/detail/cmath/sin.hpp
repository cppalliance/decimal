// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_SIN_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_SIN_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/numbers.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/cmath/cos.hpp>
#include <boost/decimal/detail/cmath/remquo.hpp>
#include <boost/decimal/detail/cmath/impl/sin_impl.hpp>
#include <boost/decimal/detail/cmath/impl/cos_impl.hpp>
#include <type_traits>
#include <cstdint>

namespace boost {
namespace decimal {

template <typename T>
constexpr auto sin(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    T result { };

    const auto fpc = fpclassify(x);

    // First check non-finite values and small angles.
    if (fabs(x) < std::numeric_limits<T>::epsilon() || (fpc == FP_INFINITE) || (fpc == FP_NAN))
    {
        result = x;
    }
    else if (signbit(x))
    {
        result = -sin(-x);
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

        #if (defined(_MSC_VER) && (_MSC_VER < 1920))
        const auto my_pi_half = numbers::pi_v<T> / 2;
        #else
        constexpr auto my_pi_half = numbers::pi_v<T> / 2;
        #endif

        int k { };
        auto r { remquo(x, my_pi_half, &k) };

        const auto n = static_cast<unsigned>(k % 4);

        switch(n)
        {
            case 3U:
                result = -detail::cos_impl(r);
                break;
            case 2U:
                result = -detail::sin_impl(r);
                break;
            case 1U:
                result = detail::cos_impl(r);
                break;
            case 0U:
            default:
                result = detail::sin_impl(r);
                break;
        }
    }

    return result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_SIN_HPP
