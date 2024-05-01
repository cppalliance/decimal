// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt


#ifndef BOOST_DECIMAL_DETAIL_TO_STRING
#define BOOST_DECIMAL_DETAIL_TO_STRING

#include <boost/decimal/charconv.hpp>

#if !defined(BOOST_DECIMAL_DISABLE_CLIB)

namespace boost {
namespace decimal {

template <typename DecimalType>
auto to_string(DecimalType value)
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, DecimalType, std::string)
{
    char buffer[44];
    auto r = to_chars(buffer, buffer + sizeof(buffer), value, chars_format::fixed, 6);
    *r.ptr = '\0';
    return std::string(buffer);
}

} //namespace decimal
} //namespace boost

#endif

#endif //BOOST_DECIMAL_DETAIL_TO_STRING
