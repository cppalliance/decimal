// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_EXP_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_EXP_HPP

#include <array>
#include <type_traits>

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/numbers.hpp>

namespace boost { namespace decimal {

template<typename T>
constexpr auto exp(T x) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T> // NOLINT(misc-no-recursion)
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
            if (signbit(x))
            {
                result = zero;
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
        if (signbit(x))
        {
            result = one / exp(-x);
        }
        else
        {
            // Scale the argument to 0 < x < log(2).

            int nf2 { };

            if (x > numbers::ln2_v<T>)
            {
                nf2 = int(x / numbers::ln2_v<T>);

                x -= (numbers::ln2_v<T> * nf2);
            }

            // Specifically derive a polynomial expansion for Exp[x] - 1 for this work.
            //   Table[{x, Exp[x] - 1}, {x, -Log[2], Log[2], 1/120}]
            //   N[%, 48]
            //   Fit[%, {x, x^2, x^3, x^4, x^5, x^6, x^7, x^8, x^9, x^10, x^11, x^12, x^13, x^14}, x]

            // 0.10000000000000000031253715431509025238419040635E+01 x + 
            // 0.49999999999999999984496573772883553268579270080E+00 x^2 + 
            // 0.16666666666666640990495014803355939249488014570E+00 x^3 + 
            // 0.41666666666666669211628619573955168058871592450E-01 x^4 + 
            // 0.83333333333393951785026963540146404509293729880E-02 x^5 + 
            // 0.13888888888889531930153627330611258989144651196E-02 x^6 + 
            // 0.19841269834993605726436085548163823346843735024E-03 x^7 + 
            // 0.24801587300034818820409788738954033986515647940E-04 x^8 + 
            // 0.27557322544760672800374557020710996254381528860E-05 x^9 + 
            // 0.27557320418080198508304620049466379620223056430E-06 x^10 + 
            // 0.25051171110638608777162102769329794003047092510E-07 x^11 + 
            // 0.20876329425290606504750348960465969428397241893E-08 x^12 + 
            // 0.16193258511277332147199978827602163814705181995E-09 x^13 + 
            // 0.11543684568750446489173904407189863158196792345E-10 x^14

            using coefficient_array_type = std::array<T, static_cast<std::size_t>(UINT8_C(14))>;

            // TODO(ckormanyos) Is a Pade approximation more precise or faster?
            // And, ... how can you make it scalable if you go ahead and "Pade"-it?

            constexpr coefficient_array_type
                coefficient_table
                {
                    T { 1,  0 },                                       // * x
                    T { 5, -1 },                                       // * x^2
                    T { UINT64_C(166'666'666'666'666'409), -18 -  0 }, // * x^3
                    T { UINT64_C(416'666'666'666'666'692), -18 -  1 }, // * x^4
                    T { UINT64_C(833'333'333'333'939'517), -18 -  2 }, // * x^5
                    T { UINT64_C(138'888'888'888'895'319), -18 -  2 }, // * x^6
                    T { UINT64_C(198'412'698'349'936'057), -18 -  3 }, // * x^7
                    T { UINT64_C(248'015'873'000'348'188), -18 -  4 }, // * x^8
                    T { UINT64_C(275'573'225'447'606'728), -18 -  5 }, // * x^9
                    T { UINT64_C(275'573'204'180'801'985), -18 -  6 }, // * x^10
                    T { UINT64_C(250'511'711'106'386'087), -18 -  7 }, // * x^11
                    T { UINT64_C(208'763'294'252'906'065), -18 -  8 }, // * x^12
                    T { UINT64_C(161'932'585'112'773'321), -18 -  9 }, // * x^13
                    T { UINT64_C(115'436'845'687'504'464), -18 - 10 }  // * x^14
                };

            auto rit = coefficient_table.crbegin() + static_cast<std::size_t>((sizeof(T) == 4U) ? 5U : 0U);

            result = *rit;

            while(rit != coefficient_table.crend())
            {
                result = fma(result, x, *rit++);
            }

            result = fma(result, x, one);

            if (nf2 > 0)
            {
                if (nf2 < 64)
                {
                    result *= T { static_cast<std::uint64_t>(1ULL << static_cast<unsigned>(nf2)), 0 };
                }
                else
                {
                    result *= pow(T { 2, 0 }, nf2);
                }
            }
        }
    }

    return result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_EXP_HPP
