// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_ATAN2_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_ATAN2_HPP

#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/cmath/atan.hpp>
#include <boost/decimal/detail/cmath/fabs.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/numbers.hpp>

#include <type_traits>
#include <cstdint>
#include <cmath>

namespace boost {
namespace decimal {

namespace detail {

namespace atan2_detail {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
struct pi_constants
{
    static constexpr T pi_over_two        = numbers::pi_v<T> / 2;
    static constexpr T pi_over_four       = numbers::pi_v<T> / 4;
    static constexpr T three_pi_over_four = (3 * numbers::pi_v<T>) / 4;
};

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T> constexpr T pi_constants<T>::pi_over_two;
template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T> constexpr T pi_constants<T>::pi_over_four;
template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T> constexpr T pi_constants<T>::three_pi_over_four;

} // namespace atan2_detail
} // namespace detail

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto atan2(T y, T x) noexcept
{
    const auto fpcx {fpclassify(x)};
    const auto fpcy {fpclassify(y)};
    const auto signx {signbit(x)}; // True if neg
    const auto signy {signbit(y)};
    const auto isfinitex {fpcx != FP_INFINITE && fpcx != FP_NAN};
    const auto isfinitey {fpcy != FP_INFINITE && fpcy != FP_NAN};

    T result { };

    if (fpcx == FP_NAN)
    {
        result = x;
    }
    else if (fpcy == FP_NAN)
    {
        result = y;
    }
    else if (fpcy == FP_ZERO && signx)
    {
        result = signy ? -numbers::pi_v<T> : numbers::pi_v<T>;
    }
    else if (fpcy == FP_ZERO && !signx)
    {
        result = y;
    }
    else if (fpcy == FP_INFINITE && isfinitex)
    {
        result = detail::atan2_detail::pi_constants<T>::pi_over_two;

        if (signy) { result = -result; }
    }
    else if (fpcy == FP_INFINITE && fpcx == FP_INFINITE && signx)
    {
        result = detail::atan2_detail::pi_constants<T>::three_pi_over_four;

        if (signy) { result = -result; }
    }
    else if (fpcy == FP_INFINITE && fpcx == FP_INFINITE && !signx)
    {
        result = detail::atan2_detail::pi_constants<T>::pi_over_four;

        if (signy) { result = -result; }
    }
    else if (fpcx == FP_ZERO)
    {
        result = detail::atan2_detail::pi_constants<T>::pi_over_two;

        if (signy) { result = -result; }
    }
    else if (fpcx == FP_INFINITE && signx && isfinitey)
    {
        result = signy ? -numbers::pi_v<T> : numbers::pi_v<T>;
    }
    else if (fpcx == FP_INFINITE && !signx && isfinitey)
    {
        constexpr T zero { 0, 0 };

        result = signy ? -zero : zero;
    }
    else
    {
        if (x == T{1, 0})
        {
            result = atan(y);
        }
        else
        {
            const auto ret_val {atan(fabs(y / x))};

            if (!signy && !signx)
            {
                result = ret_val;
            }
            else if (signy && !signx)
            {
                result = -ret_val;
            }
            else if (!signy && signx)
            {
                result = numbers::pi_v<T> - ret_val;
            }
            else
            {
                result = ret_val - numbers::pi_v<T>;
            }
        }
    }

    return result;
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_ATAN2_HPP
