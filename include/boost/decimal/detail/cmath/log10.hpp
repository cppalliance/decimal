// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_LOG10_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_LOG10_HPP

#include <cmath>
#include <type_traits>

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>

namespace boost { namespace decimal {

template<typename T>
constexpr auto log10(T x) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>;
{
    return T();
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_LOG10_HPP
