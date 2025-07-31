// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_NAN_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_NAN_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/cstdlib.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <limits>
#endif

#if !defined(BOOST_DECIMAL_DISABLE_CLIB)

namespace boost {
namespace decimal {

namespace detail {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE TargetDecimalType>
constexpr auto nan_impl(const char* arg) noexcept -> TargetDecimalType
{
    char* endptr {};
    const auto val {strtod_impl<TargetDecimalType>(arg, &endptr)};
    return val & std::numeric_limits<TargetDecimalType>::quiet_NaN();
}

} //namespace detail

BOOST_DECIMAL_EXPORT constexpr auto nand32(const char* arg) noexcept -> decimal32_t
{
    return detail::nan_impl<decimal32_t>(arg);
}

BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto nan(const char* arg) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    return detail::nan_impl<T>(arg);
}

BOOST_DECIMAL_EXPORT constexpr auto nand64(const char* arg) noexcept -> decimal64_t
{
    return detail::nan_impl<decimal64_t>(arg);
}

BOOST_DECIMAL_EXPORT constexpr auto nand128(const char* arg) noexcept -> decimal128_t
{
    return detail::nan_impl<decimal128_t>(arg);
}

} //namespace decimal
} //namespace boost

#endif //#if !defined(BOOST_DECIMAL_DISABLE_CLIB)

#endif //BOOST_DECIMAL_DETAIL_CMATH_NAN_HPP
