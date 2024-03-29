// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_HYPOT_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_HYPOT_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/cmath/abs.hpp>
#include <boost/decimal/detail/cmath/sqrt.hpp>
#include <boost/decimal/detail/cmath/fmax.hpp>
#include <boost/decimal/detail/utilities.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/promotion.hpp>
#include <type_traits>
#include <cmath>

namespace boost {
namespace decimal {

namespace detail {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto hypot_impl(T x, T y) noexcept
{
    constexpr T zero {0, 0};

    if (abs(x) == zero || isnan(y))
    {
        return y;
    }
    else if (abs(y) == zero || isnan(x))
    {
        return x;
    }
    else if (isinf(x) || isinf(y))
    {
        // Return +inf even if the other value is nan
        return std::numeric_limits<T>::infinity();
    }

    auto new_x {abs(x)};
    auto new_y {abs(y)};

    if (new_y > new_x)
    {
        detail::swap(new_x, new_y);
    }

    if (new_x * std::numeric_limits<T>::epsilon() >= new_y)
    {
        return new_x;
    }

    const auto rat {new_y / new_x};
    return new_x * sqrt(1 + rat * rat);
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto hypot_impl(T x, T y, T z) noexcept
{
    if (isinf(x) || isinf(y) || isinf(z))
    {
        return std::numeric_limits<T>::infinity();
    }
    else if (isnan(x))
    {
        return x;
    }
    else if (isnan(y))
    {
        return y;
    }
    else if (isnan(z))
    {
        return z;
    }

    const auto a {fmax(fmax(x, y), z)};
    const auto x_over_a {x / a};
    const auto y_over_a {y / a};
    const auto z_over_a {z / a};

    return a * sqrt((x_over_a * x_over_a) + (y_over_a * y_over_a) + (z_over_a * z_over_a));
}

} // namespace detail

template <typename T1, typename T2>
constexpr auto hypot(T1 x, T2 y) noexcept
    BOOST_DECIMAL_REQUIRES_TWO(detail::is_decimal_floating_point_v, T1, detail::is_decimal_floating_point_v, T2)
{
    #if BOOST_DECIMAL_DEC_EVAL_METHOD == 0

    using evaluation_type = detail::promote_args_t<T1, T2>;

    #elif BOOST_DECIMAL_DEC_EVAL_METHOD == 1

    using evaluation_type = detail::promote_args_t<T1, T2, decimal64>;

    #else // BOOST_DECIMAL_DEC_EVAL_METHOD == 2

    using evaluation_type = detail::promote_args_t<T1, T2, decimal128>;

    #endif

    using return_type = detail::promote_args_t<T1, T2>;

    return static_cast<return_type>(detail::hypot_impl(static_cast<evaluation_type>(x), static_cast<evaluation_type>(y)));
}

template <typename T1, typename T2, typename T3>
constexpr auto hypot(T1 x, T2 y, T3 z) noexcept
    BOOST_DECIMAL_REQUIRES_THREE(detail::is_decimal_floating_point_v, T1, detail::is_decimal_floating_point_v, T2, detail::is_decimal_floating_point_v, T3)
{
    #if BOOST_DECIMAL_DEC_EVAL_METHOD == 0

    using evaluation_type = detail::promote_args_t<T1, T2, T3>;

    #elif BOOST_DECIMAL_DEC_EVAL_METHOD == 1

    using evaluation_type = detail::promote_args_t<T1, T2, T3, decimal64>;

    #else // BOOST_DECIMAL_DEC_EVAL_METHOD == 2

    using evaluation_type = detail::promote_args_t<T1, T2, T3, decimal128>;

    #endif

    using return_type = detail::promote_args_t<T1, T2, T3>;

    return static_cast<return_type>(detail::hypot_impl(static_cast<evaluation_type>(x),
                                                       static_cast<evaluation_type>(y),
                                                       static_cast<evaluation_type>(z)));
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_HYPOT_HPP
