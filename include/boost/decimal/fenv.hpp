// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_FENV_HPP
#define BOOST_DECIMAL_FENV_HPP

#ifdef _MSC_VER
// Expands to "This header is deprecated; use expr instead."
#  define BOOST_DECIMAL_HEADER_DEPRECATED(expr) __pragma("This header is deprecated; use " expr " instead.")
#else // GNU, Clang, Intel, IBM, etc.
// Expands to "This header is deprecated use expr instead"
#  define BOOST_DECIMAL_HEADER_DEPRECATED_MESSAGE(expr) _Pragma(#expr)
#  define BOOST_DECIMAL_HEADER_DEPRECATED(expr) BOOST_DECIMAL_HEADER_DEPRECATED_MESSAGE(message "This header is deprecated use " expr " instead")
#endif

BOOST_DECIMAL_HEADER_DEPRECATED("<boost/decimal/cfenv.hpp>");

#include <boost/decimal/cfenv.hpp>

#endif //BOOST_DECIMAL_FENV_HPP
