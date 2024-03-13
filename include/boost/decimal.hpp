// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_HPP
#define BOOST_DECIMAL_HPP

// Clang-Cl likes to throw warnings everywhere for this
// disable at the global level
#if defined(__clang__) && !defined(__GNUC__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#endif

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/decimal32.hpp>
#include <boost/decimal/decimal64.hpp>
#include <boost/decimal/decimal128.hpp>
#include <boost/decimal/cmath.hpp>
#include <boost/decimal/cstdlib.hpp>
#include <boost/decimal/fenv.hpp>
#include <boost/decimal/literals.hpp>
#include <boost/decimal/hash.hpp>
#include <boost/decimal/cfloat.hpp>
#include <boost/decimal/charconv.hpp>
#include <boost/decimal/type_traits.hpp>
#include <boost/decimal/detail/io.hpp>

#if defined(__clang__) && !defined(__GNUC__)
#  pragma clang diagnostic pop
#endif

#endif // BOOST_DECIMAL_HPP
