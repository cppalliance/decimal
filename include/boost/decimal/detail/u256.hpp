// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_U256_HPP
#define BOOST_DECIMAL_DETAIL_U256_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/wide-integer/uintwide_t.hpp>
#include <boost/decimal/detail/u128.hpp>
#include <cstdint>

namespace boost {
namespace decimal {
namespace detail {

struct u256
{
    std::uint64_t bytes[4] {};

    // Constructors
    constexpr u256() noexcept = default;
    constexpr u256(const u256& other) = default;
    constexpr u256(u256&& other) noexcept = default;
    constexpr u256& operator=(const u256& other) = default;
    constexpr u256& operator=(u256&& other) noexcept = default;

    // Construction from uints
    constexpr u256(const u128& high_, const u128& low_) noexcept;
};

constexpr u256::u256(const u128& high_, const u128& low_) noexcept
{
    bytes[0] = low_.low;
    bytes[1] = low_.high;
    bytes[2] = high_.low;
    bytes[3] = high_.high;
}


} // namespace detail
} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_U256_HPP
