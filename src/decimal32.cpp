// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal/decimal32.hpp>
#include <cmath>
#include <cstring>

namespace boost { namespace decimal {

namespace detail {

// See section 3.5.2
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t inf_mask = 0b11110;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t nan_mask = 0b11111;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t snan_mask = 0b100000;

// Values from IEEE 754-2019 table 3.6
BOOST_ATTRIBUTE_UNUSED static constexpr auto storage_width = 32;
BOOST_ATTRIBUTE_UNUSED static constexpr auto precision = 7;
BOOST_ATTRIBUTE_UNUSED static constexpr auto emax = 96;
BOOST_ATTRIBUTE_UNUSED static constexpr auto bias = 101;
BOOST_ATTRIBUTE_UNUSED static constexpr auto combination_field_width = 11;
BOOST_ATTRIBUTE_UNUSED static constexpr auto trailing_significand_field_width = 20;

// Other useful values
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t max_significand = 9'999'999;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t max_binary_significand = 0b1001'1000100101'1001111111;
BOOST_ATTRIBUTE_UNUSED static constexpr auto max_string_length = 15;

// Masks for the combination field since we use the binary encoding for the significand
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t g0_mask = 0b10000;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t g1_mask = 0b01000;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t g2_mask = 0b00100;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t g3_mask = 0b00010;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t g4_mask = 0b00001;

} // Namespace detail

decimal32::decimal32(long long coeff, int exp) noexcept
{
    // If the coefficient fits directly we don't need to use the combination field
    if (std::abs(coeff) < (1 << 20))
    {
        this->bits_.significand = std::abs(coeff);
        if (coeff < 0)
        {
            this->bits_.sign = 1;
        }
    }

    // If the exponent fits we do not need to use the combination field
    if (exp + detail::bias < (1 << 6))
    {
        this->bits_.exponent = exp + detail::bias;
    }

    // TODO(mborland): Handle the cases that need the combination field
    this->bits_.combination_field = 0;
}

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

}} // Namespace boost::decimal
