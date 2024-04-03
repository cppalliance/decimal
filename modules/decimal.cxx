// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

// Global module fragment required for non-module preprocessing
module;

// Should be able to add a macro check for the following
//import std;
//import std.compat;

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

#include <boost/decimal.hpp>
