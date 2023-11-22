// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_FMIN_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_FMIN_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <type_traits>
#include <cmath>

namespace boost {
namespace decimal {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto fmin(T lhs, T rhs) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>
{
    if (isnan(lhs) && !isnan(rhs))
    {
        return rhs;
    }
    else if ((!isnan(lhs) && isnan(rhs)) ||
             (isnan(lhs) && isnan(rhs)))
    {
        return lhs;
    }

    return lhs < rhs ? lhs : rhs;
}

} // namespace decimal
} // namespace boost

#endif //BOOST_FMIN_HPP
