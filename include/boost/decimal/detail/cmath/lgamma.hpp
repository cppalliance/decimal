// Copyright 2023 - 2024 Matt Borland
// Copyright 2023 - 2024 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_LGAMMA_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_LGAMMA_HPP

#include <array>
#include <type_traits>

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/cmath/impl/lgamma_impl.hpp>
#include <boost/decimal/detail/cmath/impl/taylor_series_result.hpp>
#include <boost/decimal/detail/cmath/abs.hpp>
#include <boost/decimal/detail/cmath/log.hpp>
#include <boost/decimal/detail/cmath/sin.hpp>
#include <boost/decimal/detail/cmath/sqrt.hpp>
#include <boost/decimal/detail/cmath/tgamma.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/numbers.hpp>

namespace boost {
namespace decimal {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto lgamma(T x) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T> // NOLINT(misc-no-recursion)
{
    T result { };

    const auto nx = static_cast<int>(x);

    const auto is_pure_int = (nx == x);

    const auto fpc = fpclassify(x);

    if (fpc != FP_NORMAL)
    {
      if ((fpc == FP_ZERO) || (fpc == FP_INFINITE))
      {
          result = std::numeric_limits<T>::infinity();
      }
      else
      {
          result = x;
      }
    }
    else if ((is_pure_int) && (nx < 0))
    {
        // Pure negative integer argument.
        result = std::numeric_limits<T>::infinity();
    }
    else if ((is_pure_int) && ((nx == 1) || (nx == 2)))
    {
        constexpr T zero { 0, 0 };

        result = zero;
    }
    else
    {
        constexpr T one { 1, 0 };

        if (signbit(x))
        {
            // Reflection for negative argument.
            const auto za = -x + one;

            const auto phase = sin(numbers::pi_v<T> * za);

            result = log(numbers::pi_v<T>) - log(abs(phase)) - lgamma(za);
        }
        else
        {
            constexpr T half         { 5, -1 };
            constexpr T asymp_cutoff { 8, 0 };

            if (x < half)
            {
                // Perform the Taylor series expansion.
                //result = detail::taylor_series_result(x, coefficient_table);
                result = detail::lgamma_taylor_series_expansion(x);

                result = x * fma(result, x, -numbers::egamma_v<T>);

                result -= log(x);
            }
            else if (x < asymp_cutoff)
            {
                result = log(tgamma(x));
            }
            else
            {
                using coefficient_array_type = std::array<T, static_cast<std::size_t>(UINT8_C(26))>;

                #if (defined(__clang__) && (__clang__ < 6))
                #  pragma clang diagnostic push
                #  pragma clang diagnostic ignored "-Wmissing-braces"
                #endif

                constexpr auto coefficient_table =
                    coefficient_array_type
                    {
                        // Use a Laurent infinite-series expansion of the logarithm of the
                        // gamma function divided by the square root.
                        // N[Series[Log[Gamma[x]], {x, Infinity, 26}], 32]
                        // log( e^-x * x^x * sqrt(x) * [Series below] )
                        // See also Wolfram Alpha(R): https://www.wolframalpha.com/input?i=Series%5BLog%5BGamma%5Bx%5D%5D%2C+%7Bx%2C+Infinity%2C+4%7D%5D
                          T { UINT64_C(250'662'827'463'100'050), + 1 - 18 }, // * x^-1
                        + T { UINT64_C(208'885'689'552'583'375), - 0 - 18 }, // * x^-2
                        + T { UINT64_C(870'357'039'802'430'730), - 2 - 18 }, // * x^-3
                        - T { UINT64_C(672'109'047'402'988'175), - 2 - 18 }, // * x^-4
                        - T { UINT64_C(575'201'238'110'171'235), - 3 - 18 }, // * x^-5
                        + T { UINT64_C(196'529'488'158'320'306), - 2 - 18 }, // * x^-6
                        + T { UINT64_C(174'782'521'204'559'121), - 3 - 18 }, // * x^-7
                        - T { UINT64_C(148'434'113'515'827'614), - 2 - 18 }, // * x^-8
                        - T { UINT64_C(129'637'573'211'255'432), - 3 - 18 }, // * x^-9
                        + T { UINT64_C(210'431'122'975'320'637), - 2 - 18 }, // * x^-10
                        + T { UINT64_C(180'599'945'655'550'436), - 3 - 18 }, // * x^-11
                        - T { UINT64_C(479'878'567'054'634'606), - 2 - 18 }, // * x^-12
                        - T { UINT64_C(407'367'859'381'525'183), - 3 - 18 }, // * x^-13
                        + T { UINT64_C(160'508'503'319'445'960), - 1 - 18 }, // * x^-14
                        + T { UINT64_C(135'399'228'015'909'411), - 2 - 18 }, // * x^-15
                        - T { UINT64_C(740'154'212'684'273'819), - 1 - 18 }, // * x^-16
                        - T { UINT64_C(622'080'867'880'877'866), - 2 - 18 }, // * x^-17
                        + T { UINT64_C(450'040'333'856'250'984), - 0 - 18 }, // * x^-18
                        + T { UINT64_C(377'400'786'521'707'440), - 1 - 18 }, // * x^-19
                        - T { UINT64_C(348'872'797'304'123'310), + 1 - 18 }, // * x^-20
                        - T { UINT64_C(292'138'192'227'179'792), - 0 - 18 }, // * x^-21
                        + T { UINT64_C(335'837'691'649'553'084), + 2 - 18 }, // * x^-22
                        + T { UINT64_C(280'944'016'052'174'395), + 1 - 18 }, // * x^-23
                        - T { UINT64_C(393'042'854'585'987'930), + 3 - 18 }, // * x^-24
                        - T { UINT64_C(328'565'400'725'242'178), + 2 - 18 }, // * x^-25
                        + T { UINT64_C(549'592'170'046'323'711), + 4 - 18 }, // * x^-26
                    };

                #if (defined(__clang__) && (__clang__ < 6))
                #  pragma clang diagnostic pop
                #endif

                // Perform the Laurent series expansion.
                const auto one_over_x = one / x;

                result = detail::taylor_series_result(one_over_x, coefficient_table);

                result = (x * (log(x) - one)) + log(result / sqrt(x));
            }
        }
    }

    return result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_LGAMMA_HPP
