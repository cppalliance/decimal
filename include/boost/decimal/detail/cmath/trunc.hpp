// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_TRUNC_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_TRUNC_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/cmath/floor.hpp>
#include <boost/decimal/detail/cmath/ceil.hpp>
#include <type_traits>
#include <cmath>

namespace boost { namespace decimal {

template<typename T>
constexpr auto trunc(T val) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>
{
    return (val > 0) ? floor(val) : ceil(val);
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_TRUNC_HPP
