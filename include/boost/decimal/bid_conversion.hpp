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

#if defined(__GNUC__) && __GNUC__ == 7
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wconversion"
#endif

constexpr auto to_bid_d32(decimal32 val) noexcept -> std::uint32_t
{
    return val.bits_;
}

constexpr auto from_bid_d32(std::uint32_t bits) noexcept -> decimal32
{
    return from_bits(bits);
}

constexpr auto to_bid_d32f(decimal32_fast val) noexcept -> std::uint32_t
{
    const decimal32 compliant_val {val};
    return to_bid_d32(compliant_val);
}

constexpr auto from_bid_d32f(std::uint32_t bits) noexcept -> decimal32_fast
{
    const auto compliant_val {from_bid_d32(bits)};
    const decimal32_fast val {compliant_val};
    return val;
}

constexpr auto to_bid_d64(decimal64 val) noexcept -> std::uint64_t
{
    return val.bits_;
}

constexpr auto from_bid_d64(std::uint64_t bits) noexcept -> decimal64
{
    return from_bits(bits);
}

constexpr auto to_bid_d64f(decimal64_fast val) noexcept -> std::uint64_t
{
    const decimal64 compliant_val {val};
    return to_bid_d64(compliant_val);
}

constexpr auto from_bid_d64f(std::uint64_t bits) noexcept -> decimal64_fast
{
    const auto compliant_val {from_bid_d64(bits)};
    const decimal64_fast val {compliant_val};
    return val;
}

constexpr auto to_bid_d128(decimal128 val) noexcept -> detail::uint128
{
    return val.bits_;
}

constexpr auto from_bid_d128(detail::uint128 bits) noexcept -> decimal128
{
    return from_bits(bits);
}

#ifdef BOOST_DECIMAL_HAS_INT128
constexpr auto from_bid_d128(detail::uint128_t bits) noexcept -> decimal128
{
    return from_bits(bits);
}
#endif

constexpr auto to_bid_d128f(decimal128_fast val) noexcept -> detail::uint128
{
    const decimal128 compliant_val {val};
    return to_bid_d128(compliant_val);
}

constexpr auto from_bid_d128f(detail::uint128 bits) noexcept -> decimal128_fast
{
    const auto compliant_val {from_bid_d128(bits)};
    const decimal128_fast val {compliant_val};
    return val;
}

#ifdef BOOST_DECIMAL_HAS_INT128
constexpr auto from_bid_d128f(detail::uint128_t bits) noexcept -> decimal128_fast
{
    const auto compliant_val {from_bid_d128(bits)};
    const decimal128_fast val {compliant_val};
    return val;
}
#endif

constexpr auto to_bid(decimal32 val) noexcept -> std::uint32_t
{
    return to_bid_d32(val);
}

constexpr auto to_bid(decimal32_fast val) noexcept -> std::uint32_t
{
    return to_bid_d32f(val);
}

constexpr auto to_bid(decimal64 val) noexcept -> std::uint64_t
{
    return to_bid_d64(val);
}

constexpr auto to_bid(decimal64_fast val) noexcept -> std::uint64_t
{
    return to_bid_d64f(val);
}

constexpr auto to_bid(decimal128 val) noexcept -> detail::uint128
{
    return to_bid_d128(val);
}

constexpr auto to_bid(decimal128_fast val) noexcept -> detail::uint128
{
    return to_bid_d128f(val);
}

template <typename T>
constexpr auto to_bid(T val) noexcept
{
    return to_bid(val);
}

template <typename T = decimal32_fast>
constexpr auto from_bid(std::uint32_t bits) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    return from_bid_d32f(bits);
}

template <>
constexpr auto from_bid<decimal32>(std::uint32_t bits) noexcept -> decimal32
{
    return from_bid_d32(bits);
}

template <typename T = decimal64_fast>
constexpr auto from_bid(std::uint64_t bits) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    return from_bid_d64f(bits);
}

template <>
constexpr auto from_bid<decimal64>(std::uint64_t bits) noexcept -> decimal64
{
    return from_bid_d64(bits);
}

template <typename T = decimal128_fast>
constexpr auto from_bid(detail::uint128 bits) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    return from_bid_d128f(bits);
}

template <>
constexpr auto from_bid<decimal128>(detail::uint128 bits) noexcept -> decimal128
{
    return from_bid_d128(bits);
}

#if defined(__GNUC__) && __GNUC__ == 7
#  pragma GCC diagnostic pop
#endif

} // namespace decimal
} // namespace boost

#endif //BOOST_BID_CONVERSION_HPP
