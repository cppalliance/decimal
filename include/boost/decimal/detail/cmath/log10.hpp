// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_LOG10_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_LOG10_HPP

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/cmath/impl/log_impl.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/numbers.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <cmath>
#include <type_traits>
#endif

namespace boost {
namespace decimal {

namespace detail {

template <typename T>
constexpr auto log10_impl(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    constexpr T zero { 0, 0 };
    constexpr T one  { 1, 0 };

    T result { };

    const auto fpc = fpclassify(x);

    if (fpc == FP_ZERO)
    {
        result = -std::numeric_limits<T>::infinity();
    }
    else if (signbit(x) || (fpc == FP_NAN))
    {
        result = std::numeric_limits<T>::quiet_NaN();
    }
    else if (fpc == FP_INFINITE)
    {
        result = std::numeric_limits<T>::infinity();
    }
    else
    {
        int exp10val { };

        auto gn { frexp10(x, &exp10val) };

        const auto zeros_removal_result { detail::remove_trailing_zeros(gn) };

        const bool is_pure { (zeros_removal_result.trimmed_number == one) };

        if(is_pure)
        {
            // Here, a pure power-of-10 argument gets a pure integral result.
            result =
                T
                {
                      exp10val
                    + static_cast<int>(zeros_removal_result.number_of_removed_zeros)
                };
        }
        else
        {
            if (x < one)
            {
                // Handle reflection, the [+/-] zero-pole, and non-pole, negative x.
                if (x > zero)
                {
                    result = -log10(one / x);
                }
                else if ((x == zero) || (-x == zero))
                {
                    result = -std::numeric_limits<T>::infinity();
                }
                else
                {
                    result = std::numeric_limits<T>::quiet_NaN();
                }
            }
            else if(x > one)
            {
                // The algorithm for base-10 logarithm is based on Chapter 5, pages 35-36
                // of Cody and Waite, Software Manual for the Elementary Functions,
                // Prentice Hall, 1980.

                T g { T { gn } / pow10(std::numeric_limits<T>::digits10) };

                exp10val += std::numeric_limits<T>::digits10;

                constexpr T inv_sqrt10 { UINT64_C(3162277660168379332), -19 };

                const bool reduce_sqrt10 { g < inv_sqrt10 };

                if (reduce_sqrt10)
                {
                    constexpr T sqrt10 { UINT64_C(3162277660168379332), -18 };

                    g *= sqrt10;
                }

                const T s   { (g - one) / (g + one) };
                const T z   { s + s };
                const T zsq { z * z };

                result = z * fma(detail::log_series_expansion(zsq), zsq, one);

                result /= numbers::ln10_v<T>;

                if(reduce_sqrt10)
                {
                    constexpr T half { 5, -1 };

                    result -= half;
                }

                result += static_cast<T>(exp10val);
            }
            else
            {
                result = zero;
            }
        }
    }

    return result;
}

} //namespace detail

BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto log10(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    #if BOOST_DECIMAL_DEC_EVAL_METHOD == 0

    using evaluation_type = T;

    #elif BOOST_DECIMAL_DEC_EVAL_METHOD == 1

    using evaluation_type = detail::promote_args_t<T, decimal64>;

    #else // BOOST_DECIMAL_DEC_EVAL_METHOD == 2

    using evaluation_type = detail::promote_args_t<T, decimal128>;

    #endif

    return static_cast<T>(detail::log10_impl(static_cast<evaluation_type>(x)));
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_LOG10_HPP
