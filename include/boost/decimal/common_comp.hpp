// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_COMMON_COMP_HPP
#define BOOST_DECIMAL_COMMON_COMP_HPP

#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/config.hpp>
#include <type_traits>

namespace boost { namespace decimal {

template <typename T, std::enable_if_t<detail::is_integral<T>::value, bool> = true>
constexpr bool operator<=(decimal32 lhs, T rhs) noexcept
{
    return lhs <= static_cast<decimal32>(rhs);
}

template <typename T, std::enable_if_t<detail::is_integral<T>::value, bool> = true>
constexpr bool operator<=(T lhs, decimal32 rhs) noexcept
{
    return static_cast<decimal32>(lhs) <= rhs;
}

template <typename T, std::enable_if_t<detail::is_integral<T>::value, bool> = true>
constexpr bool operator>(decimal32 lhs, T rhs) noexcept
{
    return lhs > static_cast<decimal32>(rhs);
}

template <typename T, std::enable_if_t<detail::is_integral<T>::value, bool> = true>
constexpr bool operator>(T lhs, decimal32 rhs) noexcept
{
    return static_cast<decimal32>(lhs) > rhs;
}

template <typename T, std::enable_if_t<detail::is_integral<T>::value, bool> = true>
constexpr bool operator>=(decimal32 lhs, T rhs) noexcept
{
    return lhs >= static_cast<decimal32>(rhs);
}

template <typename T, std::enable_if_t<detail::is_integral<T>::value, bool> = true>
constexpr bool operator>=(T lhs, decimal32 rhs) noexcept
{
    return static_cast<decimal32>(lhs) >= rhs;
}

#ifdef BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR

template <typename T, std::enable_if_t<detail::is_integral<T>::value, bool> = true>
constexpr std::strong_ordering operator<=>(decimal32 lhs, T rhs) noexcept
{
    return lhs <=> static_cast<decimal32>(rhs);
}

template <typename T, std::enable_if_t<detail::is_integral<T>::value, bool> = true>
constexpr std::strong_ordering operator<=>(T lhs, decimal32 rhs) noexcept
{
    return static_cast<decimal32>(lhs) <=> rhs;
}

#endif

}} // Namespaces

#endif // BOOST_DECIMAL_COMMON_COMP_HPP
