//  (C) Copyright John Maddock 2008 - 2023.
//  (C) Copyright Matt Borland 2023.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_DECIMAL_DETAIL_CMATH_NEXT_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_NEXT_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/promotion.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/cmath/modf.hpp>
#include <boost/decimal/detail/cmath/abs.hpp>
#include <boost/decimal/detail/cmath/round.hpp>
#include <boost/decimal/detail/cmath/ilogb.hpp>
#include <boost/decimal/detail/cmath/fpclassify.hpp>
#include <type_traits>
#include <limits>
#include <cstdint>

namespace boost {
namespace decimal {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T2>
constexpr auto nextafter(T1 val, T2 direction) noexcept
    -> std::enable_if_t<(detail::is_decimal_floating_point_v<T1> || detail::is_decimal_floating_point_v<T2>),
                         detail::promote_args_t<T1, T2>>
{
    if (isnan(val) || isinf(val))
    {
        return val;
    }
    else if (isnan(direction) || val == direction)
    {
        return direction;
    }
    else if (val < direction)
    {
        return val + std::numeric_limits<T1>::epsilon();
    }

    return val - std::numeric_limits<T1>::epsilon();
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
BOOST_DECIMAL_CXX20_CONSTEXPR auto nexttoward(T val, long double direction) noexcept
    -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>
{
    const auto dec_direction {static_cast<T>(direction)};
    return nextafter(val, dec_direction);
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_NEXT_HPP