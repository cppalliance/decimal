// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_EXPM1_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_EXPM1_HPP

#include <array>
#include <type_traits>

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/numbers.hpp>

namespace boost {
namespace decimal {

template <typename T>
constexpr auto expm1(T x) noexcept
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
                result = -one;
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
    else
    {
        if (abs(x) > numbers::ln2_v<T>)
        {
            if (signbit(x))
            {
                result = one / exp(-x) - one;
            }
            else
            {
                result = exp(x) - one;
            }
        }
        else
        {
            // Specifically derive a polynomial expansion for Exp[x] - 1 for this work.
            //   Table[{x, Exp[x] - 1}, {x, -Log[2], Log[2], 1/60}]
            //   N[%, 48]
            //   Fit[%, {x, x^2, x^3, x^4, x^5, x^6, x^7, x^8, x^9, x^10, x^11, x^12, x^13, x^14}, x]

            //   0.1000000000000000003213692169066381945529176657E+01 x
            // + 0.4999999999999999998389405741198241227273662223E+00 x^2
            // + 0.1666666666666664035765593562709186076539985328E+00 x^3
            // + 0.4166666666666666934614928838666442575683452206E-01 x^4
            // + 0.8333333333339521841328202617206868796855583809E-02 x^5
            // + 0.1388888888888953513176946682731620625302469979E-02 x^6
            // + 0.1984126983488689186859793276462049624439889135E-03 x^7
            // + 0.2480158730001499149369647648735612017495156006E-04 x^8
            // + 0.2755732258782898252481007286813761544775538366E-05 x^9
            // + 0.2755732043147979013276287368071846972098889744E-06 x^10
            // + 0.2505116286861719378770371641094067075234027345E-07 x^11
            // + 0.2087632598463662328337672597832718168295672334E-08 x^12
            // + 0.1619385892296180390338553597911165126625722485E-09 x^13
            // + 0.1154399218598221557485183048765404442959841646E-10 x^14

            using coefficient_array_type = std::array<T, static_cast<std::size_t>(UINT8_C(14))>;

            #if (defined(__clang__) && (__clang__ < 6))
            #  pragma clang diagnostic push
            #  pragma clang diagnostic ignored "-Wmissing-braces"
            #endif

            constexpr auto coefficient_table =
                coefficient_array_type
                {
                    T { UINT64_C(100000000000000000), -17 -  0 }, // * x
                    T { UINT64_C(500000000000000000), -18 -  0 }, // * x^2
                    T { UINT64_C(166666666666666404), -18 -  0 }, // * x^3
                    T { UINT64_C(416666666666666693), -18 -  1 }, // * x^4
                    T { UINT64_C(833333333333952184), -18 -  2 }, // * x^5
                    T { UINT64_C(138888888888895351), -18 -  2 }, // * x^6
                    T { UINT64_C(198412698348868919), -18 -  3 }, // * x^7
                    T { UINT64_C(248015873000149915), -18 -  4 }, // * x^8
                    T { UINT64_C(275573225878289825), -18 -  5 }, // * x^9
                    T { UINT64_C(275573204314797901), -18 -  6 }, // * x^10
                    T { UINT64_C(250511628686171938), -18 -  7 }, // * x^11
                    T { UINT64_C(208763259846366233), -18 -  8 }, // * x^12
                    T { UINT64_C(161938589229618039), -18 -  9 }, // * x^13
                    T { UINT64_C(115439921859822156), -18 - 10 }  // * x^14
                };

            #if (defined(__clang__) && (__clang__ < 6))
            #  pragma clang diagnostic pop
            #endif

            auto rit = coefficient_table.crbegin() + static_cast<std::size_t>((sizeof(T) == 4U) ? 5U : 0U);

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

#endif // BOOST_DECIMAL_DETAIL_CMATH_EXPM1_HPP
