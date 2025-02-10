// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_COSH_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_COSH_HPP

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/cmath/impl/cosh_impl.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/numbers.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <array>
#include <type_traits>
#endif

namespace boost {
namespace decimal {

namespace detail {

template <typename T>
constexpr auto cosh_impl(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    const auto fpc = fpclassify(x);

    const T zero { 0, 0 };
    const T one  { 1, 0 };

    auto result = zero;

    if (fpc == FP_ZERO)
    {
        result = one;
    }
    #ifndef BOOST_DECIMAL_FAST_MATH
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
    #endif
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
                const auto xsq = x * x;

                result = detail::cosh_series_expansion(xsq);

                result = fma(result, xsq, one);
            }
            else
            {
                const auto exp_pos_val = exp(x);

                constexpr T two { 2, 0 };

                result = (exp_pos_val + (one / exp_pos_val)) / two;
            }
        }
    }

    return result;
}

} // namespace detail

BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto cosh(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    #if BOOST_DECIMAL_DEC_EVAL_METHOD == 0

    using evaluation_type = T;

    #elif BOOST_DECIMAL_DEC_EVAL_METHOD == 1

    using evaluation_type = detail::promote_args_t<T, decimal64>;

    #else // BOOST_DECIMAL_DEC_EVAL_METHOD == 2

    using evaluation_type = detail::promote_args_t<T, decimal128>;

    #endif

    return static_cast<T>(detail::cosh_impl(static_cast<evaluation_type>(x)));
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_COSH_HPP
