// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_TUPLE_HPP
#define BOOST_DECIMAL_DETAIL_TUPLE_HPP

#include <boost/math/tools/config.hpp>

#ifdef BOOST_MATH_ENABLE_CUDA

#include <thrust/pair.h>
#include <thrust/tuple.h>

namespace boost { 
namespace decimal {

using thrust::pair;
using thrust::tuple;

using thrust::make_pair;
using thrust::make_tuple;

using thrust::tie;
using thrust::get;

using thrust::tuple_size;
using thrust::tuple_element;

} // namespace decimal
} // namespace boost

#else

#include <tuple>

namespace boost { 
namespace decimal {

using ::std::tuple;
using ::std::pair;

// [6.1.3.2] Tuple creation functions
using ::std::ignore;
using ::std::make_tuple;
using ::std::tie;
using ::std::get;

// [6.1.3.3] Tuple helper classes
using ::std::tuple_size;
using ::std::tuple_element;

// Pair helpers
using ::std::make_pair;

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_HAS_CUDA

#endif // BOOST_DECIMAL_DETAIL_TUPLE_HPP
