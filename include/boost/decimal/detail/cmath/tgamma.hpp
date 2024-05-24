// Copyright 2023 - 2024 Matt Borland
// Copyright 2023 - 2024 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_TGAMMA_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_TGAMMA_HPP

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/cmath/impl/tgamma_impl.hpp>
#include <boost/decimal/detail/cmath/sin.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/numbers.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <iterator>
#include <limits>
#endif

namespace boost {
namespace decimal {

namespace detail {

template <typename T>
constexpr auto tgamma_impl(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    T result { };

    const auto nx = static_cast<int>(x);

    const auto is_pure_int = (nx == x);

    const auto fpc = fpclassify(x);

    if (fpc != FP_NORMAL)
    {
        if (fpc == FP_ZERO)
        {
            result = (signbit(x) ? -std::numeric_limits<T>::infinity() : std::numeric_limits<T>::infinity());
        }
        else if(fpc == FP_INFINITE)
        {
            result = (signbit(x) ? std::numeric_limits<T>::quiet_NaN() : std::numeric_limits<T>::infinity());
        }
        else
        {
            result = x;
        }
    }
    else if ((nx < 0) && is_pure_int && ((nx & 1) != 0))
    {
        // Pure negative integer argument.
        result = std::numeric_limits<T>::quiet_NaN();
    }
    else
    {
        if (signbit(x))
        {
            // Reflection for negative argument.
            const auto ga = tgamma(-x);

            result = -numbers::pi_v<T> / ((x * ga) * sin(numbers::pi_v<T> * x));
        }
        else
        {
            constexpr T one { 1, 0 };

            if (is_pure_int)
            {
                result = one;

                for(auto index = 2; index < nx; ++index)
                {
                    result *= index;
                }
            }
            else
            {
                constexpr int asymp_cutoff
                {
                      std::numeric_limits<T>::digits10 < 10 ? T {  2, 1 } //  20
                    : std::numeric_limits<T>::digits10 < 20 ? T {  5, 1 } //  50
                    :                                         T { 15, 1 } // 150
                };

                if (x < T { asymp_cutoff })
                {
                    T r { 1 };

                    T z { x };

                    // Use small-argument Taylor series expansion
                    // (with scaling for arguments greater than one).

                    for(auto k = 1; k <= nx; ++k)
                    {
                        r *= (z - k);
                    }

                    z -= nx;

                    result = r / (z * fma(detail::tgamma_series_expansion(z), z, one));
                }
                else
                {
                    // Use large-argument asymptotic expansion.

                    const T prefix { exp(((x  - T { 5, -1 }) * log(x)) - x) };

                    result =  prefix * detail::tgamma_series_expansion_asymp(one / x);
                }
            }
        }
    }

    return result;
}

} // namespace detail

BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto tgamma(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    #if BOOST_DECIMAL_DEC_EVAL_METHOD == 0

    using evaluation_type = T;

    #elif BOOST_DECIMAL_DEC_EVAL_METHOD == 1

    using evaluation_type = detail::promote_args_t<T, decimal64>;

    #else // BOOST_DECIMAL_DEC_EVAL_METHOD == 2

    using evaluation_type = detail::promote_args_t<T, decimal128>;

    #endif

    return static_cast<T>(detail::tgamma_impl(static_cast<evaluation_type>(x)));
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_TGAMMA_HPP
