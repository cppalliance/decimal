// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal/decimal32.hpp>
#include <iostream>
#include <cmath>
#include <cstring>

namespace boost { namespace decimal {

std::ostream& operator<<(std::ostream& os, const decimal32& d)
{
    if (d.bits_.sign == 1)
    {
        os << "-";
    }

    os << d.full_significand() << "e";

    const auto print_exp = static_cast<int>(d.full_exponent()) - detail::bias;

    if (print_exp < 0)
    {
        os << '-';
    }
    else
    {
        os << '+';
    }

    if (abs(print_exp) < 10)
    {
        os << '0';
    }

    os << print_exp;

    return os;
}

std::uint32_t to_bits(decimal32 rhs) noexcept
{
    std::uint32_t bits;
    std::memcpy(&bits, &rhs.bits_, sizeof(std::uint32_t));
    return bits;
}

}} // Namespace boost::decimal
