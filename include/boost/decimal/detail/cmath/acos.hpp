// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_ACOS_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_ACOS_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/numbers.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/cmath/fabs.hpp>
#include <boost/decimal/detail/cmath/sqrt.hpp>
#include <boost/decimal/detail/cmath/impl/asin_impl.hpp>
#include <type_traits>
#include <cstdint>

namespace boost {
namespace decimal {

template <typename T>
constexpr auto acos(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    if (isnan(x))
    {
        return x;
    }

    constexpr T half_pi {numbers::pi_v<T> / 2};
    const auto absx {fabs(x)};
    T result {};

    if (absx > 1)
    {
        result = std::numeric_limits<T>::signaling_NaN();
    }
    else if (x < T{5, -1, true})
    {
        result = numbers::pi_v<T> - 2 * detail::asin_impl(sqrt((1 - absx) / 2));
    }
    else if (x < -std::numeric_limits<T>::epsilon())
    {
        result = half_pi + detail::asin_impl(absx);
    }
    else if (x < T{5, -1})
    {
        result = half_pi - detail::asin_impl(x);
    }
    else
    {
        result = half_pi - (half_pi - 2 * detail::asin_impl(sqrt((1 - x) / 2)));
    }

    return result;
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_ACOS_HPP
