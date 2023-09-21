// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_SQRT_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_SQRT_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <type_traits>
#include <cstdint>
#include <cmath>

namespace boost { namespace decimal {

// Bakhsali Approximation
template <typename T>
constexpr auto sqrt(T val) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>
{
    std::uint64_t i {};
    while ((i * i) <= val)
    {
        ++i;
    }

    --i;
    const decimal32 d {val - (i * i)};
    const decimal32 p {d / (2 * i)};
    const decimal32 a {i + p};

    return a - (p * p) / (2 * a);
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_SQRT_HPP

