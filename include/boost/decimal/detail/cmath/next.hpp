//  (C) Copyright John Maddock 2008 - 2023.
//  (C) Copyright Matt Borland 2023.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_DECIMAL_DETAIL_CMATH_NEXT_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_NEXT_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/promotion.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/cmath/modf.hpp>
#include <boost/decimal/detail/cmath/abs.hpp>
#include <boost/decimal/detail/cmath/round.hpp>
#include <boost/decimal/detail/cmath/ilogb.hpp>
#include <boost/decimal/detail/cmath/fpclassify.hpp>
#include <type_traits>
#include <limits>
#include <cstdint>

namespace boost {
namespace decimal {

namespace detail {

template <typename T>
constexpr auto float_next_imp(T val) noexcept -> T
{
    const auto fpclass {fpclassify(val)};

    if (fpclass == FP_NAN || fpclass == FP_INFINITE)
    {
        return val;
    }
    else if (fpclass == FP_ZERO)
    {
        return std::numeric_limits<T>::epsilon();
    }

    return val + std::numeric_limits<T>::epsilon();
}

template <typename T>
constexpr auto float_prior_imp(T val) noexcept -> T
{
    const auto fpclass {fpclassify(val)};

    if (fpclass == FP_NAN || fpclass == FP_INFINITE)
    {
        return val;
    }
    else if (fpclass == FP_ZERO)
    {
        return -std::numeric_limits<T>::epsilon();
    }

    return val - std::numeric_limits<T>::epsilon();
}

} //namespace detail

template <typename T1, typename T2>
constexpr auto nextafter(T1 val, T2 direction) noexcept
    -> std::enable_if_t<(detail::is_decimal_floating_point_v<T1> || detail::is_decimal_floating_point_v<T2>),
                         detail::promote_args_t<T1, T2>>
{
    if (isnan(direction) || val == direction)
    {
        return direction;
    }
    else if (val < direction)
    {
        return detail::float_next_imp(val);
    }

    return detail::float_prior_imp(val);
}

template <typename T>
BOOST_DECIMAL_CXX20_CONSTEXPR auto nexttoward(T val, long double direction) noexcept
    -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>
{
    const auto dec_direction {static_cast<T>(direction)};
    return nextafter(val, dec_direction);
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_NEXT_HPP
