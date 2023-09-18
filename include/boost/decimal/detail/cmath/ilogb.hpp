// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_ILOGB_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_ILOGB_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <type_traits>
#include <climits>
#include <cmath>

namespace boost { namespace decimal {

// TODO(mborland): Allow conversion between decimal types via a promotion system

template <typename T>
constexpr auto ilogb(T d) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, int>
{
    if (d == 0)
    {
        return FP_ILOGB0;
    }
    else if (isinf(d))
    {
        return INT_MAX;
    }
    else if (isnan(d))
    {
        return FP_ILOGBNAN;
    }

    const auto offset =
        static_cast<int>
        (
            detail::num_digits(d.full_significand()) - static_cast<int>(INT8_C(1))
        );

    const auto expval = static_cast<int>(static_cast<int>(d.unbiased_exponent()) + offset);

    return expval;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_ILOGB_HPP
