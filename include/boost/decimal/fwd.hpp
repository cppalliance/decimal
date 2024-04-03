// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_FWD_HPP
#define BOOST_DECIMAL_FWD_HPP

#include <limits>
#include <boost/decimal/detail/config.hpp>

namespace boost { namespace decimal {

class decimal32;
class decimal64;
class decimal128;

} // namespace decimal
} // namespace boost

namespace std {

template <>
#ifdef _MSC_VER
BOOST_DECIMAL_EXPORT class numeric_limits<boost::decimal::decimal32>;
#else
BOOST_DECIMAL_EXPORT struct numeric_limits<boost::decimal::decimal32>;
#endif

template <>
#ifdef _MSC_VER
BOOST_DECIMAL_EXPORT class numeric_limits<boost::decimal::decimal64>;
#else
BOOST_DECIMAL_EXPORT struct numeric_limits<boost::decimal::decimal64>;
#endif

template <>
#ifdef _MSC_VER
BOOST_DECIMAL_EXPORT class numeric_limits<boost::decimal::decimal128>;
#else
BOOST_DECIMAL_EXPORT struct numeric_limits<boost::decimal::decimal128>;
#endif

} // Namespace std

#endif // BOOST_DECIMAL_FWD_HPP
