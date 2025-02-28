// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// If the architecture (e.g. 32-bit x86) does not have __int128 we need to emulate it

#ifndef BOOST_DECIMAL_DETAIL_U128_HPP
#define BOOST_DECIMAL_DETAIL_U128_HPP

#include <boost/decimal/detail/config.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE

#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

#if !defined(BOOST_DECIMAL_DISABLE_IOSTREAM)
#include <iomanip>
#include <iosfwd>
#include <ostream>
#endif

#endif // BOOST_DECIMAL_BUILD_MODULE

namespace boost {
namespace decimal {
namespace detail {

struct
    #ifdef BOOST_DECIMAL_HAS_INT128
    alignas(alignof(unsigned __int128))
    #else
    alignas(16)
    #endif
u128
{
private:

    static constexpr std::uint64_t low_word_mask {~UINT64_C(0)};

public:

    #if BOOST_DECIMAL_ENDIAN_LITTLE_BYTE
    std::uint64_t low {};
    std::uint64_t high {};
    #else
    std::uint64_t high {};
    std::uint64_t low {};
    #endif

    // Constructors
    constexpr u128() noexcept = default;
    constexpr u128(const u128& other) noexcept = default;
    constexpr u128(u128&& other) noexcept = default;
    constexpr u128& operator=(const u128& other) noexcept = default;
    constexpr u128& operator=(u128&& other) noexcept = default;

    // Direct construction of the number
    constexpr u128(const std::uint64_t hi, const std::uint64_t lo) noexcept : low{lo}, high{hi} {}

    // Signed arithmetic constructors
    explicit constexpr u128(const std::int8_t value) noexcept : low {static_cast<std::uint64_t>(value)}, high {value < 0 ? UINT64_MAX : UINT64_C(0)} {}
    explicit constexpr u128(const std::int16_t value) noexcept : low {static_cast<std::uint64_t>(value)}, high {value < 0 ? UINT64_MAX : UINT64_C(0)} {}
    explicit constexpr u128(const std::int32_t value) noexcept : low {static_cast<std::uint64_t>(value)}, high {value < 0 ? UINT64_MAX : UINT64_C(0)} {}
    explicit constexpr u128(const std::int64_t value) noexcept : low {static_cast<std::uint64_t>(value)}, high {value < 0 ? UINT64_MAX : UINT64_C(0)} {}

    #ifdef BOOST_DECIMAL_HAS_INT128
    explicit constexpr u128(const __int128 value) noexcept :
        low {static_cast<std::uint64_t>(value & low_word_mask)},
        high {static_cast<std::uint64_t>(static_cast<unsigned __int128>(value) >> 64U)} {}
    #endif // BOOST_DECIMAL_HAS_INT128

    // Unsigned arithmetic constructors
    explicit constexpr u128(const std::uint8_t value) noexcept : low {static_cast<std::uint64_t>(value)}, high {UINT64_C(0)} {}
    explicit constexpr u128(const std::uint16_t value) noexcept : low {static_cast<std::uint64_t>(value)}, high {UINT64_C(0)} {}
    explicit constexpr u128(const std::uint32_t value) noexcept : low {static_cast<std::uint64_t>(value)}, high {UINT64_C(0)} {}
    explicit constexpr u128(const std::uint64_t value) noexcept : low {static_cast<std::uint64_t>(value)}, high {UINT64_C(0)} {}

    #ifdef BOOST_DECIMAL_HAS_INT128
    explicit constexpr u128(const unsigned __int128 value) noexcept :
        low {static_cast<std::uint64_t>(value & low_word_mask)},
        high {static_cast<std::uint64_t>(value >> 64U)} {}
    #endif // BOOST_DECIMAL_HAS_INT128

    // Signed assignment operators
    constexpr u128& operator=(std::int8_t value) noexcept;
    constexpr u128& operator=(std::int16_t value) noexcept;
    constexpr u128& operator=(std::int32_t value) noexcept;
    constexpr u128& operator=(std::int64_t value) noexcept;

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr u128& operator=(__int128 value) noexcept;
    #endif // BOOST_DECIMAL_HAS_INT128

    // Unsigned assignment operators
    constexpr u128& operator=(std::uint8_t value) noexcept;
    constexpr u128& operator=(std::uint16_t value) noexcept;
    constexpr u128& operator=(std::uint32_t value) noexcept;
    constexpr u128& operator=(std::uint64_t value) noexcept;

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr u128& operator=(unsigned __int128 value) noexcept;
    #endif

    // Bool conversion is not explicit so we can do stuff like if (num)
    constexpr operator bool() const noexcept { return low || high; }

    // Conversion to signed integer types
    explicit constexpr operator std::int8_t() const noexcept { return static_cast<std::int8_t>(low); }
    explicit constexpr operator std::int16_t() const noexcept { return static_cast<std::int16_t>(low); }
    explicit constexpr operator std::int32_t() const noexcept { return static_cast<std::int32_t>(low); }
    explicit constexpr operator std::int64_t() const noexcept { return static_cast<std::int64_t>(low); }

    #ifdef BOOST_DECIMAL_HAS_INT128
    explicit constexpr operator __int128() const noexcept { return (static_cast<__int128>(high) << 64) + low; }
    #endif // BOOST_DECIMAL_HAS_INT128

    // Conversion to unsigned integer types
    explicit constexpr operator std::uint8_t() const noexcept { return static_cast<std::uint8_t>(low); }
    explicit constexpr operator std::uint16_t() const noexcept { return static_cast<std::uint16_t>(low); }
    explicit constexpr operator std::uint32_t() const noexcept { return static_cast<std::uint32_t>(low); }
    explicit constexpr operator std::uint64_t() const noexcept { return low; }

    #ifdef BOOST_DECIMAL_HAS_INT128
    explicit constexpr operator unsigned __int128() const noexcept { return (static_cast<unsigned __int128>(high) << 64U) + low; }
    #endif // BOOST_DECIMAL_HAS_INT128

    // Conversion to float
    // This is basically the same as ldexp(static_cast<T>(high), 64) + static_cast<T>(low),
    // but can be constexpr at C++11 instead of C++26
    explicit constexpr operator float() const noexcept;
    explicit constexpr operator double() const noexcept;
    explicit constexpr operator long double() const noexcept;

    #ifdef BOOST_DECIMAL_HAS_FLOAT128
    explicit constexpr operator __float128() const noexcept;
    #endif // BOOST_DECIMAL_HAS_FLOAT128

    // Unary operators
    friend constexpr u128 operator+(u128 value) noexcept;
    friend constexpr u128 operator-(u128 value) noexcept;

    // Equality to signed integers
    friend constexpr bool operator==(u128 lhs, bool rhs) noexcept;
    friend constexpr bool operator==(u128 lhs, std::int8_t rhs) noexcept;
    friend constexpr bool operator==(u128 lhs, std::int16_t rhs) noexcept;
    friend constexpr bool operator==(u128 lhs, std::int32_t rhs) noexcept;
    friend constexpr bool operator==(u128 lhs, std::int64_t rhs) noexcept;

    friend constexpr bool operator==(bool lhs, u128 rhs) noexcept;
    friend constexpr bool operator==(std::int8_t lhs, u128 rhs) noexcept;
    friend constexpr bool operator==(std::int16_t lhs, u128 rhs) noexcept;
    friend constexpr bool operator==(std::int32_t lhs, u128 rhs) noexcept;
    friend constexpr bool operator==(std::int64_t lhs, u128 rhs) noexcept;

    // Equality to unsigned integers
    friend constexpr bool operator==(u128 lhs, std::uint8_t rhs) noexcept;
    friend constexpr bool operator==(u128 lhs, std::uint16_t rhs) noexcept;
    friend constexpr bool operator==(u128 lhs, std::uint32_t rhs) noexcept;
    friend constexpr bool operator==(u128 lhs, std::uint64_t rhs) noexcept;

    friend constexpr bool operator==(std::uint8_t lhs, u128 rhs) noexcept;
    friend constexpr bool operator==(std::uint16_t lhs, u128 rhs) noexcept;
    friend constexpr bool operator==(std::uint32_t lhs, u128 rhs) noexcept;
    friend constexpr bool operator==(std::uint64_t lhs, u128 rhs) noexcept;

    // 128-bit equality
    friend constexpr bool operator==(u128 lhs, u128 rhs) noexcept;

    #ifdef BOOST_DECIMAL_HAS_INT128

    friend constexpr bool operator==(u128 lhs, __int128 rhs) noexcept;
    friend constexpr bool operator==(__int128 lhs, u128 rhs) noexcept;

    friend constexpr bool operator==(u128 lhs, unsigned __int128 rhs) noexcept;
    friend constexpr bool operator==(unsigned __int128 lhs, u128 rhs) noexcept;

    #endif // BOOST_DECIMAL_HAS_INT128

    // Inequality to signed integers
    friend constexpr bool operator!=(u128 lhs, bool rhs) noexcept;
    friend constexpr bool operator!=(u128 lhs, std::int8_t rhs) noexcept;
    friend constexpr bool operator!=(u128 lhs, std::int16_t rhs) noexcept;
    friend constexpr bool operator!=(u128 lhs, std::int32_t rhs) noexcept;
    friend constexpr bool operator!=(u128 lhs, std::int64_t rhs) noexcept;

    friend constexpr bool operator!=(bool lhs, u128 rhs) noexcept;
    friend constexpr bool operator!=(std::int8_t lhs, u128 rhs) noexcept;
    friend constexpr bool operator!=(std::int16_t lhs, u128 rhs) noexcept;
    friend constexpr bool operator!=(std::int32_t lhs, u128 rhs) noexcept;
    friend constexpr bool operator!=(std::int64_t lhs, u128 rhs) noexcept;

    // Inequality to unsigned integers
    friend constexpr bool operator!=(u128 lhs, std::uint8_t rhs) noexcept;
    friend constexpr bool operator!=(u128 lhs, std::uint16_t rhs) noexcept;
    friend constexpr bool operator!=(u128 lhs, std::uint32_t rhs) noexcept;
    friend constexpr bool operator!=(u128 lhs, std::uint64_t rhs) noexcept;

    friend constexpr bool operator!=(std::uint8_t lhs, u128 rhs) noexcept;
    friend constexpr bool operator!=(std::uint16_t lhs, u128 rhs) noexcept;
    friend constexpr bool operator!=(std::uint32_t lhs, u128 rhs) noexcept;
    friend constexpr bool operator!=(std::uint64_t lhs, u128 rhs) noexcept;

    // 128-bit inequality
    friend constexpr bool operator!=(u128 lhs, u128 rhs) noexcept;

    #ifdef BOOST_DECIMAL_HAS_INT128

    friend constexpr bool operator!=(u128 lhs, __int128 rhs) noexcept;
    friend constexpr bool operator!=(__int128 lhs, u128 rhs) noexcept;

    friend constexpr bool operator!=(u128 lhs, unsigned __int128 rhs) noexcept;
    friend constexpr bool operator!=(unsigned __int128 lhs, u128 rhs) noexcept;

    #endif // BOOST_DECIMAL_HAS_INT128
};

// Signed assignment operators
constexpr u128& u128::operator=(const std::int8_t value) noexcept
{
    low = static_cast<std::uint64_t>(value);
    high = value < 0 ? UINT64_MAX : UINT64_C(0);
    return *this;
}

constexpr u128& u128::operator=(const std::int16_t value) noexcept
{
    low = static_cast<std::uint64_t>(value);
    high = value < 0 ? UINT64_MAX : UINT64_C(0);
    return *this;
}

constexpr u128& u128::operator=(const std::int32_t value) noexcept
{
    low = static_cast<std::uint64_t>(value);
    high = value < 0 ? UINT64_MAX : UINT64_C(0);
    return *this;
}

constexpr u128& u128::operator=(const std::int64_t value) noexcept
{
    low = static_cast<std::uint64_t>(value);
    high = value < 0 ? UINT64_MAX : UINT64_C(0);
    return *this;
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr u128& u128::operator=(const __int128 value) noexcept
{
    low = static_cast<std::uint64_t>(value & low_word_mask); high = static_cast<std::uint64_t>(static_cast<unsigned __int128>(value) >> 64U); return *this;
}

#endif // BOOST_DECIMAL_HAS_INT128

// Unsigned assignment operators
constexpr u128& u128::operator=(const std::uint8_t value) noexcept { low = static_cast<std::uint64_t>(value); high = UINT64_C(0); return *this; }
constexpr u128& u128::operator=(const std::uint16_t value) noexcept { low = static_cast<std::uint64_t>(value); high = UINT64_C(0); return *this; }
constexpr u128& u128::operator=(const std::uint32_t value) noexcept { low = static_cast<std::uint64_t>(value); high = UINT64_C(0); return *this; }
constexpr u128& u128::operator=(const std::uint64_t value) noexcept { low = static_cast<std::uint64_t>(value); high = UINT64_C(0); return *this; }

#ifdef BOOST_DECIMAL_HAS_INT128
constexpr u128& u128::operator=(const unsigned __int128 value) noexcept { low = static_cast<std::uint64_t>(value & low_word_mask); high = static_cast<std::uint64_t>(value >> 64U); return *this; }
#endif

constexpr u128::operator float() const noexcept
{
    constexpr float offset {static_cast<float>(std::numeric_limits<std::uint64_t>::max())};
    return static_cast<float>(high) * offset + static_cast<float>(low);
}

constexpr u128::operator double() const noexcept
{
    constexpr double offset {static_cast<double>(std::numeric_limits<std::uint64_t>::max())};
    return static_cast<double>(high) * offset + static_cast<double>(low);
}

constexpr u128::operator long double() const noexcept
{
    constexpr long double offset {static_cast<long double>(std::numeric_limits<std::uint64_t>::max())};
    return static_cast<long double>(high) * offset + static_cast<long double>(low);
}

#ifdef BOOST_DECIMAL_HAS_FLOAT128

constexpr u128::operator __float128() const noexcept
{
    constexpr __float128 offset {static_cast<__float128>(std::numeric_limits<std::uint64_t>::max())};
    return static_cast<__float128>(high) * offset + static_cast<__float128>(low);
}

#endif // BOOST_DECIMAL_HAS_FLOAT128

constexpr u128 operator+(const u128 value) noexcept
{
    return value;
}

constexpr u128 operator-(const u128 value) noexcept
{
    return u128{~value.high + static_cast<std::uint64_t>(value.low == UINT64_C(0)), ~value.low + UINT64_C(1)};
}

constexpr bool operator==(const u128 lhs, const bool rhs) noexcept
{
    return lhs.high == UINT64_C(0) && lhs.low == static_cast<std::uint64_t>(rhs);
}

constexpr bool operator==(const u128 lhs, const std::int8_t rhs) noexcept
{
    return rhs >= 0 && lhs.high == UINT64_C(0) && lhs.low == static_cast<std::uint64_t>(rhs);
}

constexpr bool operator==(const u128 lhs, const std::int16_t rhs) noexcept
{
    return rhs >= 0 && lhs.high == UINT64_C(0) && lhs.low == static_cast<std::uint64_t>(rhs);
}

constexpr bool operator==(const u128 lhs, const std::int32_t rhs) noexcept
{
    return rhs >= 0 && lhs.high == UINT64_C(0) && lhs.low == static_cast<std::uint64_t>(rhs);
}

constexpr bool operator==(const u128 lhs, const std::int64_t rhs) noexcept
{
    return rhs >= 0 && lhs.high == UINT64_C(0) && lhs.low == static_cast<std::uint64_t>(rhs);
}

constexpr bool operator==(const bool lhs, const u128 rhs) noexcept
{
    return rhs.high == UINT64_C(0) && rhs.low == static_cast<std::uint64_t>(lhs);
}

constexpr bool operator==(const std::int8_t lhs, const u128 rhs) noexcept
{
    return lhs >= 0 && rhs.high == UINT64_C(0) && rhs.low == static_cast<std::uint64_t>(lhs);
}

constexpr bool operator==(const std::int16_t lhs, const u128 rhs) noexcept
{
    return lhs >= 0 && rhs.high == UINT64_C(0) && rhs.low == static_cast<std::uint64_t>(lhs);
}

constexpr bool operator==(const std::int32_t lhs, const u128 rhs) noexcept
{
    return lhs >= 0 && rhs.high == UINT64_C(0) && rhs.low == static_cast<std::uint64_t>(lhs);
}

constexpr bool operator==(const std::int64_t lhs, const u128 rhs) noexcept
{
    return lhs >= 0 && rhs.high == UINT64_C(0) && rhs.low == static_cast<std::uint64_t>(lhs);
}

constexpr bool operator==(const u128 lhs, const std::uint8_t rhs) noexcept
{
    return lhs.high == UINT64_C(0) && lhs.low == static_cast<std::uint64_t>(rhs);
}

constexpr bool operator==(const u128 lhs, const std::uint16_t rhs) noexcept
{
    return lhs.high == UINT64_C(0) && lhs.low == static_cast<std::uint64_t>(rhs);
}

constexpr bool operator==(const u128 lhs, const std::uint32_t rhs) noexcept
{
    return lhs.high == UINT64_C(0) && lhs.low == static_cast<std::uint64_t>(rhs);
}

constexpr bool operator==(const u128 lhs, const std::uint64_t rhs) noexcept
{
    return lhs.high == UINT64_C(0) && lhs.low == static_cast<std::uint64_t>(rhs);
}

constexpr bool operator==(const std::uint8_t lhs, const u128 rhs) noexcept
{
    return rhs.high == UINT64_C(0) && rhs.low == static_cast<std::uint64_t>(lhs);
}

constexpr bool operator==(const std::uint16_t lhs, const u128 rhs) noexcept
{
    return rhs.high == UINT64_C(0) && rhs.low == static_cast<std::uint64_t>(lhs);
}

constexpr bool operator==(const std::uint32_t lhs, const u128 rhs) noexcept
{
    return rhs.high == UINT64_C(0) && rhs.low == static_cast<std::uint64_t>(lhs);
}

constexpr bool operator==(const std::uint64_t lhs, const u128 rhs) noexcept
{
    return rhs.high == UINT64_C(0) && rhs.low == static_cast<std::uint64_t>(lhs);
}

constexpr bool operator==(const u128 lhs, const u128 rhs) noexcept
{
    return lhs.low == rhs.low && lhs.high == rhs.high;
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr bool operator==(const u128 lhs, const __int128 rhs) noexcept
{
    return lhs == static_cast<u128>(rhs);
}

constexpr bool operator==(const __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) == rhs;
}

constexpr bool operator==(const u128 lhs, const unsigned __int128 rhs) noexcept
{
    return lhs == static_cast<u128>(rhs);
}

constexpr bool operator==(const unsigned __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) == rhs;
}

#endif

constexpr bool operator!=(const u128 lhs, const bool rhs) noexcept
{
    return lhs.high != UINT64_C(0) || lhs.low != static_cast<std::uint64_t>(rhs);
}

constexpr bool operator!=(const u128 lhs, const std::int8_t rhs) noexcept
{
    return rhs < 0 || lhs.high != UINT64_C(0) || lhs.low != static_cast<std::uint64_t>(rhs);
}

constexpr bool operator!=(const u128 lhs, const std::int16_t rhs) noexcept
{
    return rhs < 0 || lhs.high != UINT64_C(0) || lhs.low != static_cast<std::uint64_t>(rhs);
}

constexpr bool operator!=(const u128 lhs, const std::int32_t rhs) noexcept
{
    return rhs < 0 || lhs.high != UINT64_C(0) || lhs.low != static_cast<std::uint64_t>(rhs);
}

constexpr bool operator!=(const u128 lhs, const std::int64_t rhs) noexcept
{
    return rhs < 0 || lhs.high != UINT64_C(0) || lhs.low != static_cast<std::uint64_t>(rhs);
}

constexpr bool operator!=(const u128 lhs, const std::uint8_t rhs) noexcept
{
    return lhs.high != UINT64_C(0) || lhs.low != static_cast<std::uint64_t>(rhs);
}

constexpr bool operator!=(const u128 lhs, const std::uint16_t rhs) noexcept
{
    return lhs.high != UINT64_C(0) || lhs.low != static_cast<std::uint64_t>(rhs);
}

constexpr bool operator!=(const u128 lhs, const std::uint32_t rhs) noexcept
{
    return lhs.high != UINT64_C(0) || lhs.low != static_cast<std::uint64_t>(rhs);
}

constexpr bool operator!=(const u128 lhs, const std::uint64_t rhs) noexcept
{
    return lhs.high != UINT64_C(0) || lhs.low != static_cast<std::uint64_t>(rhs);
}

constexpr bool operator!=(const bool lhs, const u128 rhs) noexcept
{
    return rhs.high != UINT64_C(0) || rhs.low != static_cast<std::uint64_t>(lhs);
}

constexpr bool operator!=(const std::int8_t lhs, const u128 rhs) noexcept
{
    return lhs < 0 || rhs.high != UINT64_C(0) || rhs.low != static_cast<std::uint64_t>(lhs);
}

constexpr bool operator!=(const std::int16_t lhs, const u128 rhs) noexcept
{
    return lhs < 0 || rhs.high != UINT64_C(0) || rhs.low != static_cast<std::uint64_t>(lhs);
}

constexpr bool operator!=(const std::int32_t lhs, const u128 rhs) noexcept
{
    return lhs < 0 || rhs.high != UINT64_C(0) || rhs.low != static_cast<std::uint64_t>(lhs);
}

constexpr bool operator!=(const std::int64_t lhs, const u128 rhs) noexcept
{
    return lhs < 0 || rhs.high != UINT64_C(0) || rhs.low != static_cast<std::uint64_t>(lhs);
}

constexpr bool operator!=(const std::uint8_t lhs, const u128 rhs) noexcept
{
    return rhs.high != UINT64_C(0) || rhs.low != static_cast<std::uint64_t>(lhs);
}

constexpr bool operator!=(const std::uint16_t lhs, const u128 rhs) noexcept
{
    return rhs.high != UINT64_C(0) || rhs.low != static_cast<std::uint64_t>(lhs);
}

constexpr bool operator!=(const std::uint32_t lhs, const u128 rhs) noexcept
{
    return rhs.high != UINT64_C(0) || rhs.low != static_cast<std::uint64_t>(lhs);
}

constexpr bool operator!=(const std::uint64_t lhs, const u128 rhs) noexcept
{
    return rhs.high != UINT64_C(0) || rhs.low != static_cast<std::uint64_t>(lhs);
}

constexpr bool operator!=(const u128 lhs, const u128 rhs) noexcept
{
    return lhs.low != rhs.low || lhs.high != rhs.high;
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr bool operator!=(const u128 lhs, const __int128 rhs) noexcept
{
    return lhs != static_cast<u128>(rhs);
}

constexpr bool operator!=(const __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) != rhs;
}

constexpr bool operator!=(const u128 lhs, const unsigned __int128 rhs) noexcept
{
    return lhs != static_cast<u128>(rhs);
}

constexpr bool operator!=(const unsigned __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) != rhs;
}

#endif // BOOST_DECIMAL_HAS_INT128

} // namespace detail
} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_U128_HPP
