# Decimal
[![CI](https://github.com/cppalliance/decimal/actions/workflows/ci.yml/badge.svg?branch=develop)](https://github.com/cppalliance/decimal/actions/workflows/ci.yml)
[![Build Status](https://drone.cpp.al/api/badges/cppalliance/decimal/status.svg?ref=refs/heads/develop)](https://drone.cpp.al/cppalliance/decimal)
[![codecov](https://codecov.io/gh/cppalliance/decimal/graph/badge.svg?token=drvY8nnV5S)](https://codecov.io/gh/cppalliance/decimal)
---

An implementation of IEEE 754 decimal floating point numbers.
It is header only, and requires C++14

# Notice

This library is not an official boost library, and is under active development.

# How To Use The Library

This library is header only, and contains no dependencies.
It can either be copied into the directory of your choice, or installed with CMake.

# Supported Platforms

Boost.Decimal is tested on Ubuntu (x86_64, s390x, and aarch64), macOS (x86_64, and Apple Silicon), and Windows with the following compilers:

* GCC 7 and later
* Clang 6 and later
* Visual Studio 2017 and later

# Synopsis

Decimal provides 3 types: 

````
namespace boost {
namespace decimal {

class decimal32;
class decimal64;
class decimal128;

} //namespace decimal
} //namespace boost
````

These types operate like built-in floating point types, and have their own implementations of the STL functions (e.g. cmath, charconv, cstdlib, etc.).
The entire library can be conveniently included with `#include <boost/decimal.hpp>`

Using the types is simple:

````
#include <boost/decimal.hpp>
#include <iostream>

int main()
{
    using boost::decimal::decimal32;
    
    constexpr decimal32 a {2, -1}; // Constructs the number 0.2
    constexpr decimal32 b {1, -1}; // Constructs the number 0.1
    auto sum {a + b};

    std::cout << sum << std::endl; // prints 0.3

    const auto neg_a {2, -1, true}; // Constructs the number -0.2

    sum += neg_a;

    std::cout << sum << std::endl; // Prints 0.1

    return 0;
}
````

Same with using STL functions:

````
#include <boost/decimal.hpp>
#include <cassert>
#include <cstring>

int main()
{
    using namespace boost::decimal;

    decimal64 val {-0.25}; // Construction from a double
    val = abs(val); // DO NOT call std::abs

    char buffer[256];
    auto r_to = to_chars(buffer, buffer + sizeof(buffer) - 1, val);
    assert(r_to); // checks std::errc()
    *r_to.ptr = '\0';

    decimal64 return_value;
    auto r_from = from_chars(buffer, buffer + std::strlen(buffer), return_value);

    assert(val == return_value);

    return 0;
}
````

# Full Documentation

The complete documentation can be found at: https://cppalliance.org/decimal/overview.html
