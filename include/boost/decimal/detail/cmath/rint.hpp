// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_RINT_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_RINT_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/fenv_rounding.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <boost/decimal/detail/cmath/floor.hpp>
#include <boost/decimal/detail/cmath/ceil.hpp>
#include <boost/decimal/detail/cmath/trunc.hpp>
#include <boost/decimal/detail/cmath/frexp10.hpp>
#include <type_traits>
#include <limits>
#include <cmath>
#include <climits>

namespace boost {
namespace decimal {

namespace detail {

template <typename T1, typename T2>
constexpr auto rint_impl(T1& sig, T2 exp, bool sign)
{
    sig /= detail::pow10<std::uint32_t>(std::abs(exp) - 1);
    detail::fenv_round(sig, sign);
}

}

// Rounds the number using the default rounding mode
template <typename T>
constexpr auto rint(T num) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>
{
    constexpr T zero {0, 0};

    if (isinf(num) || isnan(num) || abs(num) == zero)
    {
        return num;
    }

    int expptr {};
    auto sig {frexp10(num, &expptr)}; // Always returns detail::precision digits
    const bool is_neg {num < 0};

    if (expptr > detail::precision)
    {
        return num;
    }
    else if (expptr < -detail::precision)
    {
        return is_neg ? -zero : zero;
    }

    detail::rint_impl(sig, expptr, is_neg);

    return {sig, 0, is_neg};
}

namespace detail {

template<typename T, typename Int>
constexpr auto lrint_impl(T num) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, Int>
{
    constexpr T zero {0, 0};
    constexpr T lmax {std::numeric_limits<Int>::max()};
    constexpr T lmin {std::numeric_limits<Int>::min()};

    if (isinf(num) || isnan(num))
    {
        // Implementation defined what to return here
        return std::numeric_limits<Int>::min();
    }
    else if (abs(num) == zero)
    {
        return 0;
    }

    int expptr {};
    auto sig {frexp10(num, &expptr)}; // Always returns detail::precision digits
    const bool is_neg {num < 0};

    if (num > lmax)
    {
        return std::numeric_limits<Int>::max();
    }
    else if (num < lmin)
    {
        return std::numeric_limits<Int>::min();
    }

    if (expptr > detail::precision)
    {
        return static_cast<long>(num);
    }
    else if (expptr < -detail::precision)
    {
        return 0;
    }

    detail::rint_impl(sig, expptr, is_neg);

    auto res {static_cast<long>(sig)};
    if (is_neg)
    {
        res = -res;
    }

    return res;
}

}

template <typename T>
constexpr auto lrint(T num) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, long>
{
    return detail::lrint_impl<T, long>(num);
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_RINT_HPP
