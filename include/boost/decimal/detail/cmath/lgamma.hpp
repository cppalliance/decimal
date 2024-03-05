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

                // Perform the Laurent series expansion.
                const auto one_over_x = one / x;

                result = detail::lgamma_laurent_series_expansion(one_over_x);

                result = (x * (log(x) - one)) + log(result / sqrt(x));
            }
        }
    }

    return result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_LGAMMA_HPP
