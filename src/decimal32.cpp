// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal/decimal32.hpp>
#include <iostream>
#include <cmath>
#include <cstring>

namespace boost { namespace decimal {

bool signbit(decimal32 rhs) noexcept
{
    return rhs.bits_.sign;
}

bool isinf(decimal32 rhs) noexcept
{
    return rhs.bits_.combination_field & detail::inf_mask;
}

bool isnan(decimal32 rhs) noexcept
{
    return rhs.bits_.combination_field & detail::nan_mask;
}

bool issignaling(decimal32 rhs) noexcept
{
    return isnan(rhs) && rhs.bits_.exponent & detail::snan_mask;
}

bool isfinite(decimal32 rhs) noexcept
{
    return !isinf(rhs) && !isnan(rhs);
}

decimal32 operator+(decimal32 rhs) noexcept
{
    return rhs;
}

decimal32 operator-(decimal32 rhs) noexcept
{
    rhs.bits_.sign ^= 1;
    return rhs;
}

bool operator==(decimal32 lhs, decimal32 rhs) noexcept
{
    return lhs.bits_.sign == rhs.bits_.sign &&
           lhs.bits_.combination_field == rhs.bits_.combination_field &&
           lhs.bits_.exponent == rhs.bits_.exponent &&
           lhs.bits_.significand == rhs.bits_.significand;
}

bool operator!=(decimal32 lhs, decimal32 rhs) noexcept
{
    return !(lhs == rhs);
}

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
