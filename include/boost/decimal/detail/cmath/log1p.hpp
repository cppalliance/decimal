// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_LOG1P_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_LOG1P_HPP

#include <array>
#include <type_traits>

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/numbers.hpp>

namespace boost { namespace decimal {

template<typename T>
constexpr auto log1p(T x) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T> // NOLINT(misc-no-recursion)
{
    const auto fpc = fpclassify(x);

    constexpr T zero { 0, 0 };
    constexpr T one  { 1, 0 };

    auto result = zero;

    if (fpc == FP_ZERO)
    {
        result = x;
    }
    else if (fpc != FP_NORMAL)
    {
        if (fpc == FP_INFINITE)
        {
            if (signbit(x))
            {
                result = std::numeric_limits<T>::quiet_NaN();
            }
            else
            {
                result = x;
            }
        }
        else if (fpc == FP_NAN)
        {
            result = x;
        }
    }
    else if (-x >= one)
    {
        if (-x == one)
        {
            result = -std::numeric_limits<T>::infinity();
        }
        else
        {
            result = std::numeric_limits<T>::quiet_NaN();
        }
    }
    else
    {
        if (x > T { 5, -1 })
        {
            result = log(x + one);
        }
        else
        {
            using coefficient_array_type = std::array<T, static_cast<std::size_t>(UINT8_C(21))>;

            #if (defined(__clang__) && (__clang__ < 6))
            const
            #else
            constexpr
            #endif
            coefficient_array_type
                coefficient_table
                {
                     // Series[Log[1+x], {x, 0, 15}]

                     one,                          //             x
                     -T { 5, -1},                  // (-1 /  2) * x^2
                     one / T { UINT8_C(3), 0 },    // ( 1 /  3) * x^3
                     -T { 25, -2 },                // (-1 /  4) * x^4
                     T { 2, -1 },                  // ( 1 /  5) * x^5
                    -one / T { UINT8_C(6), 0 },    // (-1 /  6) * x^6
                     one / T { UINT8_C(7), 0 },    // ( 1 /  7) * x^7
                     -T { 125, -3 },               // (-1 /  8) * x^8
                     one / T { UINT8_C(9), 0 },    // ( 1 /  9) * x^9
                     -T { 1, -1 },                 // (-1 / 10) * x^10
                     one / T { UINT8_C(11), 0 },   // ( 1 / 11) * x^11
                    -one / T { UINT8_C(12), 0 },   // (-1 / 12) * x^12
                     one / T { UINT8_C(13), 0 },   // ( 1 / 13) * x^13
                    -one / T { UINT8_C(14), 0 },   // (-1 / 14) * x^14
                     one / T { UINT8_C(15), 0 },   // ( 1 / 15) * x^15
                    -one / T { UINT8_C(16), 0 },   // ( 1 / 16) * x^16
                     one / T { UINT8_C(17), 0 },   // ( 1 / 17) * x^17
                    -one / T { UINT8_C(18), 0 },   // ( 1 / 18) * x^18
                     one / T { UINT8_C(19), 0 },   // ( 1 / 19) * x^19
                     T { 5, -2 },                  // ( 1 / 20) * x^20
                     one / T { UINT8_C(21), 0 },   // ( 1 / 21) * x^21
                };

            auto rit = coefficient_table.crbegin() + static_cast<std::size_t>((sizeof(T) == 4U) ? 7U : 0U);

            result = *rit;

            while(rit != coefficient_table.crend())
            {
                result = fma(result, x, *rit++);
            }

            result *= x;
        }
    }

    return result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_LOG1P_HPP
