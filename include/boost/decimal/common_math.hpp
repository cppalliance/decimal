// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_COMMON_MATH_HPP
#define BOOST_DECIMAL_COMMON_MATH_HPP

#include <type_traits>

#include <boost/decimal/decimal32.hpp>

namespace boost { namespace decimal {

// Overloads may be any integral type or,
// TODO(mborland): any decimal type returning the promoted decimal type (e.g. dec32 + dec64 yields dec64)

template <typename T, std::enable_if_t<detail::is_integral<T>::value, bool> = true>
constexpr auto operator*(decimal32 lhs, T rhs) noexcept -> decimal32
{
    return lhs * static_cast<decimal32>(rhs);
}

template <typename T, std::enable_if_t<detail::is_integral<T>::value, bool> = true>
constexpr auto operator*(T lhs, decimal32 rhs) noexcept -> decimal32
{
    return static_cast<decimal32>(lhs) * rhs;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_COMMON_MATH_HPP
