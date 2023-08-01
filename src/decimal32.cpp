// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal/decimal32.hpp>

namespace boost { namespace decimal {

namespace detail {

// See section 3.5.2
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t sign_bit_flag = 1 << 31;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t nan_flag = 0b0111'1100'0000'0000'0000'0000'0000'0000;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t snan_flag = 0b0111'1110'0000'0000'0000'0000'0000'0000;

// Values from IEEE 754-2019 table 3.6
BOOST_ATTRIBUTE_UNUSED static constexpr auto storage_width = 32;
BOOST_ATTRIBUTE_UNUSED static constexpr auto precision = 7;
BOOST_ATTRIBUTE_UNUSED static constexpr auto emax = 96;
BOOST_ATTRIBUTE_UNUSED static constexpr auto bias = 101;
BOOST_ATTRIBUTE_UNUSED static constexpr auto combination_field_width = 11;
BOOST_ATTRIBUTE_UNUSED static constexpr auto trailing_significand_field_width = 20;

} // Namespace detail

// True if negative otherwise false
constexpr bool signbit(decimal32 rhs) noexcept
{
    return rhs.bits & detail::sign_bit_flag;
}

constexpr bool isnan(decimal32 rhs) noexcept
{
    return rhs.bits & detail::nan_flag;
}

constexpr bool issignaling(decimal32 rhs) noexcept
{
    return rhs.bits & detail::snan_flag;
}

constexpr decimal32 operator+(decimal32 rhs) noexcept
{
    return rhs;
}

constexpr decimal32 operator-(decimal32 rhs) noexcept
{
    return decimal32{rhs.bits ^= detail::sign_bit_flag};
}

constexpr bool operator==(decimal32 lhs, decimal32 rhs) noexcept
{
    return lhs.bits == rhs.bits;
}

constexpr bool operator!=(decimal32 lhs, decimal32 rhs) noexcept
{
    return lhs.bits != rhs.bits;
}

}} // Namespace boost::decimal
