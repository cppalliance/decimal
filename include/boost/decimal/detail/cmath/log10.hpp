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

        using representation_type = std::conditional_t<std::is_same<T, decimal32>::value, std::uint32_t,
                                    std::conditional_t<std::is_same<T, decimal64>::value, std::uint64_t, detail::uint128>>;

        const representation_type gn { frexp10(x, &exp10val) };

        const remove_trailing_zeros_return<representation_type>
            zeros_removal
            {
                remove_trailing_zeros(gn)
            };

        const bool is_pure
        {
            (zeros_removal.trimmed_number == static_cast<representation_type>(UINT8_C(1)))
        };

        if(is_pure)
        {
            // Here, a pure power-of-10 argument gets a pure integral result.
            const int p10 { exp10val + static_cast<int>(zeros_removal.number_of_removed_zeros) };

            result = T { p10 };
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
                // The algorithm for base-10 logarithm is based on Chapter 5,
                // pages 35-36 of Cody and Waite, "Software Manual for the
                // Elementary Functions", Prentice Hall, 1980.

                // In this implementation, however, we use 2s (as for natural
                // logarithm in Cody and Waite) even though we are computing
                // the base-10 logarithm.

                T g { gn, -std::numeric_limits<T>::digits10 };

                exp10val += std::numeric_limits<T>::digits10;

                int reduce_sqrt2 { };

                while (g < numbers::inv_sqrt2_v<T>)
                {
                    g += g;

                    ++reduce_sqrt2;
                }

                const T s   { (g - one) / (g + one) };
                const T z   { s + s };
                const T zsq { z * z };

                result = z * fma(detail::log_series_expansion(zsq), zsq, one);

                result /= numbers::ln10_v<T>;

                for(int i = 0; i < reduce_sqrt2; ++i)
                {
                    result -= numbers::log10_2_v<T>;
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
