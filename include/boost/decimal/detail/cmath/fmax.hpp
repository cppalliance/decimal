// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_FMAX_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_FMAX_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/promotion.hpp>
#include <type_traits>
#include <cmath>

namespace boost {
namespace decimal {

template <typename T1, typename T2>
constexpr auto fmax(T1 lhs, T2 rhs) noexcept
    BOOST_DECIMAL_REQUIRES_TWO(detail::is_decimal_floating_point_v, T1, detail::is_decimal_floating_point_v, T2)
{
    using promoted_type = detail::promote_args_t<T1, T2>;

    if (isnan(lhs) && !isnan(rhs))
    {
        return static_cast<promoted_type>(rhs);
    }
    else if ((!isnan(lhs) && isnan(rhs)) ||
             (isnan(lhs) && isnan(rhs)))
    {
        return static_cast<promoted_type>(lhs);
    }

    return lhs > rhs ? static_cast<promoted_type>(lhs) : static_cast<promoted_type>(rhs);
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_FMAX_HPP
