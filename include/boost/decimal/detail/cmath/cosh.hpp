// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_COSH_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_COSH_HPP

#include <array>
#include <type_traits>

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/numbers.hpp>

namespace boost { namespace decimal {

template<typename T>
constexpr auto cosh(T x) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T> // NOLINT(misc-no-recursion)
{
    const auto fpc = fpclassify(x);

    T result { };

    constexpr T zero { 0, 0 };
    constexpr T one  { 1, 0 };

    if (fpc == FP_ZERO)
    {
        result = one;
    }
    else if (fpc != FP_NORMAL)
    {
        if (fpc == FP_INFINITE)
        {
            result = abs(x);
        }
        else if (fpc == FP_NAN)
        {
            result = abs(x);
        }
        else
        {
            result = zero;
        }
    }
    else
    {
        if (signbit(x))
        {
            result = cosh(-x);
        }
        else
        {
            if (x < one)
            {
                using coefficient_array_type = std::array<T, static_cast<std::size_t>(UINT8_C(9))>;

                constexpr auto coefficient_table =
                    coefficient_array_type
                    {
                        // Series[Cosh[x], {x, 0, 18}]
                        //            (1),                    // * 1
                        one / UINT8_C (2),                    // * x^2
                        one / UINT8_C (24),                   // * x^4
                        one / UINT16_C(720),                  // * x^6
                        one / UINT16_C(40'320),               // * x^8
                        one / UINT32_C(3'628'800),            // * x^10
                        one / UINT32_C(479'001'600),          // * x^12
                        one / UINT64_C(87'178'291'200),       // * x^14
                        one / UINT64_C(20'922'789'888'000),   // * x^16
                        one / UINT64_C(6'402'373'705'728'000) // * x^18
                    };

                auto rit =
                    coefficient_table.crbegin()
                  + static_cast<std::size_t>
                    (
                      (sizeof(T) == static_cast<std::size_t>(UINT8_C(4))) ? 4U : 0U
                    );

                result = *rit;

                const auto xsq = x * x;

                while(rit != coefficient_table.crend())
                {
                    result = fma(result, xsq, *rit++);
                }

                result = fma(result, xsq, one);
            }
            else
            {
                const auto exp_pos_val = exp(x);
                const auto exp_neg_val = one / exp_pos_val;

                constexpr T two { 2, 0 };

                result = (exp_pos_val + exp_neg_val) / two;
            }
        }
    }

    return result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_COSH_HPP
