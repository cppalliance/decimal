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

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T1, BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T2>
constexpr auto hypot(T1 x, T2 y) noexcept
{
    using promoted_type = detail::promote_args_t<T1, T2>;

    constexpr promoted_type zero {0, 0};

    if (abs(x) == zero || isnan(y))
    {
        return static_cast<promoted_type>(y);
    }
    else if (abs(y) == zero || isnan(x))
    {
        return static_cast<promoted_type>(x);
    }
    else if (isinf(x) || isinf(y))
    {
        // Return +inf even if the other value is nan
        return std::numeric_limits<promoted_type>::infinity();
    }

    auto new_x {static_cast<promoted_type>(abs(x))};
    auto new_y {static_cast<promoted_type>(abs(y))};

    if (new_y > new_x)
    {
        detail::swap(new_x, new_y);
    }

    if (new_x * std::numeric_limits<promoted_type>::epsilon() >= new_y)
    {
        return new_x;
    }

    const auto rat {new_y / new_x};
    return new_x * sqrt(1 + rat * rat);
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T1,
          BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T2,
          BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T3>
constexpr auto hypot(T1 x, T2 y, T3 z) noexcept
{
    using promoted_type = detail::promote_args_t<T1, T2, T3>;

    if (isinf(x) || isinf(y) || isinf(z))
    {
        return std::numeric_limits<promoted_type>::infinity();
    }
    else if (isnan(x))
    {
        return static_cast<promoted_type>(x);
    }
    else if (isnan(y))
    {
        return static_cast<promoted_type>(y);
    }
    else if (isnan(z))
    {
        return static_cast<promoted_type>(z);
    }

    const promoted_type a {fmax(fmax(x, y), z)};
    const promoted_type x_over_a {x / a};
    const promoted_type y_over_a {y / a};
    const promoted_type z_over_a {z / a};

    return a * sqrt((x_over_a * x_over_a) + (y_over_a * y_over_a) + (z_over_a * z_over_a));
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_HYPOT_HPP
