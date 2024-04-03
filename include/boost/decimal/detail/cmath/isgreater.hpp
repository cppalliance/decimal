// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_ISGREATER_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_ISGREATER_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/config.hpp>
#include <type_traits>
#include <cmath>

namespace boost {
namespace decimal {

BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto isgreater(T lhs, T rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, T, bool)
{
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }

    return lhs > rhs;
}

BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto isgreaterequal(T lhs, T rhs) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, T, bool)
{
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }

    return lhs >= rhs;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_ISGREATER_HPP
