// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_TAN_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_TAN_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/numbers.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/cmath/cos.hpp>
#include <boost/decimal/detail/cmath/remquo.hpp>
#include <boost/decimal/detail/cmath/impl/sin_impl.hpp>
#include <boost/decimal/detail/cmath/impl/cos_impl.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <type_traits>
#include <cstdint>
#endif

namespace boost {
namespace decimal {

BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto tan(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    T result { };

    const auto fpc = fpclassify(x);

    // First check non-finite values and small angles.
    if (fabs(x) < std::numeric_limits<T>::epsilon() || (fpc == FP_NAN))
    {
        result = x;
    }
    else if (fpc == FP_INFINITE)
    {
        result = std::numeric_limits<T>::quiet_NaN();
    }
    else if (signbit(x))
    {
        result = -tan(-x);
    }
    else
    {
        // Perform argument reduction.

        // Given x = k * (pi/2) + r, compute n = (k % 4).

        // | n |  sin(x) |  cos(x) |  sin(x)/cos(x) |
        // |----------------------------------------|
        // | 0 |  sin(r) |  cos(r) |  sin(r)/cos(r) |
        // | 1 |  cos(r) | -sin(r) | -cos(r)/sin(r) |
        // | 2 | -sin(r) | -cos(r) |  sin(r)/cos(r) |
        // | 3 | -cos(r) |  sin(r) | -cos(r)/sin(r) |

        constexpr T my_pi_half { numbers::pi_v<T> / 2 };

        const T two_x = x * 2;

        const auto k = static_cast<unsigned>(two_x / numbers::pi_v<T>);
        const auto n = static_cast<unsigned>(k % static_cast<unsigned>(UINT8_C(4)));

        const T two_r { two_x - (numbers::pi_v<T> * k) };

        const T r { two_r / 2 };

        constexpr T cbrt_epsilon { cbrt(std::numeric_limits<T>::epsilon()) };

        constexpr T one { 1 };
        constexpr T two { 2 };

        switch(n)
        {
            case static_cast<unsigned>(UINT8_C(1)):
            case static_cast<unsigned>(UINT8_C(3)):
            {
                if (two_r < cbrt_epsilon)
                {
                    result = (two / two_r) - (two_r / 6) - (two_r * (two_r * two_r)) / 360;
                }
                else
                {
                    result = cos(r) / sin(r);
                }

                result = -result;

                break;
            }

            case static_cast<unsigned>(UINT8_C(0)):
            case static_cast<unsigned>(UINT8_C(2)):
            default:
            {
                const T d2r { numbers::pi_v<T> - two_r };

                if (d2r < cbrt_epsilon)
                {
                    result = (two / d2r) - (d2r / 6) - (d2r * (d2r * d2r)) / 360;
                }
                else
                {
                    result = sin(r) / cos(r);
                }

                break;
            }
        }
        
    }

    return result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_TAN_HPP
