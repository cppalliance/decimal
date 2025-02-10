// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_RINT_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_RINT_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/fenv_rounding.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/cmath/floor.hpp>
#include <boost/decimal/detail/cmath/ceil.hpp>
#include <boost/decimal/detail/cmath/trunc.hpp>
#include <boost/decimal/detail/cmath/frexp10.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <type_traits>
#include <limits>
#include <cmath>
#include <climits>
#endif

namespace boost {
namespace decimal {

namespace detail {

template <BOOST_DECIMAL_INTEGRAL T1, BOOST_DECIMAL_INTEGRAL T2>
constexpr auto rint_impl(T1& sig, T2 exp, bool sign)
{
    using RoundType = std::conditional_t<std::is_same<T1, std::uint32_t>::value, decimal32,
                      std::conditional_t<std::is_same<T1, std::uint64_t>::value, decimal64, decimal128>>;

    const T2 abs_exp { (exp < T2(0)) ? -exp : exp };

    sig /= detail::pow10(static_cast<T1>(abs_exp - 1));

    detail::fenv_round<RoundType>(sig, sign);
}

// MSVC 14.1 warns of unary minus being applied to unsigned type from numeric_limits::min
// 14.2 and on get it right
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4146)
#endif

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T, BOOST_DECIMAL_INTEGRAL Int>
constexpr auto lrint_impl(T num) noexcept -> Int
{
    const T zero {0, 0};
    constexpr T lmax {(std::numeric_limits<Int>::max)()};
    constexpr T lmin {(std::numeric_limits<Int>::min)()};

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isinf(num) || isnan(num))
    {
        // Implementation defined what to return here
        return std::numeric_limits<Int>::min();
    }
    else if (abs(num) == zero)
    {
        return 0;
    }
    #else
    if (abs(num) == zero)
    {
        return zero;
    }
    #endif

    int expptr {};
    auto sig {frexp10(num, &expptr)}; // Always returns detail::precision digits
    const bool is_neg {num < 0};

    if (num > lmax)
    {
        return (std::numeric_limits<Int>::max)();
    }
    else if (num < lmin)
    {
        return (std::numeric_limits<Int>::min)();
    }

    if (expptr > detail::precision_v<T>)
    {
        return static_cast<Int>(num);
    }
    else if (expptr < -detail::precision_v<T>)
    {
        return 0;
    }

    detail::rint_impl(sig, expptr, is_neg);

    auto res {static_cast<Int>(sig)};
    if (is_neg)
    {
        res = -res;
    }

    return res;
}

#ifdef _MSC_VER
#  pragma warning(pop)
#endif

} //namespace detail

// Rounds the number using the default rounding mode
BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto rint(T num) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    const T zero {0, 0};
    constexpr T max_round_value {1 / std::numeric_limits<T>::epsilon()};

    #ifndef BOOST_DECIMAL_FAST_MATH
    if (isinf(num) || isnan(num) || abs(num) == zero || abs(num) > max_round_value)
    {
        return num;
    }
    #else
    if (abs(num) == zero || abs(num) > max_round_value)
    {
        return num;
    }
    #endif

    int expptr {};
    auto sig {frexp10(num, &expptr)}; // Always returns detail::precision digits
    const bool is_neg {num < 0};

    if (expptr > detail::precision_v<T>)
    {
        return num;
    }
    else if (expptr < -detail::precision_v<T>)
    {
        return is_neg ? -zero : zero;
    }

    detail::rint_impl(sig, expptr, is_neg);

    return {sig, 0, is_neg};
}

BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto lrint(T num) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, T, long)
{
    return detail::lrint_impl<T, long>(num);
}

BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto llrint(T num) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, T, long long)
{
    return detail::lrint_impl<T, long long>(num);
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_RINT_HPP
