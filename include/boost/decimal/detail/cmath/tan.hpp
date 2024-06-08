// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_TAN_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_TAN_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/numbers.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/cmath/cos.hpp>
#include <boost/decimal/detail/cmath/remquo.hpp>
#include <boost/decimal/detail/cmath/impl/sin_impl.hpp>
#include <boost/decimal/detail/cmath/impl/cos_impl.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <type_traits>
#include <cstdint>
#endif

namespace boost {
namespace decimal {

BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto tan(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    T result { };

    const auto fpc = fpclassify(x);

    // First check non-finite values and small angles.
    if (fabs(x) < std::numeric_limits<T>::epsilon() || (fpc == FP_NAN))
    {
        result = x;
    }
    else if (fpc == FP_INFINITE)
    {
        result = std::numeric_limits<T>::quiet_NaN();
    }
    else if (signbit(x))
    {
        result = -tan(-x);
    }
    else
    {
        result = sin(x) / cos(x);
    }

    return result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_TAN_HPP
