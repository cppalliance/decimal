// Copyright 2023 - 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// This is not a fully featured implementation of a 256-bit integer like int128::uint128_t is
// u256 only contains the minimum amount that we need to perform operations like decimal128 mul

#ifndef BOOST_DECIMAL_DETAIL_U256_HPP
#define BOOST_DECIMAL_DETAIL_U256_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/int128.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <cstdint>
#include <cmath>
#include <tuple>
#endif

namespace boost {
namespace decimal {
namespace detail {

struct alignas(sizeof(std::uint64_t) * 4)
u256
{
    std::uint64_t bytes[4] {};

    // Constructors
    constexpr u256() noexcept = default;
    constexpr u256(const u256& other) noexcept = default;
    constexpr u256(u256&& other) noexcept = default;
    constexpr u256& operator=(const u256& other) noexcept = default;
    constexpr u256& operator=(u256&& other) noexcept = default;

    constexpr u256(std::uint64_t byte3, std::uint64_t byte2, std::uint64_t byte1, std::uint64_t byte0) noexcept;

    // Conversion to/from int128::uint128_t
    constexpr u256(const int128::uint128_t& high_, const int128::uint128_t& low_) noexcept;
    explicit constexpr operator int128::uint128_t() const noexcept;

    explicit constexpr operator std::size_t() const noexcept { return static_cast<std::size_t>(bytes[0]); }

    constexpr std::uint64_t operator[](std::size_t i) const noexcept;
    constexpr std::uint64_t& operator[](std::size_t i) noexcept;
};

constexpr u256::u256(const int128::uint128_t& high_, const int128::uint128_t& low_) noexcept
{
    bytes[0] = low_.low;
    bytes[1] = low_.high;
    bytes[2] = high_.low;
    bytes[3] = high_.high;
}

constexpr u256::operator int128::uint128_t() const noexcept
{
    return int128::uint128_t {bytes[1], bytes[0]};
}

constexpr std::uint64_t u256::operator[](const std::size_t i) const noexcept
{
    BOOST_DECIMAL_ASSERT(i < 4);
    return bytes[i];
}

constexpr std::uint64_t& u256::operator[](const std::size_t i) noexcept
{
    BOOST_DECIMAL_ASSERT(i < 4);
    return bytes[i];
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_U256_HPP
