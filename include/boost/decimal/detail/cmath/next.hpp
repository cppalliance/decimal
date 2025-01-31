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
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/cmath/modf.hpp>
#include <boost/decimal/detail/cmath/abs.hpp>
#include <boost/decimal/detail/cmath/round.hpp>
#include <boost/decimal/detail/cmath/ilogb.hpp>
#include <boost/decimal/detail/cmath/fpclassify.hpp>
#include <boost/decimal/detail/cmath/frexp10.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <limits>
#endif

namespace boost {
namespace decimal {

namespace detail {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T1>
constexpr auto nextafter_impl(T1 val, bool direction) noexcept -> T1
{
    constexpr T1 zero {0};

    // Val < direction = +
    // Val > direction = -
    const auto abs_val {abs(val)};

    if (val == zero)
    {
        const auto min_val {direction ? std::numeric_limits<T1>::denorm_min() :
                                        -std::numeric_limits<T1>::denorm_min()};
        return min_val;
    }
    if (abs_val > zero && abs_val < std::numeric_limits<T1>::epsilon())
    {
        const auto min_val {direction ? val + std::numeric_limits<T1>::min() :
                                        val - std::numeric_limits<T1>::min()};
        return min_val;
    }

    const auto val_eps {direction ? val + std::numeric_limits<T1>::epsilon() :
                                    val - std::numeric_limits<T1>::epsilon()};

    // If adding epsilon does nothing then we need to manipulate the representation
    if (val == val_eps)
    {
        int exp {} ;
        auto significand {frexp10(val, &exp)};

        direction ? significand++ : significand--;

        return T1{significand, exp};
    }
    else
    {
        return val_eps;
    }
}

} // namespace detail

BOOST_DECIMAL_EXPORT
template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T1,
          BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T2>
constexpr auto nextafter(T1 val, T2 direction) noexcept
    BOOST_DECIMAL_REQUIRES_TWO(detail::is_decimal_floating_point_v, T1, detail::is_decimal_floating_point_v, T2)
{
    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isnan(val) || isinf(val))
    {
        return val;
    }
    else if (isnan(direction) || val == direction)
    {
        return direction;
    }
    #else
    if (val == direction)
    {
        return direction;
    }
    #endif
    else
    {
        return detail::nextafter_impl(val, val < direction);
    }
}

BOOST_DECIMAL_EXPORT template <typename T>
BOOST_DECIMAL_CXX20_CONSTEXPR auto nexttoward(T val, long double direction) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    const auto dec_direction {static_cast<T>(direction)};
    return nextafter(val, dec_direction);
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_NEXT_HPP
