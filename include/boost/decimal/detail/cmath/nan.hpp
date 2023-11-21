// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_NAN_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_NAN_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/cstdlib.hpp>
#include <limits>

#if !defined(BOOST_DECIMAL_DISABLE_CLIB)

namespace boost {
namespace decimal {

namespace detail {

template <typename TargetDecimalType>
constexpr auto nan_impl(const char* arg) noexcept -> TargetDecimalType
{
    char* endptr {};
    const auto val {strtod_impl<TargetDecimalType>(arg, &endptr)};
    return val & std::numeric_limits<TargetDecimalType>::quiet_NaN();
}

} //namespace detail

constexpr auto nand32(const char* arg) noexcept -> decimal32
{
    return detail::nan_impl<decimal32>(arg);
}

template <typename Dec>
constexpr auto nan(const char* arg) noexcept -> Dec
{
    return detail::nan_impl<Dec>(arg);
}

constexpr auto nand64(const char* arg) noexcept -> decimal64
{
    return detail::nan_impl<decimal64>(arg);
}

constexpr auto nand128(const char* arg) noexcept -> decimal128
{
    return detail::nan_impl<decimal128>(arg);
}

} //namespace decimal
} //namespace boost

#endif //#if !defined(BOOST_DECIMAL_DISABLE_CLIB)

#endif //BOOST_DECIMAL_DETAIL_CMATH_NAN_HPP
