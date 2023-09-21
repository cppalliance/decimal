// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_FPCLASSIFY_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_FPCLASSIFY_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <type_traits>
#include <cmath>

namespace boost { namespace decimal {

template <typename T>
constexpr auto fpclassify BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (T rhs) noexcept
    -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, int>
{
    if (isinf(rhs))
    {
        return FP_INFINITE;
    }
    else if (isnan(rhs))
    {
        return FP_NAN;
    }
    else if (abs(rhs) == 0)
    {
        return FP_ZERO;
    }
    else if (!isnormal(rhs))
    {
        return FP_SUBNORMAL;
    }
    else
    {
        return FP_NORMAL;
    }
}

} // namespace decimal
} // namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_FPCLASSIFY_HPP
