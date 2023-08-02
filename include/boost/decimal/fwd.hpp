// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_FWD_HPP
#define BOOST_DECIMAL_FWD_HPP

#include <limits>

namespace boost { namespace decimal {

class decimal32;

}} // Namespace boost::decimal

namespace std {

template <>
struct numeric_limits<boost::decimal::decimal32>;

} // Namespace std

#endif // BOOST_DECIMAL_FWD_HPP
