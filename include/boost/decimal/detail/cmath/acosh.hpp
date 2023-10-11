// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_ACOSH_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_ACOSH_HPP

#include <array>
#include <type_traits>

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/numbers.hpp>

namespace boost { namespace decimal {

template<typename T>
constexpr auto acosh(T x) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T> // NOLINT(misc-no-recursion)
{
    const auto fpc = fpclassify(x);

    auto result = -std::numeric_limits<T>::quiet_NaN();

    if (fpc != FP_NORMAL)
    {
        if ((fpc == FP_INFINITE) && (!signbit(x)))
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
        constexpr T one  { 1, 0 };

        if (x < one)
        {
            // In this case, acosh(x) for x < 1 retains the initial value of -NaN.
        }
        else if (x > one)
        {

            constexpr T two { 2, 0 };

            if (x < T { 15, -1 })
            {
               // This is just a rearrangement of the standard form below
               // devised to minimise loss of precision when x ~ 1:
               const auto y = x - one;

               result = log1p(y + sqrt(y * y + two * y));
            }
            else if (x < T { 16, -1 })
            {
                // Use a Taylor series expansion of ArcCosh[x] for large argument.
                //   Series[ArcCosh[x], {x, Infinity, 48}]
                //   N[%, 48]

                const auto xm1 = x - one;

                using coefficient_array_type = std::array<T, static_cast<std::size_t>(UINT8_C(25))>;

                constexpr coefficient_array_type
                    coefficient_table
                    {
                        T { INT64_C(-833'333'333'333'333'333), -18 - 1 }, // * (x - 1)^1
                        T { INT64_C(+187'500'000'000'000'000), -18 - 1 }, // * (x - 1)^2
                        T { INT64_C(-558'035'714'285'714'286), -18 - 2 }, // * (x - 1)^3
                        T { INT64_C(+189'887'152'777'777'778), -18 - 2 }, // * (x - 1)^4
                        T { INT64_C(-699'129'971'590'909'091), -18 - 3 }, // * (x - 1)^5
                        T { INT64_C(+271'136'944'110'576'923), -18 - 3 }, // * (x - 1)^6
                        T { INT64_C(-109'100'341'796'875'000), -18 - 3 }, // * (x - 1)^7
                        T { INT64_C(+451'242'222'505'457'261), -18 - 4 }, // * (x - 1)^8
                        T { INT64_C(-190'656'436'117'071'854), -18 - 4 }, // * (x - 1)^9
                        T { INT64_C(+193'687'314'078'921'363), -18 - 5 }, // * (x - 1)^10
                        T { INT64_C(-357'056'927'421'818'609), -18 - 5 }, // * (x - 1)^11
                        T { INT64_C(+157'402'595'505'118'370), -18 - 5 }, // * (x - 1)^12
                        T { INT64_C(-700'688'192'241'445'736), -18 - 6 }, // * (x - 1)^13
                        T { INT64_C(+314'533'061'665'033'215), -18 - 6 }, // * (x - 1)^14
                        T { INT64_C(-142'216'292'935'641'362), -18 - 6 }, // * (x - 1)^15
                        T { INT64_C(+647'111'067'761'133'282), -18 - 7 }, // * (x - 1)^16
                        T { INT64_C(-296'094'097'811'711'825), -18 - 7 }, // * (x - 1)^17
                        T { INT64_C(+136'154'380'562'817'938), -18 - 7 }, // * (x - 1)^18
                        T { INT64_C(-628'864'193'625'161'122), -18 - 8 }, // * (x - 1)^19
                        T { INT64_C(+291'616'597'104'838'435), -18 - 8 }, // * (x - 1)^20
                        T { INT64_C(-135'716'362'052'390'202), -18 - 8 }, // * (x - 1)^21
                        T { INT64_C(+633'685'740'997'145'160), -18 - 9 }, // * (x - 1)^22
                        T { INT64_C(-296'765'408'306'942'403), -18 - 9 }, // * (x - 1)^23
                        T { INT64_C(+139'361'136'681'555'223), -18 - 9 }, // * (x - 1)^24
                        T { INT64_C(-656'090'370'926'302'137), -18 - 10 } // * (x - 1)^25
                    };

                auto rit = coefficient_table.crbegin() + static_cast<std::size_t>((sizeof(T) == 4U) ? 12U : 0U);

                result = *rit;

                while(rit != coefficient_table.crend())
                {
                    result = fma(result, xm1, *rit++);
                }

                result = fma(result, xm1, one);

                result *= sqrt(two * xm1);
            }
            else
            {
                // Use a Taylor series expansion of ArcCosh[x] for large argument.
                //   Series[ArcCosh[x], {x, Infinity, 48}]
                //   N[%, 48]

                const auto one_over_xsq = one / (x * x);

                using coefficient_array_type = std::array<T, static_cast<std::size_t>(UINT8_C(24))>;

                constexpr coefficient_array_type
                    coefficient_table
                    {
                        T { -25, -2 },                                    // didided-by x^2
                        T { -9375, -5 },                                  // didided-by x^4
                        T { INT64_C(-520'833'333'333'333'333), -18 - 1 }, // didided-by x^6
                        T { INT32_C(-341'796'875), -10 },                 // didided-by x^8
                        T { INT32_C( -24'609'375), -9 },                  // didided-by x^10
                        T { -0.000244140625L } * UINT8_C(77),             // didided-by x^12
                        T { INT64_C(149623325892857143), -18 - 1 },       // didided-by x^14
                        T { -0.0000019073486328125L } * UINT16_C(6435),   // didided-by x^16
                        T { INT64_C(-103'039'211'697'048'611), -18 - 1 }, // didided-by x^18
                        T { INT64_C(-880'985'260'009'765'625), -18 - 2 }, // didided-by x^20
                        T { INT64_C(-764'491'341'330'788'352), -18 - 2 }, // didided-by x^22
                        T { INT64_C(-671'584'407'488'505'046), -18 - 2 }, // didided-by x^24
                        T { INT64_C(-596'080'835'048'968'975), -18 - 2 }, // didided-by x^26
                        T { INT64_C(-533'735'645'668'847'220), -18 - 2 }, // didided-by x^28
                        T { INT64_C(-481'548'160'314'559'937), -18 - 2 }, // didided-by x^30
                        T { INT64_C(-437'343'544'035'684'317), -18 - 2 }, // didided-by x^32
                        T { INT64_C(-399'511'057'527'407'127), -18 - 2 }, // didided-by x^34
                        T { INT64_C(-366'834'998'809'887'717), -18 - 2 }, // didided-by x^36
                        T { INT64_C(-338'382'422'724'910'277), -18 - 2 }, // didided-by x^38
                        T { INT64_C(-313'426'719'048'948'143), -18 - 2 }, // didided-by x^40
                        T { INT64_C(-291'394'455'351'629'793), -18 - 2 }, // didided-by x^42
                        T { INT64_C(-271'827'678'907'563'743), -18 - 2 }, // didided-by x^44
                        T { INT64_C(-254'356'712'777'398'966), -18 - 2 }, // didided-by x^46
                        T { INT64_C(-238'680'213'986'430'800), -18 - 2 }, // didided-by x^48
                    };

                auto rit = coefficient_table.crbegin() + static_cast<std::size_t>((sizeof(T) == 4U) ? 10U : 0U);

                result = *rit;

                while(rit != coefficient_table.crend())
                {
                    result = fma(result, one_over_xsq, *rit++);
                }

                result = fma(result, one_over_xsq, log( two * x));
            }
        }
        else
        {
            // This is acosh of 1.
            result = T { 0, 0 };
        }
    }

    return result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_ACOSH_HPP
