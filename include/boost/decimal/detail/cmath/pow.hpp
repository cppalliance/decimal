// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_POW_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_POW_HPP

#include <algorithm>
#include <cmath>
#include <type_traits>

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/cmath/impl/pow_impl.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>

namespace boost {
namespace decimal {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T, BOOST_DECIMAL_INTEGRAL IntegralType>
constexpr auto pow(T b, IntegralType p) noexcept
    -> std::enable_if_t<(detail::is_decimal_floating_point_v<T> && std::is_integral<IntegralType>::value), T> // NOLINT(misc-no-recursion)
{
    using local_integral_type = IntegralType;

    constexpr T zero { 0, 0 };
    constexpr T one  { 1, 0 };

    T result { };

    const auto fpc_x = fpclassify(b);

    const auto p_is_integer_odd = (static_cast<local_integral_type>(p & 1) != static_cast<local_integral_type>(0));

    if (fpc_x == FP_ZERO)
    {
        if(p < static_cast<local_integral_type>(0))
        {
            // pow(  +0, exp), where exp is a negative odd integer, returns +infinity.
            // pow(  -0, exp), where exp is a negative odd integer, returns +infinity.
            // pow(+/-0, exp), where exp is a negative even integer, returns +infinity.

            result = std::numeric_limits<T>::infinity();
        }
        else if (p > static_cast<local_integral_type>(0))
        {
            // pow(  +0, exp), where exp is a positive odd integer, returns +0.
            // pow(  -0, exp), where exp is a positive odd integer, returns -0.
            // pow(+/-0, exp), where exp is a positive even integer, returns +0.

            if (p_is_integer_odd)
            {
                result = (signbit(b) ? -zero : zero);
            }
            else
            {
                result = zero;
            }
        }
        else
        {
            // pow(base, +/-0) returns 1 for any base, even when base is NaN.

            result = one;
        }
    }
    else if (fpc_x == FP_INFINITE)
    {
        if (signbit(b))
        {
            if (p < static_cast<local_integral_type>(0))
            {
                // pow(-infinity, exp) returns -0 if exp is a negative odd integer.
                // pow(-infinity, exp) returns +0 if exp is a negative even integer.

                result = (p_is_integer_odd ? -zero : zero);
            }
            else if (p > static_cast<local_integral_type>(0))
            {
                // pow(-infinity, exp) returns -infinity if exp is a positive odd integer.
                // pow(-infinity, exp) returns +infinity if exp is a positive even integer.

                result = (p_is_integer_odd ? -std::numeric_limits<T>::infinity() : std::numeric_limits<T>::infinity());
            }
            else
            {
                result = one;
            }
        }
        else
        {
            if (p < static_cast<local_integral_type>(0))
            {
                // pow(+infinity, exp) returns +0 for any negative exp.

                result = zero;
            }
            else if (p > static_cast<local_integral_type>(0))
            {
                // pow(+infinity, exp) returns +infinity for any positive exp.

                result = std::numeric_limits<T>::infinity();
            }
            else
            {
                result = one;
            }
        }
    }
    else if (fpc_x != FP_NORMAL)
    {
        result = ((p == static_cast<local_integral_type>(UINT8_C(0))) ? one : std::numeric_limits<T>::quiet_NaN());
    }
    else
    {
        using local_unsigned_integral_type = std::make_unsigned_t<IntegralType>;

        if (p == static_cast<local_integral_type>(UINT8_C(0)))
        {
            result = one;
        }
        else BOOST_DECIMAL_IF_CONSTEXPR (std::is_signed<local_integral_type>::value)
        {
            if(p < static_cast<local_integral_type>(UINT8_C(0)))
            {
                const auto up =
                    static_cast<local_unsigned_integral_type>
                    (
                          static_cast<local_unsigned_integral_type>(~p)
                        + static_cast<local_unsigned_integral_type>(UINT8_C(1))
                    );

                result = one / detail::pow_n_impl(b, up);
            }
            else
            {
                result = detail::pow_n_impl(b, static_cast<local_unsigned_integral_type>(p));
            }
        }
        else
        {
            result = detail::pow_n_impl(b, static_cast<local_unsigned_integral_type>(p));
        }
    }

    return result;
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto pow(T x, T a) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>
{
    constexpr T zero { 0, 0 };

    auto result = zero;

    const auto na = static_cast<int>(a);

    if ((na == a) || ((na == 0) && (na == abs(a))))
    {
        result = pow(x, na);
    }
    else
    {
        constexpr T one  { 1, 0 };

        const auto fpc_x = fpclassify(x);
        const auto fpc_a = fpclassify(a);

        if (fpc_a == FP_ZERO)
        {
            // pow(base, +/-0) returns 1 for any base, even when base is NaN.

            result = one;
        }
        else if (fpc_x == FP_ZERO)
        {
            if ((fpc_a == FP_NORMAL) || (fpc_a == FP_INFINITE))
            {
                // pow(+/-0, exp), where exp is negative and finite, returns +infinity.
                // pow(+/-0, exp), where exp is positive non-integer, returns +0.

                // pow(+/-0, -infinity) returns +infinity.
                // pow(+/-0, +infinity) returns +0.

                result = (signbit(a) ? std::numeric_limits<T>::infinity() : zero);
            }
            else if (fpc_a == FP_NAN)
            {
                result = std::numeric_limits<T>::quiet_NaN();
            }
        }
        else if (fpc_x == FP_INFINITE)
        {
            if ((fpc_a == FP_NORMAL) || (fpc_a == FP_INFINITE))
            {
                // pow(+infinity, exp) returns +0 for any negative exp.
                // pow(-infinity, exp) returns +infinity for any positive exp.

                result = (signbit(a) ? zero : std::numeric_limits<T>::infinity());
            }
            else if (fpc_a == FP_NAN)
            {
                result = std::numeric_limits<T>::quiet_NaN();
            }
        }
        else if (fpc_x != FP_NORMAL)
        {
            result = x;
        }
        else
        {
            if (fpc_a == FP_ZERO)
            {
                result = one;
            }
            else if (fpc_a == FP_INFINITE)
            {
                using std::abs;

                result =
                    (
                          (abs(x) < one) ? (signbit(a) ? std::numeric_limits<T>::infinity() : zero)
                        : (abs(x) > one) ? (signbit(a) ? zero : std::numeric_limits<T>::infinity())
                        : one
                    );
            }
            else
            {
                const auto a_log_x = a * log(x);

                result = exp(a_log_x);
            }
        }
    }

    return result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_POW_HPP
