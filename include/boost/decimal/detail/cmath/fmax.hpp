// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_FMAX_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_FMAX_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <type_traits>
#include <cmath>

namespace boost { namespace decimal {

// TODO(mborland): Allow conversion between decimal types via a promotion system

template<typename T, std::enable_if_t<detail::is_decimal_floating_point_v<T>, bool> = true>
constexpr auto fmax(T lhs, T rhs) noexcept -> T
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

    return lhs > rhs ? lhs : rhs;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_FMAX_HPP
