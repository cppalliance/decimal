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
class decimal32_fast;
class decimal64;
class decimal64_fast;
class decimal128;
class decimal128_fast;

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
class numeric_limits<boost::decimal::decimal32_fast>;
#else
struct numeric_limits<boost::decimal::decimal32_fast>;
#endif

template <>
#ifdef _MSC_VER
class numeric_limits<boost::decimal::decimal64>;
#else
struct numeric_limits<boost::decimal::decimal64>;
#endif

template <>
#ifdef _MSC_VER
class numeric_limits<boost::decimal::decimal64_fast>;
#else
struct numeric_limits<boost::decimal::decimal64_fast>;
#endif

template <>
#ifdef _MSC_VER
class numeric_limits<boost::decimal::decimal128>;
#else
struct numeric_limits<boost::decimal::decimal128>;
#endif

template <>
#ifdef _MSC_VER
class numeric_limits<boost::decimal::decimal128_fast>;
#else
struct numeric_limits<boost::decimal::decimal128_fast>;
#endif

} // Namespace std

#endif // BOOST_DECIMAL_BUILD_MODULE

#endif // BOOST_DECIMAL_FWD_HPP
