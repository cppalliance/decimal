// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_LOG1P_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_LOG1P_HPP

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/cmath/impl/log1p_impl.hpp>
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

BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto log1p(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    constexpr T one { 1, 0 };

    T result { };

    const auto fpc = fpclassify(x);

    if (fpc == FP_ZERO)
    {
        result = x;
    }
    else if (fpc != FP_NORMAL)
    {
        result =
        (
            ((fpc == FP_INFINITE) && signbit(x)) ? std::numeric_limits<T>::quiet_NaN() : x
        );
    }
    else if (-x >= one)
    {
        result =
        (
            (-x == one) ? -std::numeric_limits<T>::infinity() : std::numeric_limits<T>::quiet_NaN()
        );
    }
    else
    {
        if (x > T { 5, -1 })
        {
            result = log(x + one);
        }
        else
        {
            result = x * fma(detail::log1p_series_expansion(x), x, one);
        }
    }

    return result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_LOG1P_HPP
