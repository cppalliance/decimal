// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_TANH_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_TANH_HPP

#include <array>
#include <type_traits>

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/numbers.hpp>

namespace boost { namespace decimal {

template<typename T>
constexpr auto tanh(T x) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T> // NOLINT(misc-no-recursion)
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
                result = -one;
            }
            else
            {
                result = one;
            }
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
            result = -tanh(-x);
        }
        else
        {
            constexpr T quarter { 25, -2 };

            if (x < quarter)
            {
                using coefficient_array_type = std::array<T, static_cast<std::size_t>(UINT8_C(11))>;

                #if (defined(__clang__) && (__clang__ < 6))
                const
                #else
                constexpr
                #endif
                auto coefficient_table =
                    coefficient_array_type
                    {
                        // Series[Tanh[x], {x, 0, 23}]

                        //         (1),                                                        // * x
                        -one                               / UINT8_C (3),                      // * x^3
                        T { INT8_C(2),                 0 } / UINT8_C (15),                     // * x^5
                        T { INT8_C(-17),               0 } / UINT16_C(315),                    // * x^7
                        T { UINT8_C(62),               0 } / UINT16_C(2'835),                  // * x^9
                        T { INT16_C(-1'382),           0 } / UINT32_C(155'925),                // * x^11
                        T { UINT16_C(21'844),          0 } / UINT32_C(6'081'075),              // * x^13
                        T { INT32_C(-929'569),         0 } / UINT32_C(638'512'875),            // * x^15
                        T { UINT32_C(6'404'582),       0 } / UINT64_C(10'854'718'875),         // * x^17
                        T { INT32_C(-443'861'162),     0 } / UINT64_C(1'856'156'927'625),      // * x^19
                        T { UINT64_C(18'888'466'084),  0 } / UINT64_C(194'896'477'400'625),    // * x^21
                        T { INT64_C(-113'927'491'862), 0 } / UINT64_C(2'900'518'163'668'125),  // * x^23
                    };

                auto rit =
                    coefficient_table.crbegin()
                  + static_cast<std::size_t>
                    (
                      (sizeof(T) == static_cast<std::size_t>(UINT8_C(4))) ? 6U : 0U
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

                result = (exp_pos_val - exp_neg_val) / (exp_pos_val + exp_neg_val);
            }
        }
    }

    return result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_TANH_HPP
