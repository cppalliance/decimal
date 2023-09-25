// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_RINT_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_RINT_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/fenv_rounding.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <boost/decimal/detail/cmath/floor.hpp>
#include <boost/decimal/detail/cmath/ceil.hpp>
#include <boost/decimal/detail/cmath/trunc.hpp>
#include <boost/decimal/detail/cmath/frexp10.hpp>
#include <type_traits>
#include <limits>
#include <cmath>

namespace boost {
namespace decimal {

// Rounds the number using the default rounding mode
template <typename T>
constexpr auto rint(T num) -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>
{
    constexpr T zero {0, 0};

    if (isinf(num) || isnan(num) || abs(num) == zero)
    {
        return num;
    }

    int expptr {};
    auto sig {frexp10(num, &expptr)}; // Always returns detail::precision digits
    const bool is_neg {num < 0};

    if (expptr > detail::precision)
    {
        return num;
    }
    else if (expptr < -detail::precision)
    {
        return is_neg ? -zero : zero;
    }

    sig /= detail::pow10<std::uint32_t>(std::abs(expptr) - 1);
    detail::fenv_round(sig, is_neg);

    return {sig, 0, is_neg};
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_RINT_HPP
