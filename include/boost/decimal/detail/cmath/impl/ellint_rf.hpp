//  Copyright (c) 2006 Xiaogang Zhang, 2015 John Maddock
//  Copyright 2024 Matt Borland
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_ELLINT_RF_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_ELLINT_RF_HPP

#include <boost/decimal/numbers.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/utilities.hpp>
#include <boost/decimal/detail/cmath/log1p.hpp>
#include <boost/decimal/detail/cmath/sqrt.hpp>
#include <boost/decimal/detail/cmath/atan.hpp>
#include <boost/decimal/detail/cmath/impl/ellint_rc.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <limits>
#endif

namespace boost {
namespace decimal {
namespace detail {
namespace ellint_impl {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto ellint_rf_imp(T x, T y, T z) noexcept
{
    constexpr T zero {0, 0};

    if (x < zero || y < zero || z < zero)
    {
        return std::numeric_limits<T>::signaling_NaN();
    }
    if (x + y == zero || y + z == zero || z + x == zero)
    {
        return std::numeric_limits<T>::signaling_NaN();
    }

    //
    // Special cases from http://dlmf.nist.gov/19.20#i
    //
    if (x == y)
    {
        if (x == z)
        {
            // x, y, z equal:
            return 1 / sqrt(x);
        }
        else
        {
            // 2 equal, x and y:
            if(z == zero)
            {
                return numbers::pi_v<T> / (2 * sqrt(x));
            }
            else
            {
                return ellint_rc_imp(z, x);
            }
        }
    }
    if (x == z)
    {
        if (y == 0)
        {
            return numbers::pi_v<T> / (2 * sqrt(x));
        }
        else
        {
            return ellint_rc_imp(y, x);
        }
    }
    if (y == z)
    {
        if (x == 0)
        {
            return numbers::pi_v<T> / (2 * sqrt(y));
        }
        else
        {
            return ellint_rc_imp(x, y);
        }
    }

    if (x == 0)
    {
        swap(x, z);
    }
    else if (y == 0)
    {
        swap(y, z);
    }
    if (z == 0)
    {
        //
        // Special case for one value zero:
        //
        T xn {sqrt(x)};
        T yn {sqrt(y)};

        unsigned k {1U};
        while(fabs(xn - yn) >= T{27, -1} * std::numeric_limits<T>::epsilon() * fabs(xn) && k < 20)
        {
            T t {sqrt(xn * yn)};
            xn = (xn + yn) / 2;
            yn = t;
            ++k;
        }

        return numbers::pi_v<T> / (xn + yn);
    }

    // TODO(mborland): Unblock once ellint_1 is in
    // LCOV_EXCL_START
    T xn {x};
    T yn {y};
    T zn {z};
    T An {(x + y + z) / 3};
    T A0 {An};
    T Q {pow(3 * std::numeric_limits<T>::epsilon(), T{-1} / 8) * (std::max)((std::max)(fabs(An - xn), fabs(An - yn)), fabs(An - zn))};
    T fn {1};


    // duplication
    unsigned k {1U};
    for(; k < 1000U; ++k)
    {
        T root_x {sqrt(xn)};
        T root_y {sqrt(yn)};
        T root_z {sqrt(zn)};
        T lambda {root_x * root_y + root_x * root_z + root_y * root_z};
        An = (An + lambda) / 4;
        xn = (xn + lambda) / 4;
        yn = (yn + lambda) / 4;
        zn = (zn + lambda) / 4;
        Q /= 4;
        fn *= 4;
        if(Q < fabs(An))
        {
            break;
        }
    }

    T X {(A0 - x) / (An * fn)};
    T Y {(A0 - y) / (An * fn)};
    T Z {-X - Y};

    // Taylor series expansion to the 7th order
    T E2 {X * Y - Z * Z};
    T E3 {X * Y * Z};
    return (1 + E3 * (T(1) / 14 + 3 * E3 / 104) + E2 * (T(-1) / 10 + E2 / 24 - (3 * E3) / 44 - 5 * E2 * E2 / 208 + E2 * E3 / 16)) / sqrt(An);
    // LCOV_EXCL_STOP
}

} //namespace ellint_impl

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T1,
          BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T2,
          BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T3>
constexpr auto ellint_rf(T1 x, T2 y, T3 z) noexcept
{
    using promoted_type = promote_args_t<T1, T2, T3>;
    return ellint_impl::ellint_rf_imp(static_cast<promoted_type>(x),
                                      static_cast<promoted_type>(y),
                                      static_cast<promoted_type>(z));
}

} //namespace detail
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_IMPL_ELLINT_RF_HPP
