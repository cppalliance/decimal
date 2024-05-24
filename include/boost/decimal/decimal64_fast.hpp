// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DECIMAL64_FAST_HPP
#define BOOST_DECIMAL_DECIMAL64_FAST_HPP

namespace boost {
namespace decimal {

class decimal64_fast final
{

};

} // namespace decimal
} // namespace boost

namespace std {

template <>
#ifdef _MSC_VER
class numeric_limits<boost::decimal::decimal64_fast>;
#else
struct numeric_limits<boost::decimal::decimal64_fast>
#endif
{

};

} // namespace std

#endif //BOOST_DECIMAL_DECIMAL64_FAST_HPP
