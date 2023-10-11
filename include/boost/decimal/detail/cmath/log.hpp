// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_LOG_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_LOG_HPP

#include <cmath>
#include <type_traits>

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/numbers.hpp>

namespace boost { namespace decimal {

template<typename T>
constexpr auto log(T x) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T> // NOLINT(misc-no-recursion)
{
    constexpr T zero { 0, 0 };
    constexpr T one  { 1, 0 };

    auto result = zero;

    if (isnan(x))
    {
        result = x;
    }
    else if (isinf(x))
    {
        if (!signbit(x))
        {
            result = x;
        }
        else
        {
            result = std::numeric_limits<T>::quiet_NaN();
        }
    }
    else if (x < one)
    {
        // Handle reflection, the [+/-] zero-pole, and non-pole, negative x.
        if (x > zero)
        {
            result = -log(one / x);
        }
        else if ((x == zero) || (-x == zero))
        {
            // Actually, this should be equivalent to -HUGE_VAL.

            result = -std::numeric_limits<T>::infinity();
        }
        else
        {
            result = std::numeric_limits<T>::quiet_NaN();
        }
    }
    else if(x > one)
    {
        // The algorithm for logarithm is based on Chapter 5, pages 35-36
        // of Cody and Waite, Software Manual for the Elementary Functions,
        // Prentice Hall, 1980.

        int exp2val { };

        // TODO(ckormanyos) There is probably something more efficient than calling frexp here.
        auto g = frexp(x, &exp2val);

        if (g < numbers::inv_sqrt2_v<T>)
        {
            g += g;

            --exp2val;
        }

        using coefficient_array_type = std::array<T, static_cast<std::size_t>(UINT8_C(11))>;

        #if (defined(__clang__) && (__clang__ < 6))
        const
        #else
        constexpr
        #endif
        auto coefficient_table =
            coefficient_array_type
            {
                // (1,) 12, 80, 448, 2304, 11264, 53248, 245760, 1114112, 4980736, 22020096, 96468992, ...
                // See also Sloane's A058962 at: https://oeis.org/A058962

                // See also
                // Series[Log[(1 + (z/2))/(1 - (z/2))], {z, 0, 21}]
                // Or at Wolfram Alpha: https://www.wolframalpha.com/input?i=Series%5BLog%5B%281+%2B+%28z%2F2%29%29%2F%281+-+%28z%2F2%29%29%5D%2C+%7Bz%2C+0%2C+21%7D%5D

                one / T { UINT8_C(12), 0 },
                one / T { UINT8_C(80), 0 },
                one / T { UINT16_C(448), 0 },
                one / T { UINT16_C(11264), 0 },
                one / T { UINT16_C(53248), 0 },
                one / T { UINT32_C(245760), 0 },
                one / T { UINT32_C(1114112), 0 },
                one / T { UINT32_C(4980736), 0 },
                one / T { UINT32_C(22020096), 0 },
                one / T { UINT32_C(96468992), 0 }
            };

        const auto s   = (g - one) / (g + one);
        const auto z   = s + s;
        const auto zsq = z * z;

        auto rit = coefficient_table.crbegin() + static_cast<std::size_t>((sizeof(T) == 4U) ? 5U : 0U);

        result = *rit;

        while(rit != coefficient_table.crend())
        {
            result = fma(result, zsq, *rit++);
        }

        result = z * fma(result, zsq, one);

        if (exp2val > 0)
        {
            result += static_cast<T>(exp2val * numbers::ln2_v<T>);
        }
    }
    else
    {
        result = zero;
    }

    return result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_LOG_HPP
