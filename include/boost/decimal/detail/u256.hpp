// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_U256_HPP
#define BOOST_DECIMAL_DETAIL_U256_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/wide-integer/uintwide_t.hpp>
#include <boost/decimal/detail/u128.hpp>
#include <boost/decimal/detail/memcpy.hpp>
#include <cstdint>
#include <cstring>

namespace boost {
namespace decimal {
namespace detail {

struct alignas(alignof(std::uint64_t) * 4)
u256
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

    explicit constexpr operator u128() const noexcept;
    explicit constexpr operator std::size_t() const noexcept;
};

constexpr u256::u256(const u128& high_, const u128& low_) noexcept
{
    bytes[0] = low_.low;
    bytes[1] = low_.high;
    bytes[2] = high_.low;
    bytes[3] = high_.high;
}

//=====================================
// Integer Conversion Operators
//=====================================

constexpr u256::operator u128() const noexcept
{
    return u128{bytes[1], bytes[0]};
}

constexpr u256::operator std::size_t() const noexcept
{
    return static_cast<std::size_t>(bytes[3]);
}

//=====================================
// Equality Operators
//=====================================

namespace impl {

#if !defined(BOOST_DECIMAL_NO_CONSTEVAL_DETECTION)

constexpr bool memcmp_modified(const std::uint64_t* lhs, const std::uint64_t* rhs)
{
    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
    {
        for (std::size_t i = 0; i < 4U; ++i)
        {
            if (lhs[i] != rhs[i])
            {
                return false;
            }
        }

        return true;
    }
    else
    {
        // This will always be the comparison of 4x std::uint64_ts
        return !static_cast<bool>(memcmp(lhs, rhs, 32));
    }
}

#else

constexpr bool memcmp_modified(const std::uint64_t* lhs, const std::uint64_t* rhs)
{
    for (std::size_t i {}; i < 4U; ++i)
    {
        if (lhs[i] != rhs[i])
        {
            return false;
        }
    }

    return true;
}

#endif

} // namespace impl

constexpr bool operator==(const u256& lhs, const u256& rhs) noexcept
{
    return impl::memcmp_modified(lhs.bytes, rhs.bytes);
constexpr bool operator!=(const u256& lhs, const u256& rhs) noexcept
{
    return lhs.bytes[0] != rhs.bytes[0] || lhs.bytes[1] != rhs.bytes[1] || lhs.bytes[2] != rhs.bytes[2] || lhs.bytes[3] != rhs.bytes[3];
}

//=====================================
// Left Shift Operators
//=====================================

constexpr u256 operator<<(const u256& lhs, int shift) noexcept
{
    u256 result {};

    if (shift >= 256)
    {
        return result;
    }

    const auto word_shift {shift / 64};
    const auto bit_shift {shift % 64};

    // Only moving whole words
    if (bit_shift == 0)
    {
        for (auto i = word_shift; i < 4; ++i)
        {
            result.bytes[i] = lhs.bytes[i - word_shift];
        }

        return result;
    }

    if (word_shift < 4)
    {
        result.bytes[word_shift] = lhs.bytes[0] << bit_shift;
    }

    for (auto i = word_shift + 1; i < 4; ++i)
    {
        result.bytes[i] = (lhs.bytes[i - word_shift] << bit_shift) |
                          (lhs.bytes[i - word_shift - 1] >> (64 - bit_shift));
    }

    return result;
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_U256_HPP
