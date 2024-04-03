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

#define BOOST_DECIMAL_BUILD_MODULE

export module boost2.decimal;

#include <boost/decimal.hpp>
