// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_SINH_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_SINH_HPP

#include <array>
#include <type_traits>

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/numbers.hpp>

namespace boost { namespace decimal {

template<typename T>
constexpr auto sinh(T x) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T> // NOLINT(misc-no-recursion)
{
    const auto fpc = fpclassify(x);

    constexpr T zero { 0, 0 };

    auto result = zero;

    if (fpc == FP_ZERO)
    {
        result = x;
    }
    else if (fpc != FP_NORMAL)
    {
        if (fpc == FP_INFINITE)
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
        if (signbit(x))
        {
            result = -sinh(-x);
        }
        else
        {
            constexpr T one  { 1, 0 };

            if (x < one)
            {
                using coefficient_array_type = std::array<T, static_cast<std::size_t>(UINT8_C(9))>;

                #if (defined(__clang__) && (__clang__ < 6))
                const
                #else
                constexpr
                #endif
                auto coefficient_table =
                    coefficient_array_type
                    {
                        // Series[Sinh[x], {x, 0, 19}]
                        //            (1),                      // * x
                        one / UINT8_C (6),                      // * x^3
                        one / UINT8_C (120),                    // * x^5
                        one / UINT16_C(5'040),                  // * x^7
                        one / UINT32_C(362'880),                // * x^9
                        one / UINT32_C(39'916'800),             // * x^11
                        one / UINT64_C(6'227'020'800),          // * x^13
                        one / UINT64_C(1'307'674'368'000),      // * x^15
                        one / UINT64_C(355'687'428'096'000),    // * x^17
                        one / UINT64_C(121'645'100'408'832'000) // * x^19
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

                result = x * fma(result, xsq, one);
            }
            else
            {
                const auto exp_pos_val = exp(x);
                const auto exp_neg_val = one / exp_pos_val;

                constexpr T two { 2, 0 };

                result = (exp_pos_val - exp_neg_val) / two;
            }
        }
    }

    return result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_SINH_HPP
