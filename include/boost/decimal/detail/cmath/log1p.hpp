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
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/numbers.hpp>

namespace boost {
namespace decimal {

template <typename T>
constexpr auto log1p(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
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
            #  pragma clang diagnostic push
            #  pragma clang diagnostic ignored "-Wmissing-braces"
            #endif

            constexpr auto coefficient_table =
                coefficient_array_type
                {
                     // Series[Log[1+x], {x, 0, 21}]

                      T { 1, 0 },                                  //             x
                     -T { 5, -1},                                  // (-1 /  2) * x^2
                      T { UINT64_C(333333333333333333), -18 },     // ( 1 /  3) * x^3
                     -T { 25, -2 },                                // (-1 /  4) * x^4
                      T { 2, -1 },                                 // ( 1 /  5) * x^5
                     -T { UINT64_C(166666666666666667), -18 },     // ( 1 /  6) * x^6
                      T { UINT64_C(142857142857142857), -18 },     // ( 1 /  7) * x^7
                     -T { 125, -3 },                               // (-1 /  8) * x^8
                      T { UINT64_C(111111111111111111), -18 },     // ( 1 /  9) * x^9
                     -T { 1, -1 },                                 // (-1 / 10) * x^10
                      T { UINT64_C(909090909090909091), -18 - 1 }, // ( 1 / 11) * x^11
                     -T { UINT64_C(833333333333333333), -18 - 1 }, // (-1 / 12) * x^12
                      T { UINT64_C(769230769230769230), -18 - 1 }, // ( 1 / 13) * x^13
                     -T { UINT64_C(714285714285714286), -18 - 1 }, // (-1 / 14) * x^14
                      T { UINT64_C(666666666666666667), -18 - 1 }, // ( 1 / 15) * x^15
                     -T { UINT64_C(625000000000000000), -18 - 1 }, // ( 1 / 16) * x^16
                      T { UINT64_C(588235294117647059), -18 - 1 }, // ( 1 / 17) * x^17
                     -T { UINT64_C(555555555555555556), -18 - 1 }, // ( 1 / 18) * x^18
                      T { UINT64_C(526315789473684211), -18 - 1 }, // ( 1 / 19) * x^19
                     -T { 5, -2 },                                 // ( 1 / 20) * x^20
                      T { UINT64_C(476190476190476190), -18 - 1 }  // ( 1 / 21) * x^21
                };

            #if (defined(__clang__) && (__clang__ < 6))
            #  pragma clang diagnostic pop
            #endif

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
