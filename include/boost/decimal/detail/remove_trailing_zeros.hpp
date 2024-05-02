// Copyright 2024 Junekey Jeon
// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// See: https://github.com/jk-jeon/rtz_benchmark/tree/master

#ifndef BOOST_DECIMAL_DETAIL_REMOVE_TRAILING_ZEROS_HPP
#define BOOST_DECIMAL_DETAIL_REMOVE_TRAILING_ZEROS_HPP

#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/emulated128.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <cstdint>
#endif

namespace boost {
namespace decimal {
namespace detail {

// n is assumed to be at most of bit_width bits.
template <std::size_t bit_width, typename UInt>
constexpr auto rotr(UInt n, unsigned int r) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_unsigned_v, UInt)
{
    r &= (bit_width - 1);
    return (n >> r) | (n << ((bit_width - r) & (bit_width - 1)));
}

template <typename T>
struct remove_trailing_zeros_return
{
    T trimmed_number;
    std::size_t number_of_removed_zeros;
};

constexpr auto remove_trailing_zeros(std::uint32_t n) noexcept -> remove_trailing_zeros_return<std::uint32_t>
{
    std::size_t s {};

    auto r = rotr<32>(n * UINT32_C(184254097), 4);
    auto b = r < UINT32_C(429497);
    s = s * 2U + static_cast<std::size_t>(b);
    n = b ? r : n;

    r = rotr<32>(n * UINT32_C(42949673), 2);
    b = r < UINT32_C(42949673);
    s = s * 2U + static_cast<std::size_t>(b);
    n = b ? r : n;

    r = rotr<32>(n * UINT32_C(1288490189), 1);
    b = r < UINT32_C(429496730);
    s = s * 2U + static_cast<std::size_t>(b);
    n = b ? r : n;

    return {n, s};
}

constexpr auto remove_trailing_zeros(std::uint64_t n) noexcept -> remove_trailing_zeros_return<std::uint64_t>
{
    std::size_t s {};

    auto r = rotr<64>(n * UINT64_C(28999941890838049), 8);
    auto b = r < UINT64_C(184467440738);
    s = s * 2U + static_cast<std::size_t>(b);
    n = b ? r : n;

    r = rotr<64>(n * UINT64_C(182622766329724561), 4);
    b = r < UINT64_C(1844674407370956);
    s = s * 2U + static_cast<std::size_t>(b);
    n = b ? r : n;

    r = rotr<64>(n * UINT64_C(10330176681277348905), 2);
    b = r < UINT64_C(184467440737095517);
    s = s * 2U + static_cast<std::size_t>(b);
    n = b ? r : n;

    r = rotr<64>(n * UINT32_C(14757395258967641293), 1);
    b = r < UINT64_C(1844674407370955162);
    s = s * 2U + static_cast<std::size_t>(b);
    n = b ? r : n;

    return {n, s};
}

// TODO(mborland): Make this better for the 2-word case
constexpr auto remove_trailing_zeros(uint128 n) noexcept -> remove_trailing_zeros_return<uint128>
{
    if (n.high == UINT64_C(0))
    {
        const auto temp {remove_trailing_zeros(n.low)};
        return {static_cast<uint128>(temp.trimmed_number), temp.number_of_removed_zeros};
    }

    std::size_t s {};

    while (n % 10 == 0)
    {
        n /= 10;
        ++s;
    }

    return {n, s};
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif //BOOST_DECIMAL_DETAIL_REMOVE_TRAILING_ZEROS_HPP
