// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_BID_CONVERSION_HPP
#define BOOST_DECIMAL_BID_CONVERSION_HPP

#include <boost/decimal/decimal32.hpp>
#include <boost/decimal/decimal64.hpp>
#include <boost/decimal/decimal128.hpp>
#include <boost/decimal/decimal32_fast.hpp>
#include <boost/decimal/decimal64_fast.hpp>
#include <boost/decimal/detail/concepts.hpp>

namespace boost {
namespace decimal {

BOOST_DECIMAL_CXX20_CONSTEXPR auto to_bid(decimal32 val) noexcept -> std::uint32_t
{
    return to_bid_d32(val);
}

BOOST_DECIMAL_CXX20_CONSTEXPR auto to_bid(decimal32_fast val) noexcept -> std::uint32_t
{
    return to_bid_d32f(val);
}

BOOST_DECIMAL_CXX20_CONSTEXPR auto to_bid(decimal64 val) noexcept -> std::uint64_t
{
    return to_bid_d64(val);
}

BOOST_DECIMAL_CXX20_CONSTEXPR auto to_bid(decimal64_fast val) noexcept -> std::uint64_t
{
    return to_bid_d64f(val);
}

BOOST_DECIMAL_CXX20_CONSTEXPR auto to_bid(decimal128 val) noexcept -> detail::uint128
{
    return to_bid_d128(val);
}

template <typename T = decimal32_fast>
BOOST_DECIMAL_CXX20_CONSTEXPR auto from_bid(std::uint32_t bits) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    return from_bid_d32f(bits);
}

template <>
BOOST_DECIMAL_CXX20_CONSTEXPR auto from_bid<decimal32>(std::uint32_t bits) noexcept -> decimal32
{
    return from_bid_d32(bits);
}

template <typename T = decimal64_fast>
BOOST_DECIMAL_CXX20_CONSTEXPR auto from_bid(std::uint64_t bits) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    return from_bid_d64f(bits);
}

template <>
BOOST_DECIMAL_CXX20_CONSTEXPR auto from_bid<decimal64>(std::uint64_t bits) noexcept -> decimal64
{
    return from_bid_d64(bits);
}

template <typename T = decimal128>
BOOST_DECIMAL_CXX20_CONSTEXPR auto from_bid(detail::uint128 bits) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    return from_bid_d128(bits);
}

} // namespace decimal
} // namespace boost

#endif //BOOST_BID_CONVERSION_HPP
