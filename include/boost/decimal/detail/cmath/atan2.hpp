// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_ATAN2_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_ATAN2_HPP

#include <boost/decimal/numbers.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/cmath/atan.hpp>
#include <boost/decimal/detail/cmath/fabs.hpp>
#include <type_traits>
#include <cstdint>
#include <cmath>

namespace boost {
namespace decimal {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto atan2(T y, T x) noexcept
{
    const auto fpcx {fpclassify(x)};
    const auto fpcy {fpclassify(y)};
    const auto signx {signbit(x)}; // True if neg
    const auto signy {signbit(y)};
    const auto isfinitex {fpcx != FP_INFINITE && fpcx != FP_NAN};
    const auto isfinitey {fpcy != FP_INFINITE && fpcy != FP_NAN};

    if (fpcx == FP_NAN)
    {
        return x;
    }
    else if (fpcy == FP_NAN)
    {
        return y;
    }
    else if (fpcy == FP_ZERO && signx)
    {
        return signy ? -numbers::pi_v<T> : numbers::pi_v<T>;
    }
    else if (fpcy == FP_ZERO && !signx)
    {
        return y;
    }
    else if (fpcy == FP_INFINITE && isfinitex)
    {
        return signy ? -numbers::pi_v<T> / 2 : numbers::pi_v<T> / 2;
    }
    else if (fpcy == FP_INFINITE && fpcx == FP_INFINITE && signx)
    {
        return signy ? -3 * numbers::pi_v<T> / 4 : 3 * numbers::pi_v<T> / 4;
    }
    else if (fpcy == FP_INFINITE && fpcx == FP_INFINITE && !signx)
    {
        return signy ? -numbers::pi_v<T> / 4 : numbers::pi_v<T> / 4;
    }
    else if (fpcx == FP_ZERO)
    {
        return signy ? -numbers::pi_v<T> / 2 : numbers::pi_v<T> / 2;
    }
    else if (fpcx == FP_INFINITE && signx && isfinitey)
    {
        return signy ? -numbers::pi_v<T> : numbers::pi_v<T>;
    }
    else if (fpcx == FP_INFINITE && !signx && isfinitey)
    {
        return signy ? -T(0) : T(0);
    }

    if (x == T{1, 0})
    {
        return atan(y);
    }

    const auto ret_val {atan(fabs(y / x))};

    if (!signy && !signx)
    {
        return ret_val;
    }
    else if (signy && !signx)
    {
        return -ret_val;
    }
    else if (!signy && signx)
    {
        return numbers::pi_v<T> - ret_val;
    }
    else
    {
        return ret_val - numbers::pi_v<T>;
    }
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_ATAN2_HPP
