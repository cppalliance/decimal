// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_MIXED_DECIMAL_ARITHMETIC_HPP
#define BOOST_DECIMAL_DETAIL_MIXED_DECIMAL_ARITHMETIC_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/promotion.hpp>
#include <type_traits>

namespace boost {
namespace decimal {

template <typename Decimal1, typename Decimal2>
constexpr auto operator+(Decimal1 lhs, Decimal2 rhs) noexcept
    -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                         detail::is_decimal_floating_point_v<Decimal2>),
                         detail::promote_args_t<Decimal1, Decimal2>>
{
    using Promoted_Type = detail::promote_args_t<Decimal1, Decimal2>;
    return static_cast<Promoted_Type>(lhs) + static_cast<Promoted_Type>(rhs);
}

template <typename Decimal1, typename Decimal2>
constexpr auto operator-(Decimal1 lhs, Decimal2 rhs) noexcept
    -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                         detail::is_decimal_floating_point_v<Decimal2>),
                         detail::promote_args_t<Decimal1, Decimal2>>
{
    using Promoted_Type = detail::promote_args_t<Decimal1, Decimal2>;
    return static_cast<Promoted_Type>(lhs) - static_cast<Promoted_Type>(rhs);
}

template <typename Decimal1, typename Decimal2>
constexpr auto operator*(Decimal1 lhs, Decimal2 rhs) noexcept
    -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                         detail::is_decimal_floating_point_v<Decimal2>),
                         detail::promote_args_t<Decimal1, Decimal2>>
{
    using Promoted_Type = detail::promote_args_t<Decimal1, Decimal2>;
    return static_cast<Promoted_Type>(lhs) * static_cast<Promoted_Type>(rhs);
}

template <typename Decimal1, typename Decimal2>
constexpr auto operator/(Decimal1 lhs, Decimal2 rhs) noexcept
    -> std::enable_if_t<(detail::is_decimal_floating_point_v<Decimal1> &&
                         detail::is_decimal_floating_point_v<Decimal2>),
                         detail::promote_args_t<Decimal1, Decimal2>>
{
    using Promoted_Type = detail::promote_args_t<Decimal1, Decimal2>;
    return static_cast<Promoted_Type>(lhs) / static_cast<Promoted_Type>(rhs);
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_MIXED_DECIMAL_ARITHMETIC_HPP
