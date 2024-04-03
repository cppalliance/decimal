// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_COS_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_COS_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/numbers.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/cmath/cos.hpp>
#include <boost/decimal/detail/cmath/remquo.hpp>
#include <boost/decimal/detail/cmath/impl/sin_impl.hpp>
#include <boost/decimal/detail/cmath/impl/cos_impl.hpp>
#include <type_traits>
#include <cstdint>

namespace boost {
namespace decimal {

namespace detail {

template <typename T>
constexpr auto cos_impl(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    T result { };

    const auto fpc = fpclassify(x);

    // First check non-finite values and small angles
    if ((fpc == FP_INFINITE) || (fpc == FP_NAN))
    {
        result = x;
    }
    else
    {
        x = abs(x);

        if (x < std::numeric_limits<T>::epsilon())
        {
            constexpr T one {1, 0};

            result = one;
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

            int k {};
            auto r { remquo(x, my_pi_half, &k) };

            const auto n = static_cast<unsigned>(k % 4);

            switch(n)
            {
                case 3U:
                    result = detail::sin_series_expansion(r);
                    break;
                case 2U:
                    result = -detail::cos_series_expansion(r);
                    break;
                case 1U:
                    result = -detail::sin_series_expansion(r);
                    break;
                case 0U:
                default:
                    result = detail::cos_series_expansion(r);
                    break;
            }
        }
    }

    return result;
}

} // namespace detail

BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto cos(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    #if BOOST_DECIMAL_DEC_EVAL_METHOD == 0

    using evaluation_type = T;

    #elif BOOST_DECIMAL_DEC_EVAL_METHOD == 1

    using evaluation_type = detail::promote_args_t<T, decimal64>;

    #else // BOOST_DECIMAL_DEC_EVAL_METHOD == 2

    using evaluation_type = detail::promote_args_t<T, decimal128>;

    #endif

    return static_cast<T>(detail::cos_impl(static_cast<evaluation_type>(x)));
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_COS_HPP
