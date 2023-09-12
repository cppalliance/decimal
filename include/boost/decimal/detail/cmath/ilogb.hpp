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

template<typename T, std::enable_if_t<detail::is_decimal_floating_point_v<T>, bool>>
constexpr auto ilogb(T d) noexcept -> int
{
    const auto offset = int { detail::num_digits(d.full_significand()) - 1 };

    auto e10 = int { static_cast<int>(d.full_exponent()) + static_cast<int>(offset - detail::bias) };

    if (offset == 0)
    {
        --e10;
    }

    return e10;
}

}} // Namespaces

#endif // BOOST_DECIMAL_DETAIL_CMATH_ILOGB_HPP
