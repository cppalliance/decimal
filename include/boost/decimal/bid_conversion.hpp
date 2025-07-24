// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_BID_CONVERSION_HPP
#define BOOST_DECIMAL_BID_CONVERSION_HPP

#include <boost/decimal/decimal32_t.hpp>
#include <boost/decimal/decimal64_t.hpp>
#include <boost/decimal/decimal128_t.hpp>
#include <boost/decimal/decimal_fast32_t.hpp>
#include <boost/decimal/decimal_fast64_t.hpp>
#include <boost/decimal/decimal_fast128_t.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include "detail/int128.hpp"

namespace boost {
namespace decimal {

#if defined(__GNUC__) && __GNUC__ == 7
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wconversion"
#endif

BOOST_DECIMAL_EXPORT constexpr auto to_bid_d32(decimal32_t val) noexcept -> std::uint32_t
{
    return val.bits_;
}

BOOST_DECIMAL_EXPORT constexpr auto from_bid_d32(std::uint32_t bits) noexcept -> decimal32_t
{
    return from_bits(bits);
}

BOOST_DECIMAL_EXPORT constexpr auto to_bid_d32f(decimal_fast32_t val) noexcept -> std::uint32_t
{
    const decimal32_t compliant_val {val};
    return to_bid_d32(compliant_val);
}

BOOST_DECIMAL_EXPORT constexpr auto from_bid_d32f(std::uint32_t bits) noexcept -> decimal_fast32_t
{
    const auto compliant_val {from_bid_d32(bits)};
    const decimal_fast32_t val {compliant_val};
    return val;
}

BOOST_DECIMAL_EXPORT constexpr auto to_bid_d64(decimal64_t val) noexcept -> std::uint64_t
{
    return val.bits_;
}

BOOST_DECIMAL_EXPORT constexpr auto from_bid_d64(std::uint64_t bits) noexcept -> decimal64_t
{
    return from_bits(bits);
}

BOOST_DECIMAL_EXPORT constexpr auto to_bid_d64f(decimal_fast64_t val) noexcept -> std::uint64_t
{
    const decimal64_t compliant_val {val};
    return to_bid_d64(compliant_val);
}

BOOST_DECIMAL_EXPORT constexpr auto from_bid_d64f(std::uint64_t bits) noexcept -> decimal_fast64_t
{
    const auto compliant_val {from_bid_d64(bits)};
    const decimal_fast64_t val {compliant_val};
    return val;
}

BOOST_DECIMAL_EXPORT constexpr auto to_bid_d128(decimal128_t val) noexcept -> int128::uint128_t
{
    return val.bits_;
}

BOOST_DECIMAL_EXPORT constexpr auto from_bid_d128(int128::uint128_t bits) noexcept -> decimal128_t
{
    return from_bits(bits);
}

#ifdef BOOST_DECIMAL_HAS_INT128
BOOST_DECIMAL_EXPORT constexpr auto from_bid_d128(detail::builtin_uint128_t bits) noexcept -> decimal128_t
{
    return from_bits(bits);
}
#endif

BOOST_DECIMAL_EXPORT constexpr auto to_bid_d128f(decimal_fast128_t val) noexcept -> int128::uint128_t
{
    const decimal128_t compliant_val {val};
    return to_bid_d128(compliant_val);
}

BOOST_DECIMAL_EXPORT constexpr auto from_bid_d128f(int128::uint128_t bits) noexcept -> decimal_fast128_t
{
    const auto compliant_val {from_bid_d128(bits)};
    const decimal_fast128_t val {compliant_val};
    return val;
}

#ifdef BOOST_DECIMAL_HAS_INT128
BOOST_DECIMAL_EXPORT constexpr auto from_bid_d128f(detail::builtin_uint128_t bits) noexcept -> decimal_fast128_t
{
    const auto compliant_val {from_bid_d128(bits)};
    const decimal_fast128_t val {compliant_val};
    return val;
}
#endif

BOOST_DECIMAL_EXPORT constexpr auto to_bid(decimal32_t val) noexcept -> std::uint32_t
{
    return to_bid_d32(val);
}

BOOST_DECIMAL_EXPORT constexpr auto to_bid(decimal_fast32_t val) noexcept -> std::uint32_t
{
    return to_bid_d32f(val);
}

BOOST_DECIMAL_EXPORT constexpr auto to_bid(decimal64_t val) noexcept -> std::uint64_t
{
    return to_bid_d64(val);
}

BOOST_DECIMAL_EXPORT constexpr auto to_bid(decimal_fast64_t val) noexcept -> std::uint64_t
{
    return to_bid_d64f(val);
}

BOOST_DECIMAL_EXPORT constexpr auto to_bid(decimal128_t val) noexcept -> int128::uint128_t
{
    return to_bid_d128(val);
}

BOOST_DECIMAL_EXPORT constexpr auto to_bid(decimal_fast128_t val) noexcept -> int128::uint128_t
{
    return to_bid_d128f(val);
}

BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto to_bid(T val) noexcept
{
    return to_bid(val);
}

BOOST_DECIMAL_EXPORT template <typename T = decimal_fast32_t>
constexpr auto from_bid(std::uint32_t bits) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    return from_bid_d32f(bits);
}

BOOST_DECIMAL_EXPORT template <>
constexpr auto from_bid<decimal32_t>(std::uint32_t bits) noexcept -> decimal32_t
{
    return from_bid_d32(bits);
}

BOOST_DECIMAL_EXPORT template <typename T = decimal_fast64_t>
constexpr auto from_bid(std::uint64_t bits) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    return from_bid_d64f(bits);
}

BOOST_DECIMAL_EXPORT template <>
constexpr auto from_bid<decimal64_t>(std::uint64_t bits) noexcept -> decimal64_t
{
    return from_bid_d64(bits);
}

BOOST_DECIMAL_EXPORT template <typename T = decimal_fast128_t>
constexpr auto from_bid(int128::uint128_t bits) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    return from_bid_d128f(bits);
}

BOOST_DECIMAL_EXPORT template <>
constexpr auto from_bid<decimal128_t>(int128::uint128_t bits) noexcept -> decimal128_t
{
    return from_bid_d128(bits);
}

#if defined(__GNUC__) && __GNUC__ == 7
#  pragma GCC diagnostic pop
#endif

} // namespace decimal
} // namespace boost

#endif //BOOST_BID_CONVERSION_HPP
