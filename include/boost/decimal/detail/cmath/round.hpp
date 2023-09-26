// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_ROUND_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_ROUND_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/cmath/modf.hpp>
#include <boost/decimal/detail/cmath/abs.hpp>
#include <type_traits>
#include <limits>
#include <cstdint>

namespace boost {
namespace decimal {

template <typename T>
constexpr auto round(T num) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>
{
    constexpr T zero {0, 0};
    constexpr T half {5, -1};

    if (isnan(num) || isinf(num) || abs(num) == zero)
    {
        return num;
    }

    T iptr {};
    const auto x {modf(num, &iptr)};

    if (x >= half && iptr > 0)
    {
        ++iptr;
    }
    else if (abs(x) >= half && iptr < 0)
    {
        --iptr;
    }

    return iptr;
}

namespace detail {

template <typename T, typename Int>
constexpr auto int_round_impl(T num) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, Int>
{
    constexpr T zero {0, 0};
    constexpr T lmax {(std::numeric_limits<Int>::max)()};
    constexpr T lmin {(std::numeric_limits<Int>::min)()};

    const auto rounded_val {round(num)};

    if (isinf(num) || isnan(num))
    {
        return std::numeric_limits<Int>::min();
    }
    else if (abs(num) == zero)
    {
        return 0;
    }

    if (rounded_val > lmax)
    {
        return (std::numeric_limits<Int>::max)();
    }
    else if (rounded_val < lmin)
    {
        return (std::numeric_limits<Int>::min)();
    }

    return static_cast<Int>(rounded_val);
}

} //namespace detail

template <typename T>
constexpr auto lround(T num) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, long>
{
    return detail::int_round_impl<T, long>(num);
}

template <typename T>
constexpr auto llround(T num) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, long long>
{
    return detail::int_round_impl<T, long long>(num);
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_ROUND_HPP
