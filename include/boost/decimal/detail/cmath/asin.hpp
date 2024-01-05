// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_ASIN_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_ASIN_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/numbers.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/cmath/fpclassify.hpp>
#include <boost/decimal/detail/cmath/fabs.hpp>
#include <boost/decimal/detail/cmath/sqrt.hpp>
#include <boost/decimal/detail/cmath/impl/asin_impl.hpp>
#include <type_traits>
#include <cstdint>

namespace boost {
namespace decimal {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto asin(T x) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T> // NOLINT(misc-no-recursion)
{
    const auto fpc {fpclassify(x)};
    const auto isneg {signbit(x)};

    if (fpc == FP_ZERO || fpc == FP_NAN)
    {
        return x;
    }

    const auto absx {fabs(x)};
    T result {};

    if (absx <= std::numeric_limits<T>::epsilon())
    {
        result = absx;
    }
    else if (absx <= T{5, -1})
    {
        result = detail::asin_impl(absx);
    }
    else if (absx <= T{1, 0})
    {
        constexpr T half_pi {numbers::pi_v<T> / 2};
        result = half_pi - 2 * detail::asin_impl(sqrt((1 - absx) / 2));
    }
    else
    {
        result = std::numeric_limits<T>::signaling_NaN();
    }

    // arcsin(-x) == -arcsin(x)
    if (isneg)
    {
        result = -result;
    }

    return result;
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_ASIN_HPP
