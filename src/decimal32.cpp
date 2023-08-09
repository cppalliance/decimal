// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal/decimal32.hpp>
#include <iostream>
#include <cmath>
#include <cstring>

namespace boost { namespace decimal {

// TODO(mborland): Add handling for values with non-zero combination field
// TODO(mborland): Add decimal point. Would be nice to use charconv here
std::ostream& operator<<(std::ostream& os, const decimal32& d)
{
    if (d.bits_.sign == 1)
    {
        os << "-";
    }

    os << d.bits_.significand << "e";

    if (d.bits_.exponent - detail::bias < 0)
    {
        os << d.bits_.exponent - detail::bias;
    }
    else
    {
        os << '+' << d.bits_.exponent - detail::bias;
    }

    return os;
}

std::uint32_t to_bits(decimal32 rhs) noexcept
{
    std::uint32_t bits;
    std::memcpy(&bits, &rhs.bits_, sizeof(std::uint32_t));
    return bits;
}

}} // Namespace boost::decimal
