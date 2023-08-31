// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_CSTDLIB_HPP
#define BOOST_DECIMAL_CSTDLIB_HPP

#include <boost/decimal/decimal32.hpp>

namespace boost { namespace decimal {

// TODO(mborland): To match regular strtod these should return decimal64 once it exists

constexpr decimal32 strtod(const char* str, char** endptr) noexcept
{
    return strtod32(str, endptr);
}

constexpr decimal32 wcstod(const wchar_t* str, wchar_t** endptr) noexcept
{
    return wcstod32(str, endptr);
}

}} // Namespaces

#endif //BOOST_DECIMAL_CSTDLIB_HPP
