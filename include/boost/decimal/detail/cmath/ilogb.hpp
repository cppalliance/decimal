// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_ILOGB_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_ILOGB_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <type_traits>
#include <cmath>

namespace boost { namespace decimal {

// TODO(mborland): Allow conversion between decimal types via a promotion system

template<typename T, std::enable_if_t<detail::is_decimal_floating_point_v<T>, bool> = true>
constexpr auto ilogb(T d) noexcept -> int
{
    auto expval = int { };

    static_cast<void>(frexp10d32(d, &expval));

    return expval;
}

}} // Namespaces

#endif // BOOST_DECIMAL_DETAIL_CMATH_ILOGB_HPP
