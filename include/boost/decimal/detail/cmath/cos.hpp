// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_COS_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_COS_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/numbers.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/cmath/cos.hpp>
#include <boost/decimal/detail/cmath/remquo.hpp>
#include <boost/decimal/detail/cmath/impl/sin_impl.hpp>
#include <boost/decimal/detail/cmath/impl/cos_impl.hpp>
#include <type_traits>
#include <cstdint>

namespace boost { namespace decimal {

template<typename T, std::enable_if_t<detail::is_decimal_floating_point_v<T>, bool> = true>
constexpr auto cos(T x) noexcept -> T
{
    constexpr T zero {0, 0};

    // First check non-finite values and small angles
    if (abs(x) < std::numeric_limits<T>::epsilon() || isinf(x) || isnan(x))
    {
        return x;
    }

    if (x < zero)
    {
        return cos(-x);
    }

    int quo {};
    auto x90 {remquo(x, numbers::pi_v<T>/T(2), &quo)};
    switch (quo)
    {
        case 0:
            return detail::cos_impl(x90);
        case 1:
            return -detail::sin_impl(x90);
        case 3:
            return detail::cos_impl(-x90);
        case 4:
            return detail::sin_impl(x90);
        default:
            assert(quo < 4);
            return static_cast<T>(quo);
    }
}

}} // Namespaces

#endif // BOOST_DECIMAL_DETAIL_CMATH_COS_HPP
