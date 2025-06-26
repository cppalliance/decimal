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

    // Compound operators
    constexpr u256& operator<<=(int amount) noexcept;
    constexpr u256& operator>>=(int amount) noexcept;
};

constexpr u256::u256(const std::uint64_t byte3, const std::uint64_t byte2, const std::uint64_t byte1, const std::uint64_t byte0) noexcept
{
    bytes[0] = byte0;
    bytes[1] = byte1;
    bytes[2] = byte2;
    bytes[3] = byte3;
}

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

//=====================================
// Equality Operators
//=====================================

namespace impl {

BOOST_DECIMAL_FORCE_INLINE constexpr bool basic_equality_impl(const u256& lhs, const u256& rhs) noexcept
{
    return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2] && lhs[3] == rhs[3];
}

BOOST_DECIMAL_FORCE_INLINE constexpr bool basic_inequality_impl(const u256& lhs, const u256& rhs) noexcept
{
    return lhs[0] != rhs[0] || lhs[1] != rhs[1] || lhs[2] != rhs[2] || lhs[3] != rhs[3];
}

} // namespace impl

#if !defined(BOOST_DECIMAL_NO_CONSTEVAL_DETECTION) && defined(__AVX2__)

constexpr bool operator==(const u256& lhs, const u256& rhs) noexcept
{
    // Start comp from low word since they will most likely be filled
    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
    {
        return impl::basic_equality_impl(lhs, rhs);
    }
    else
    {
        __m256i a = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(lhs.bytes));
        __m256i b = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(rhs.bytes));

        __m256i cmp = _mm256_cmpeq_epi64(a, b);

        const int mask = _mm256_movemask_epi8(cmp);

        return mask == -1;
    }
}

#else

constexpr bool operator==(const u256& lhs, const u256& rhs) noexcept
{
    return impl::basic_equality_impl(lhs, rhs);
}

#endif

//=====================================
// Inequality Operators
//=====================================

#if !defined(BOOST_DECIMAL_NO_CONSTEVAL_DETECTION) && defined(__AVX2__)

constexpr bool operator!=(const u256& lhs, const u256& rhs) noexcept
{
    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
    {
        return impl::basic_inequality_impl(lhs, rhs);
    }
    else
    {
        __m256i a = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(lhs.bytes));
        __m256i b = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(rhs.bytes));

        __m256i cmp = _mm256_cmpeq_epi64(a, b);

        const int mask = _mm256_movemask_epi8(cmp);

        return mask != -1;
    }
}

#else

constexpr bool operator!=(const u256& lhs, const u256& rhs) noexcept
{
    return impl::basic_inequality_impl(lhs, rhs);
}

#endif // !defined(BOOST_DECIMAL_NO_CONSTEVAL_DETECTION) && defined(__AVX2__)

//=====================================
// Less Than Operator
//=====================================

namespace impl {

BOOST_DECIMAL_FORCE_INLINE constexpr bool basic_lt_impl(const u256& lhs, const u256& rhs) noexcept
{
    for (std::size_t i {}; i < 4U; ++i)
    {
        if (lhs[i] != rhs[i])
        {
            return lhs[i] < rhs[i];
        }
    }

    return false;
}

} // namespace impl

#if !defined(BOOST_DECIMAL_NO_CONSTEVAL_DETECTION) && defined(__AVX2__)

constexpr bool operator<(const u256& lhs, const u256& rhs) noexcept
{
    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
    {
        return impl::basic_lt_impl(lhs, rhs);
    }
    else
    {
        __m256i lhs_vec = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&lhs));
        __m256i rhs_vec = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&rhs));

        __m256i eq_mask = _mm256_cmpeq_epi64(lhs_vec, rhs_vec);
        uint32_t eq_bits = _mm256_movemask_pd(_mm256_castsi256_pd(eq_mask));

        if ((eq_bits & 0x8) == 0)
        {
            return lhs[3] < rhs[3];
        }
        else if ((eq_bits & 0x4) == 0)
        {
            return lhs[2] < rhs[2];
        }
        else if ((eq_bits & 0x2) == 0)
        {
            return lhs[1] < rhs[1];
        }
        else if ((eq_bits & 0x1) == 0)
        {
            return lhs[0] < rhs[0];
        }

        return false;
    }
}

#else

constexpr bool operator<(const u256& lhs, const u256& rhs) noexcept
{
    return impl::basic_lt_impl(lhs, rhs);
}

#endif

//=====================================
// Less Equal Operator
//=====================================

namespace impl {

BOOST_DECIMAL_FORCE_INLINE constexpr bool basic_le_impl(const u256& lhs, const u256& rhs) noexcept
{
    return !(rhs < lhs);
}

} // namespace impl

#if !defined(BOOST_DECIMAL_NO_CONSTEVAL_DETECTION) && defined(__AVX2__)

constexpr bool operator<=(const u256& lhs, const u256& rhs) noexcept
{
    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
    {
        return impl::basic_le_impl(lhs, rhs);
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
    return impl::basic_le_impl(lhs, rhs);
}

#endif

//=====================================
// Greater Than Operator
//=====================================

constexpr bool operator>(const u256& lhs, const u256& rhs) noexcept
{
    return rhs < lhs;
}

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

constexpr u256 operator<<(const u256& lhs, const int shift) noexcept
{
    u256 result {};

    if (shift >= 256 || shift < 0)
    {
        return result;
    }

    const auto word_shift {shift / 64};
    const auto bit_shift {shift % 64};

    // Only moving whole words
    if (bit_shift == 0)
    {
        for (auto i {word_shift}; i < 4; ++i)
        {
            result[i] = lhs[i - word_shift];
        }

        return result;
    }

    if (word_shift < 4)
    {
        result[word_shift] = lhs[0] << bit_shift;
    }

    for (auto i {word_shift + 1}; i < 4; ++i)
    {
        result[i] = (lhs[i - word_shift] << bit_shift) |
                    (lhs[i - word_shift - 1] >> (64 - bit_shift));
    }

    return result;
}

constexpr u256& u256::operator<<=(const int amount) noexcept
{
    *this = *this << amount;
    return *this;
}

//=====================================
// Right Shift Operators
//=====================================

constexpr u256 operator>>(const u256& lhs, const int shift) noexcept
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
        for (auto i {}; i < 4 - word_shift; ++i)
        {
            result[i] = lhs[i + word_shift];
        }

        return result;
    }

    // Handle partial shifts across word boundaries
    for (auto i {}; i < 4 - word_shift - 1; ++i)
    {
        result[i] = (lhs[i + word_shift] >> bit_shift) |
                    (lhs[i + word_shift + 1] << (64 - bit_shift));
    }

    // Handle the last word that has a partial shift
    if (word_shift < 4)
    {
        result[3 - word_shift] = lhs[3] >> bit_shift;
    }

    return result;
}

constexpr u256& u256::operator>>=(const int amount) noexcept
{
    *this = *this >> amount;
    return *this;
}

//=====================================
// Or Operators
//=====================================

namespace impl {

BOOST_DECIMAL_FORCE_INLINE constexpr u256 basic_or_impl(const u256& lhs, const u256& rhs) noexcept
{
    u256 result;

    result[3] = lhs[3] | rhs[3];
    result[2] = lhs[2] | rhs[2];
    result[1] = lhs[1] | rhs[1];
    result[0] = lhs[0] | rhs[0];

    return result;
}

} // namespace impl

#if !defined(BOOST_DECIMAL_NO_CONSTEVAL_DETECTION) && defined(__AVX2__)

constexpr u256 operator|(const u256& lhs, const u256& rhs) noexcept
{
    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
    {
        return impl::basic_or_impl(lhs, rhs);
    }
    else
    {
        u256 result;

        // Load 256 bits from each operand into AVX2 registers
        __m256i lhs_vec = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&lhs));
        __m256i rhs_vec = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&rhs));

        // Perform the bitwise OR operation
        __m256i result_vec = _mm256_or_si256(lhs_vec, rhs_vec);

        // Store the result back to memory
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(&result), result_vec);

        return result;
    }
}

#elif !defined(BOOST_DECIMAL_NO_CONSTEVAL_DETECTION) && defined(BOOST_DECIMAL_HAS_ARM_INTRINSICS)

constexpr u256 operator|(const u256& lhs, const u256& rhs) noexcept
{
    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
    {
        return impl::basic_or_impl(lhs, rhs);
    }
    else
    {
        u256 result;

        uint64x2_t lhs_low = vld1q_u64(&lhs.bytes[0]);
        uint64x2_t lhs_high = vld1q_u64(&lhs.bytes[2]);

        uint64x2_t rhs_low = vld1q_u64(&rhs.bytes[0]);
        uint64x2_t rhs_high = vld1q_u64(&rhs.bytes[2]);

        // Perform bitwise OR in parallel
        uint64x2_t result_low = vorrq_u64(lhs_low, rhs_low);
        uint64x2_t result_high = vorrq_u64(lhs_high, rhs_high);

        // Store results back
        vst1q_u64(&result[0], result_low);
        vst1q_u64(&result[2], result_high);

        return result;
    }
}

#else

constexpr u256 operator|(const u256& lhs, const u256& rhs) noexcept
{
    return impl::basic_or_impl(lhs, rhs);
}

#endif


} // namespace detail
} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_U256_HPP
