// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_ACOSH_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_ACOSH_HPP

#include <array>
#include <type_traits>

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/numbers.hpp>

namespace boost { namespace decimal {

template<typename T>
constexpr auto acosh(T x) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T> // NOLINT(misc-no-recursion)
{
    const auto fpc = fpclassify(x);

    auto result = -std::numeric_limits<T>::quiet_NaN();

    if (fpc != FP_NORMAL)
    {
        if ((fpc == FP_INFINITE) && (!signbit(x)))
        {
            result = x;
        }
        else if (fpc == FP_NAN)
        {
            result = x;
        }
    }
    else
    {
        constexpr T one  { 1, 0 };

        if (x < one)
        {
            // In this case, acosh(x) for x < 1 retains the initial value of -NaN.
        }
        else if (x > one)
        {
            // Use (perts of) the implementation of acosh from Boost.Math.

            constexpr T root_epsilon { 1, -((std::numeric_limits<T>::digits10 + 1) / 2) };

            if ((x - one) >= root_epsilon)
            {
                if (x > one / root_epsilon)
                {
                    // http://functions.wolfram.com/ElementaryFunctions/ArcCosh/06/01/06/01/0001/
                    // approximation by laurent series in 1/x at 0+ order from -1 to 0
                    result = log(x) + numbers::ln2_v<T>;
                }
                else if (x < T { 15, -1 })
                {
                   // This is just a rearrangement of the standard form below
                   // devised to minimise loss of precision when x ~ 1:
                   const auto y = x - one;

                   constexpr T two { 2, 0 };

                   result = log1p(y + sqrt(y * y + two * y));
                }
                else
                {
                    // http://functions.wolfram.com/ElementaryFunctions/ArcCosh/02/
                    return(log( x + sqrt(x * x - one)));
                }
            }
            else
            {
                // see http://functions.wolfram.com/ElementaryFunctions/ArcCosh/06/01/04/01/0001/
                T y = x - 1;
                
                // approximation by taylor series in y at 0 up to order 2
                T result = sqrt(2 * y) * (1 - y /12 + 3 * y * y / 160);
                return result;
            }
        }
        else
        {
            // This is acosh of 1.
            result = T { 0, 0 };
        }
    }

    return result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_ACOSH_HPP
