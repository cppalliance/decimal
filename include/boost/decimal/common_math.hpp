// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_COMMON_MATH_HPP
#define BOOST_DECIMAL_COMMON_MATH_HPP

#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/decimal32.hpp>
#include <type_traits>

namespace boost { namespace decimal {

// Overloads may be any integral type or,
// TODO(mborland): any decimal type returning the promoted decimal type (e.g. dec32 + dec64 yields dec64)

// Decimal32 overloads
template <typename T, std::enable_if_t<detail::is_integral<T>::value, bool> = true>
constexpr decimal32 operator+(decimal32 lhs, T rhs) noexcept
{
    return lhs + static_cast<decimal32>(rhs);
}

template <typename T, std::enable_if_t<detail::is_integral<T>::value, bool> = true>
constexpr decimal32 operator+(T lhs, decimal32 rhs) noexcept
{
    return static_cast<decimal32>(lhs) + rhs;
}

template <typename T, std::enable_if_t<detail::is_integral<T>::value, bool> = true>
constexpr decimal32 operator-(decimal32 lhs, T rhs) noexcept
{
    return lhs - static_cast<decimal32>(rhs);
}

template <typename T, std::enable_if_t<detail::is_integral<T>::value, bool> = true>
constexpr decimal32 operator-(T lhs, decimal32 rhs) noexcept
{
    return static_cast<decimal32>(lhs) - rhs;
}

template <typename T, std::enable_if_t<detail::is_integral<T>::value, bool> = true>
constexpr decimal32 operator*(decimal32 lhs, T rhs) noexcept
{
    return lhs * static_cast<decimal32>(rhs);
}

template <typename T, std::enable_if_t<detail::is_integral<T>::value, bool> = true>
constexpr decimal32 operator*(T lhs, decimal32 rhs) noexcept
{
    return static_cast<decimal32>(lhs) * rhs;
}

template <typename T, std::enable_if_t<detail::is_integral<T>::value, bool> = true>
constexpr decimal32 operator/(decimal32 lhs, T rhs) noexcept
{
    return lhs / static_cast<decimal32>(rhs);
}

template <typename T, std::enable_if_t<detail::is_integral<T>::value, bool> = true>
constexpr decimal32 operator/(T lhs, decimal32 rhs) noexcept
{
    return static_cast<decimal32>(lhs) / rhs;
}

constexpr bool samequantum(decimal32 lhs, decimal32 rhs) noexcept
{
    return samequantumd32(lhs, rhs);
}

constexpr int quantexp(decimal32 x) noexcept
{
    return quantexpd32(x);
}

}}

#endif // BOOST_DECIMAL_COMMON_MATH_HPP
