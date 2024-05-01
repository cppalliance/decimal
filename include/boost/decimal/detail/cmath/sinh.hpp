// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_SINH_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_SINH_HPP

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/numbers.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <array>
#include <type_traits>
#endif

namespace boost {
namespace decimal {

BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto sinh(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
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
                #  pragma clang diagnostic push
                #  pragma clang diagnostic ignored "-Wmissing-braces"
                #endif

                constexpr auto coefficient_table =
                    coefficient_array_type
                    {
                        // Series[Sinh[x], {x, 0, 19}]
                        //            (1),                      // * x
                        T { UINT64_C(166666666666666667), -18  - 0 }, // * x^3
                        T { UINT64_C(833333333333333333), -18  - 2 }, // * x^5
                        T { UINT64_C(198412698412698413), -18  - 3 }, // * x^7
                        T { UINT64_C(275573192239858907), -18  - 5 }, // * x^9
                        T { UINT64_C(250521083854417188), -18  - 7 }, // * x^11
                        T { UINT64_C(160590438368216146), -18  - 9 }, // * x^13
                        T { UINT64_C(764716373181981648), -18 - 12 }, // * x^15
                        T { UINT64_C(281145725434552076), -18 - 14 }, // * x^17
                        T { UINT64_C(822063524662432972), -18 - 17 }, // * x^19
                    };

                #if (defined(__clang__) && (__clang__ < 6))
                #  pragma clang diagnostic pop
                #endif

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
