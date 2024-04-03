// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_LDEXP_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_LDEXP_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/cmath/impl/pow_impl.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/config.hpp>
#include <type_traits>
#include <cmath>

namespace boost {
namespace decimal {

namespace detail {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto ldexp_impl(T v, int e2) noexcept
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
            result *= detail::pow_2_impl<T>(e2);
        }
    }

    return result;
}

} // namespace detail

BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto ldexp(T v, int e2) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    #if BOOST_DECIMAL_DEC_EVAL_METHOD == 0

    using evaluation_type = T;

    #elif BOOST_DECIMAL_DEC_EVAL_METHOD == 1

    using evaluation_type = detail::promote_args_t<T, decimal64>;

    #else // BOOST_DECIMAL_DEC_EVAL_METHOD == 2

    using evaluation_type = detail::promote_args_t<T, decimal128>;

    #endif

    return static_cast<T>(detail::ldexp_impl(static_cast<evaluation_type>(v), e2));
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_LDEXP_HPP
