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

    constexpr T zero { 0, 0 };
    constexpr T one  { 1, 0 };

    auto result = zero;

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
                        //            (1),                             // * 1
                        T { 5, -1 },                                   // * x^2
                        T { UINT64_C(416666666666666667), - 18 -  1 }, // * x^4
                        T { UINT64_C(138888888888888889), - 18 -  2 }, // * x^6
                        T { UINT64_C(248015873015873016), - 18 -  4 }, // * x^8
                        T { UINT64_C(275573192239858907), - 18 -  6 }, // * x^10
                        T { UINT64_C(208767569878680990), - 18 -  8 }, // * x^12
                        T { UINT64_C(114707455977297247), - 18 - 10 }, // * x^14
                        T { UINT64_C(477947733238738530), - 18 - 13 }, // * x^16
                        T { UINT64_C(156192069685862265), - 18 - 15 }  // * x^18
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
