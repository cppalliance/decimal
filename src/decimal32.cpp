// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal/decimal32.hpp>

namespace boost { namespace decimal {

namespace detail {

// Values from IEEE 754-2019 table 3.6
BOOST_ATTRIBUTE_UNUSED static constexpr auto storage_width = 32;
BOOST_ATTRIBUTE_UNUSED static constexpr auto precision = 7;
BOOST_ATTRIBUTE_UNUSED static constexpr auto emax = 96;
BOOST_ATTRIBUTE_UNUSED static constexpr auto bias = 101;
BOOST_ATTRIBUTE_UNUSED static constexpr auto combination_field_width = 11;
BOOST_ATTRIBUTE_UNUSED static constexpr auto trailing_significand_field_width = 20;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t sign_bit_flag = 1 << 31;

} // Namespace detail

// True if negative otherwise false
constexpr bool signbit(decimal32 rhs) noexcept
{
    return rhs.bits & detail::sign_bit_flag;
}

constexpr decimal32 operator+(decimal32 rhs) noexcept
{
    return rhs;
}

constexpr decimal32 operator-(decimal32 rhs) noexcept
{
    return decimal32{rhs.bits ^= detail::sign_bit_flag};
}

}} // Namespace boost::decimal
