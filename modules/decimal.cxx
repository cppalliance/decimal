// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

// Global module fragment required for non-module preprocessing
module;

#include <version>
#include <memory>
#include <new>
#include <limits>
#include <locale>
#include <type_traits>
#include <iostream>
#include <sstream>
#include <array>
#include <utility>
#include <algorithm>
#include <iterator>
#include <initializer_list>
#include <bit>
#include <concepts>
#include <functional>
#include <iomanip>
#include <iosfwd>
#include <ios>
#include <ostream>
#include <tuple>
#include <system_error>
#include <complex>
#include <compare>
#include <charconv>

// <stdfloat> is a C++23 feature that is not everywhere yet
#if __has_include(<stdfloat>)
#  include <stdfloat>
#endif

#include <cfenv>
#include <cfloat>
#include <cstdint>
#include <clocale>
#include <cstring>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cwchar>
#include <cstddef>
#include <cinttypes>
#include <cstdlib>
#include <cassert>

#if defined(_MSC_VER)
#  include <intrin.h>
#elif defined(__x86_64__)
#  include <x86intrin.h>
#elif defined(__ARM_NEON__)
#  include <arm_neon.h>
#endif

#define BOOST_DECIMAL_BUILD_MODULE

export module boost2.decimal;

// Forward declarations are not available so add the contents of fwd.hpp here

export namespace boost::decimal {

class decimal32;
class decimal64;
class decimal128;

class decimal32_fast;
class decimal64_fast;
class decimal128_fast;

} // namespace boost::decimal

export namespace std {

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

template <>
#ifdef _MSC_VER
class numeric_limits<boost::decimal::decimal32>;
#else
struct numeric_limits<boost::decimal::decimal32_fast>;
#endif

template <>
#ifdef _MSC_VER
class numeric_limits<boost::decimal::decimal64>;
#else
struct numeric_limits<boost::decimal::decimal64_fast>;
#endif

template <>
#ifdef _MSC_VER
class numeric_limits<boost::decimal::decimal128>;
#else
struct numeric_limits<boost::decimal::decimal128_fast>;
#endif

} // Namespace std

// MSVC wants <boost/decimal> to be imported but also does not support importing it...
#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable : 5244 )
#elif defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#endif

#include <boost/decimal.hpp>

#ifdef _MSC_VER
#  pragma warning( pop )
#elif defined(__clang__)
#  pragma clang diagnostic pop
#endif
