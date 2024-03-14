//  Copyright (c) 2006 Xiaogang Zhang, 2015 John Maddock
//  Copyright 2024 Matt Borland
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_ELLINT_RC_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_ELLINT_RC_HPP

#include <boost/decimal/numbers.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/cmath/log1p.hpp>
#include <boost/decimal/detail/cmath/sqrt.hpp>
#include <boost/decimal/detail/cmath/atan.hpp>
#include <limits>

namespace boost {
namespace decimal {
namespace detail {
namespace ellint_impl {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto ellint_rc_imp(T x, T y) noexcept
{
    constexpr T zero {0, 0};
    constexpr T one {1, 0};

    if (x < zero || y == zero)
    {
        return std::numeric_limits<T>::signaling_NaN();
    }

    // For y < 0, the integral is singular, return Cauchy principal value
    T prefix {};
    T result {};
    if (y < 0)
    {
        prefix = sqrt(x / (x - y));
    }
    else
    {
        prefix = one;
    }

    if (x == zero)
    {
        result = numbers::pi_v<T> / (2 * sqrt(y));
    }
    else if (x == y)
    {
        result = 1 / sqrt(x);
    }
    else if (y > x)
    {
        result = atan(sqrt((y - x) / x)) / sqrt(y - x);
    }
    else
    {
        if (y / x > T{5, -1})
        {
            T arg = sqrt((x - y) / x);
            result = (log1p(arg) - log1p(-arg)) / (2 * sqrt(x - y));
        }
        else
        {
            result = log((sqrt(x) + sqrt(x - y)) / sqrt(y)) / sqrt(x - y);
        }
    }

    return prefix * result;
}

} //namespace ellint_impl

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T2>
constexpr auto ellint_rc(T1 x, T2 y) noexcept
{
    using promoted_type = promote_args_t<T1, T2>;
    return ellint_impl::ellint_rc_imp(static_cast<promoted_type>(x), static_cast<promoted_type>(y));
}

} //namespace detail
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_IMPL_ELLINT_RC_HPP
