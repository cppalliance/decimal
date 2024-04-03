// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_SQRT_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_SQRT_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/cmath/abs.hpp>
#include <type_traits>
#include <cstdint>
#include <cmath>

namespace boost {
namespace decimal {

namespace detail {

template <typename T>
constexpr auto sqrt_impl(T val) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    constexpr T zero {0, 0};

    T result { };

    if (isnan(val) || abs(val) == zero)
    {
        result = val;
    }
    else if (isinf(val))
    {
        if (signbit(val))
        {
            result = std::numeric_limits<T>::quiet_NaN();
        }
        else
        {
            result = val;
        }
    }
    else if (val < zero)
    {
        result = std::numeric_limits<T>::quiet_NaN();
    }
    else
    {
        // Loosens tolerance with the increase in digits so it's not excessively expensive
        constexpr T epsilon = std::numeric_limits<T>::epsilon() * std::numeric_limits<T>::digits10;
        constexpr T one { 1, 0 };
        constexpr T half {5, -1};
        T error = one / epsilon;

        T x {};
        if (val > one)
        {
            // Scale down if val is large by dividing the exp by 2
            int exp {};
            auto sig = frexp10(val, &exp);
            x = T{sig, exp / 2};
        }
        else
        {
            // Trivial heuristic
            x = val + half;
        }

        while (error > epsilon)
        {
            const T new_x = (x + val / x) / 2;

            error = fabs(new_x - x);
            x = new_x;
        }

        result = x;
    }

    return result;
}

} //namespace detail

template <typename T>
constexpr auto sqrt(T val) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    #if BOOST_DECIMAL_DEC_EVAL_METHOD == 0

    using evaluation_type = T;

    #elif BOOST_DECIMAL_DEC_EVAL_METHOD == 1

    using evaluation_type = detail::promote_args_t<T, decimal64>;

    #else // BOOST_DECIMAL_DEC_EVAL_METHOD == 2

    using evaluation_type = detail::promote_args_t<T, decimal128>;

    #endif

    return static_cast<T>(detail::sqrt_impl(static_cast<evaluation_type>(val)));
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_SQRT_HPP

