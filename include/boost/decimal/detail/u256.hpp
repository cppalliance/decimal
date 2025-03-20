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

    constexpr u256(std::uint64_t byte3, std::uint64_t byte2, std::uint64_t byte1, std::uint64_t byte0) noexcept;

    // Construction from uints
    constexpr u256(const u128& high_, const u128& low_) noexcept;

    explicit constexpr operator u128() const noexcept;
    explicit constexpr operator std::size_t() const noexcept;

    // Subscript Operators
    constexpr std::uint64_t& operator[](const std::size_t index) noexcept
    {
        BOOST_DECIMAL_ASSERT(index < 4);
        return bytes[index];
    }

    BOOST_DECIMAL_FORCE_INLINE constexpr std::uint64_t operator[](const std::size_t index) const noexcept
    {
        BOOST_DECIMAL_ASSERT(index < 4);
        return bytes[index];
    }
};

constexpr u256::u256(const u128& high_, const u128& low_) noexcept
{
    bytes[0] = low_.low;
    bytes[1] = low_.high;
    bytes[2] = high_.low;
    bytes[3] = high_.high;
}

constexpr u256::u256(std::uint64_t byte3, std::uint64_t byte2, std::uint64_t byte1, std::uint64_t byte0) noexcept
{
    bytes[0] = byte0;
    bytes[1] = byte1;
    bytes[2] = byte2;
    bytes[3] = byte3;
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
        return static_cast<bool>(memcmp(lhs, rhs, 32));
    }
}

#else

constexpr bool memcmp_modified(const std::uint64_t* lhs, const std::uint64_t* rhs)
{
    for (std::size_t i {}; i < 4U; ++i)
    {
        if (lhs[i] != rhs[i])
        {
            return true;
        }
    }

    return false;
}

#endif

} // namespace impl

#if !defined(BOOST_DECIMAL_NO_CONSTEVAL_DETECTION) && defined(__AVX2__)

constexpr bool operator==(const u256& lhs, const u256& rhs) noexcept
{
    // Start comp from low word since they will most likely be filled
    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
    {
        return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2] && lhs[3] == rhs[3];
    }
    else
    {
        __m256i a = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(lhs.bytes));
        __m256i b = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(rhs.bytes));

        // Compare for equality (0xFFFFFFFF... where elements are equal)
        __m256i cmp = _mm256_cmpeq_epi64(a, b);

        // Get comparison mask (16 bits, one per byte)
        int mask = _mm256_movemask_epi8(cmp);

        // If mask isn't all 1s, at least one element differs
        return mask == -1; // -1 is 0xFFFFFFFF in two's complement
    }
}

constexpr bool operator!=(const u256& lhs, const u256& rhs) noexcept
{
    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
    {
        return lhs[0] != rhs[0] || lhs[1] != rhs[1] || lhs[2] != rhs[2] || lhs[3] != rhs[3];
    }
    else
    {
        __m256i a = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(lhs.bytes));
        __m256i b = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(rhs.bytes));

        // Compare for equality (0xFFFFFFFF... where elements are equal)
        __m256i cmp = _mm256_cmpeq_epi64(a, b);

        // Get comparison mask (16 bits, one per byte)
        int mask = _mm256_movemask_epi8(cmp);

        // If mask isn't all 1s, at least one element differs
        return mask != -1; // -1 is 0xFFFFFFFF in two's complement
    }
}

#else

constexpr bool operator==(const u256& lhs, const u256& rhs) noexcept
{
    return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2] && lhs[3] == rhs[3];
}

constexpr bool operator!=(const u256& lhs, const u256& rhs) noexcept
{
    return lhs[0] != rhs[0] || lhs[1] != rhs[1] || lhs[2] != rhs[2] || lhs[3] != rhs[3];
}

#endif

//=====================================
// Less Than Operator
//=====================================

constexpr bool operator<(const u256& lhs, const u256& rhs) noexcept
{
    for (int i = 3; i >= 0; --i)
    {
        if (lhs[i] != rhs[i])
        {
            return lhs[i] < rhs[i];
        }
    }

    return false;
}

//=====================================
// Less Equal Operator
//=====================================

#if !defined(BOOST_DECIMAL_NO_CONSTEVAL_DETECTION) && defined(__AVX2__)

constexpr bool operator<=(const u256& lhs, const u256& rhs) noexcept
{
    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
    {
        return !(rhs < lhs);
    }
    else
    {
        __m256i lhs_vec = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&lhs));
        __m256i rhs_vec = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&rhs));

        // Compare all elements for equality
        __m256i eq_mask = _mm256_cmpeq_epi64(lhs_vec, rhs_vec);
        uint32_t eq_bits = _mm256_movemask_pd(_mm256_castsi256_pd(eq_mask));

        // Check each position from most significant to least significant
        if ((eq_bits & 0x8) == 0)
        {
            return (rhs[3] > lhs[3]);
        }
        else if ((eq_bits & 0x4) == 0)
        {
            return (rhs[2] > lhs[2]);
        }
        else if ((eq_bits & 0x2) == 0)
        {
            return (rhs[1] > lhs[1]);
        }
        else if ((eq_bits & 0x1) == 0)
        {
            return (rhs[0] > lhs[0]);
        }
        else
        {
            return true;
        }
    }
}

#else

constexpr bool operator<=(const u256& lhs, const u256& rhs) noexcept
{
    for (int i = 3; i >= 0; --i)
    {
        if (lhs[i] != rhs[i])
        {
            return lhs[i] < rhs[i];
        }
    }

    return true;
}

#endif

//=====================================
// Greater Than Operator
//=====================================

#if !defined(BOOST_DECIMAL_NO_CONSTEVAL_DETECTION) && defined(__AVX2__)

constexpr bool operator>(const u256& lhs, const u256& rhs) noexcept
{
    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
    {
        return rhs < lhs;
    }
    else
    {
        __m256i lhs_vec = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&lhs));
        __m256i rhs_vec = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&rhs));

        __m256i eq_mask = _mm256_cmpeq_epi64(lhs_vec, rhs_vec);
        uint32_t eq_bits = _mm256_movemask_pd(_mm256_castsi256_pd(eq_mask));

        if ((eq_bits & 0x8) == 0)
        {
            return (rhs[3] < lhs[3]);
        }
        else if ((eq_bits & 0x4) == 0)
        {
            return (rhs[2] < lhs[2]);
        }
        else if ((eq_bits & 0x2) == 0)
        {
            return (rhs[1] < lhs[1]);
        }
        else if ((eq_bits & 0x1) == 0)
        {
            return (rhs[0] < lhs[0]);
        }

        return false;
    }
}

#else

constexpr bool operator>(const u256& lhs, const u256& rhs) noexcept
{
    for (int i = 3; i >= 0; --i)
    {
        if (lhs[i] != rhs[i])
        {
            return rhs[i] < lhs[i];
        }
    }

    return false;
}

#endif

//=====================================
// Greater Equal Operator
//=====================================

constexpr bool operator>=(const u256& lhs, const u256& rhs) noexcept
{
    return !(lhs < rhs);
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
