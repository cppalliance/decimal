// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_FWD_HPP
#define BOOST_DECIMAL_FWD_HPP

#include <boost/decimal/detail/config.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <limits>

namespace boost {
namespace decimal {

class decimal32;
class decimal64;
class decimal128;
class decimal32_fast;

} // namespace decimal
} // namespace boost

namespace std {

template <>
#ifdef _MSC_VER
class numeric_limits<boost::decimal::decimal32>;
#else
struct numeric_limits<boost::decimal::decimal32>;
#endif

template <>
#ifdef _MSC_VER
class numeric_limits<boost::decimal::decimal64>;
#else
struct numeric_limits<boost::decimal::decimal64>;
#endif

template <>
#ifdef _MSC_VER
class numeric_limits<boost::decimal::decimal128>;
#else
struct numeric_limits<boost::decimal::decimal128>;
#endif

} // Namespace std

#endif // BOOST_DECIMAL_BUILD_MODULE

#endif // BOOST_DECIMAL_FWD_HPP
