// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_LDEXP_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_LDEXP_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/cmath/impl/pow_impl.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <type_traits>
#include <cmath>

namespace boost { namespace decimal {

template<typename T, std::enable_if_t<detail::is_decimal_floating_point_v<T>, bool> = true>
constexpr auto ldexp(T v, int e2) noexcept -> T
{
    T result { };

    const auto v_fp {fpclassify(v)};

    if (v_fp != FP_NORMAL)
    {
        if (v_fp == FP_NAN)
        {
            result = std::numeric_limits<T>::quiet_NaN();
        }
        else if (v_fp == FP_INFINITE)
        {
            result = std::numeric_limits<T>::infinity();
        }
        else
        {
            result = T { 0, 0 };
        }
    }
    else
    {
        result = v;

        if(e2 != 0)
        {
            result *= detail::pow2<T>(e2);
        }
    }

    return result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_LDEXP_HPP
