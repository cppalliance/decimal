// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_CSTDLIB_HPP
#define BOOST_DECIMAL_CSTDLIB_HPP

#include <boost/decimal/decimal32.hpp>

namespace boost { namespace decimal {

constexpr decimal32 strtod(const char* str, char** endptr) noexcept
{
    return strtod32(str, endptr);
}

}} // Namespaces

#endif //BOOST_DECIMAL_CSTDLIB_HPP
