// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// If the architecture (e.g. 32-bit x86) does not have __int128 we need to emulate it

#ifndef BOOST_DECIMAL_DETAIL_U128_HPP
#define BOOST_DECIMAL_DETAIL_U128_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/apply_sign.hpp>

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

    // Compound operators
    constexpr u128& operator+=(std::uint8_t rhs) noexcept;
    constexpr u128& operator+=(std::uint16_t rhs) noexcept;
    constexpr u128& operator+=(std::uint32_t rhs) noexcept;
    constexpr u128& operator+=(std::uint64_t rhs) noexcept;

    constexpr u128& operator+=(std::int8_t rhs) noexcept;
    constexpr u128& operator+=(std::int16_t rhs) noexcept;
    constexpr u128& operator+=(std::int32_t rhs) noexcept;
    constexpr u128& operator+=(std::int64_t rhs) noexcept;

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr u128& operator+=(__int128 rhs) noexcept;
    constexpr u128& operator+=(unsigned __int128 rhs) noexcept;
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

//=====================================
// Unary Operators
//=====================================

constexpr u128 operator+(const u128 value) noexcept
{
    return value;
}

constexpr u128 operator-(const u128 value) noexcept
{
    return u128{~value.high + static_cast<std::uint64_t>(value.low == UINT64_C(0)), ~value.low + UINT64_C(1)};
}

//=====================================
// Equality Operators
//=====================================

constexpr bool operator==(const u128 lhs, const bool rhs) noexcept
{
    return lhs.high == UINT64_C(0) && lhs.low == static_cast<std::uint64_t>(rhs);
}

constexpr bool operator==(const bool lhs, const u128 rhs) noexcept
{
    return rhs.high == UINT64_C(0) && rhs.low == static_cast<std::uint64_t>(lhs);
}

template <typename SignedInteger, std::enable_if_t<std::is_integral<SignedInteger>::value && std::is_signed<SignedInteger>::value, bool> = true>
constexpr bool operator==(const u128 lhs, const SignedInteger rhs) noexcept
{
    return rhs >= 0 && lhs.high == UINT64_C(0) && lhs.low == static_cast<std::uint64_t>(rhs);
}

template <typename SignedInteger, std::enable_if_t<std::is_integral<SignedInteger>::value && std::is_signed<SignedInteger>::value, bool> = true>
constexpr bool operator==(const SignedInteger lhs, const u128 rhs) noexcept
{
    return lhs >= 0 && rhs.high == UINT64_C(0) && rhs.low == static_cast<std::uint64_t>(lhs);
}

template <typename UnsignedInteger, std::enable_if_t<std::is_integral<UnsignedInteger>::value && std::is_unsigned<UnsignedInteger>::value, bool> = true>
constexpr bool operator==(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return lhs.high == UINT64_C(0) && lhs.low == static_cast<std::uint64_t>(rhs);
}

template <typename UnsignedInteger, std::enable_if_t<std::is_integral<UnsignedInteger>::value && std::is_unsigned<UnsignedInteger>::value, bool> = true>
constexpr bool operator==(const UnsignedInteger lhs, const u128 rhs) noexcept
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

//=====================================
// Inequality Operators
//=====================================

constexpr bool operator!=(const u128 lhs, const bool rhs) noexcept
{
    return lhs.high != UINT64_C(0) || lhs.low != static_cast<std::uint64_t>(rhs);
}

constexpr bool operator!=(const bool lhs, const u128 rhs) noexcept
{
    return rhs.high != UINT64_C(0) || rhs.low != static_cast<std::uint64_t>(lhs);
}

template <typename SignedInteger, std::enable_if_t<std::is_integral<SignedInteger>::value && std::is_signed<SignedInteger>::value, bool> = true>
constexpr bool operator!=(const u128 lhs, const SignedInteger rhs) noexcept
{
    return rhs < 0 || lhs.high != UINT64_C(0) || lhs.low != static_cast<std::uint64_t>(rhs);
}

template <typename SignedInteger, std::enable_if_t<std::is_integral<SignedInteger>::value && std::is_signed<SignedInteger>::value, bool> = true>
constexpr bool operator!=(const SignedInteger lhs, const u128 rhs) noexcept
{
    return lhs < 0 || rhs.high != UINT64_C(0) || rhs.low != static_cast<std::uint64_t>(lhs);
}

template <typename UnsignedInteger, std::enable_if_t<std::is_integral<UnsignedInteger>::value && std::is_unsigned<UnsignedInteger>::value, bool> = true>
constexpr bool operator!=(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return lhs.high != UINT64_C(0) || lhs.low != static_cast<std::uint64_t>(rhs);
}

template <typename UnsignedInteger, std::enable_if_t<std::is_integral<UnsignedInteger>::value && std::is_unsigned<UnsignedInteger>::value, bool> = true>
constexpr bool operator!=(const UnsignedInteger lhs, const u128 rhs) noexcept
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

//=====================================
// Less than Operators
//=====================================

template <typename SignedInteger, std::enable_if_t<std::is_integral<SignedInteger>::value && std::is_signed<SignedInteger>::value, bool> = true>
constexpr bool operator<(const u128 lhs, const SignedInteger rhs) noexcept
{
    return rhs > 0 && lhs.high == UINT64_C(0) && lhs.low < static_cast<std::uint64_t>(rhs);
}

template <typename SignedInteger, std::enable_if_t<std::is_integral<SignedInteger>::value && std::is_signed<SignedInteger>::value, bool> = true>
constexpr bool operator<(const SignedInteger lhs, const u128 rhs) noexcept
{
    return lhs < 0 || rhs.high > UINT64_C(0) || static_cast<std::uint64_t>(lhs) < rhs.low;
}

template <typename UnsignedInteger, std::enable_if_t<std::is_integral<UnsignedInteger>::value && std::is_unsigned<UnsignedInteger>::value, bool> = true>
constexpr bool operator<(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return lhs.high == UINT64_C(0) && lhs.low < static_cast<std::uint64_t>(rhs);
}

template <typename UnsignedInteger, std::enable_if_t<std::is_integral<UnsignedInteger>::value && std::is_unsigned<UnsignedInteger>::value, bool> = true>
constexpr bool operator<(const UnsignedInteger lhs, const u128 rhs) noexcept
{
    return rhs.high > UINT64_C(0) || static_cast<std::uint64_t>(lhs) < rhs.low;
}

constexpr bool operator<(const u128 lhs, const u128 rhs) noexcept
{
    return lhs.high == rhs.high ? lhs.low < rhs.low : lhs.high < rhs.high;
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr bool operator<(const u128 lhs, const __int128 rhs) noexcept
{
    return lhs < static_cast<u128>(rhs);
}

constexpr bool operator<(const __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) < rhs;
}

constexpr bool operator<(const u128 lhs, const unsigned __int128 rhs) noexcept
{
    return lhs < static_cast<u128>(rhs);
}

constexpr bool operator<(const unsigned __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) < rhs;
}

#endif // BOOST_DECIMAL_HAS_INT128

//=====================================
// Less-equal Operators
//=====================================

template <typename SignedInteger, std::enable_if_t<std::is_integral<SignedInteger>::value && std::is_signed<SignedInteger>::value, bool> = true>
constexpr bool operator<=(const u128 lhs, const SignedInteger rhs) noexcept
{
    return rhs > 0 && lhs.high == UINT64_C(0) && lhs.low <= static_cast<std::uint64_t>(rhs);
}

template <typename SignedInteger, std::enable_if_t<std::is_integral<SignedInteger>::value && std::is_signed<SignedInteger>::value, bool> = true>
constexpr bool operator<=(const SignedInteger lhs, const u128 rhs) noexcept
{
    return lhs < 0 || rhs.high > UINT64_C(0) || static_cast<std::uint64_t>(lhs) <= rhs.low;
}

template <typename UnsignedInteger, std::enable_if_t<std::is_integral<UnsignedInteger>::value && std::is_unsigned<UnsignedInteger>::value, bool> = true>
constexpr bool operator<=(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return lhs.high == UINT64_C(0) && lhs.low <= static_cast<std::uint64_t>(rhs);
}

template <typename UnsignedInteger, std::enable_if_t<std::is_integral<UnsignedInteger>::value && std::is_unsigned<UnsignedInteger>::value, bool> = true>
constexpr bool operator<=(const UnsignedInteger lhs, const u128 rhs) noexcept
{
    return rhs.high > UINT64_C(0) || static_cast<std::uint64_t>(lhs) <= rhs.low;
}

constexpr bool operator<=(const u128 lhs, const u128 rhs) noexcept
{
    return lhs.high == rhs.high ? lhs.low < rhs.low : lhs.high < rhs.high;
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr bool operator<=(const u128 lhs, const __int128 rhs) noexcept
{
    return lhs <= static_cast<u128>(rhs);
}

constexpr bool operator<=(const __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) <= rhs;
}

constexpr bool operator<=(const u128 lhs, const unsigned __int128 rhs) noexcept
{
    return lhs <= static_cast<u128>(rhs);
}

constexpr bool operator<=(const unsigned __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) <= rhs;
}

#endif // BOOST_DECIMAL_HAS_INT128

//=====================================
// Greater Than Operators
//=====================================

template <typename SignedInteger, std::enable_if_t<std::is_integral<SignedInteger>::value && std::is_signed<SignedInteger>::value, bool> = true>
constexpr bool operator>(const u128 lhs, const SignedInteger rhs) noexcept
{
    return rhs < 0 || lhs.high > UINT64_C(0) || lhs.low > static_cast<std::uint64_t>(rhs);
}

template <typename SignedInteger, std::enable_if_t<std::is_integral<SignedInteger>::value && std::is_signed<SignedInteger>::value, bool> = true>
constexpr bool operator>(const SignedInteger lhs, const u128 rhs) noexcept
{
    return lhs > 0 && rhs.high == UINT64_C(0) && static_cast<std::uint64_t>(lhs) > rhs.low;
}

template <typename UnsignedInteger, std::enable_if_t<std::is_integral<UnsignedInteger>::value && std::is_unsigned<UnsignedInteger>::value, bool> = true>
constexpr bool operator>(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return lhs.high > UINT64_C(0) || lhs.low > static_cast<std::uint64_t>(rhs);
}

template <typename UnsignedInteger, std::enable_if_t<std::is_integral<UnsignedInteger>::value && std::is_unsigned<UnsignedInteger>::value, bool> = true>
constexpr bool operator>(const UnsignedInteger lhs, const u128 rhs) noexcept
{
    return rhs.high == UINT64_C(0) && static_cast<std::uint64_t>(lhs) > rhs.low;
}

constexpr bool operator>(const u128 lhs, const u128 rhs) noexcept
{
    return lhs.high > rhs.high || (lhs.high == rhs.high && lhs.low > rhs.low);
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr bool operator>(const u128 lhs, const __int128 rhs) noexcept
{
    return lhs > static_cast<u128>(rhs);
}

constexpr bool operator>(const __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) > rhs;
}

constexpr bool operator>(const u128 lhs, const unsigned __int128 rhs) noexcept
{
    return lhs > static_cast<u128>(rhs);
}

constexpr bool operator>(const unsigned __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) > rhs;
}

#endif // BOOST_DECIMAL_HAS_INT128

//=====================================
// Greater-equal Operators
//=====================================

template <typename SignedInteger, std::enable_if_t<std::is_integral<SignedInteger>::value && std::is_signed<SignedInteger>::value, bool> = true>
constexpr bool operator>=(const u128 lhs, const SignedInteger rhs) noexcept
{
    return rhs < 0 || lhs.high > UINT64_C(0) || lhs.low >= static_cast<std::uint64_t>(rhs);
}

template <typename SignedInteger, std::enable_if_t<std::is_integral<SignedInteger>::value && std::is_signed<SignedInteger>::value, bool> = true>
constexpr bool operator>=(const SignedInteger lhs, const u128 rhs) noexcept
{
    return lhs > 0 && rhs.high == UINT64_C(0) && static_cast<std::uint64_t>(lhs) >= rhs.low;
}

template <typename UnsignedInteger, std::enable_if_t<std::is_integral<UnsignedInteger>::value && std::is_unsigned<UnsignedInteger>::value, bool> = true>
constexpr bool operator>=(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return lhs.high > UINT64_C(0) || lhs.low >= static_cast<std::uint64_t>(rhs);
}

template <typename UnsignedInteger, std::enable_if_t<std::is_integral<UnsignedInteger>::value && std::is_unsigned<UnsignedInteger>::value, bool> = true>
constexpr bool operator>=(const UnsignedInteger lhs, const u128 rhs) noexcept
{
    return rhs.high == UINT64_C(0) && static_cast<std::uint64_t>(lhs) >= rhs.low;
}

constexpr bool operator>=(const u128 lhs, const u128 rhs) noexcept
{
    return lhs.high > rhs.high || (lhs.high == rhs.high && lhs.low >= rhs.low);
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr bool operator>=(const u128 lhs, const __int128 rhs) noexcept
{
    return lhs >= static_cast<u128>(rhs);
}

constexpr bool operator>=(const __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) >= rhs;
}

constexpr bool operator>=(const u128 lhs, const unsigned __int128 rhs) noexcept
{
    return lhs >= static_cast<u128>(rhs);
}

constexpr bool operator>=(const unsigned __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) >= rhs;
}

#endif // BOOST_DECIMAL_HAS_INT128

//=====================================
// Not Operator
//=====================================

constexpr u128 operator~(const u128 rhs) noexcept
{
    return {~rhs.high, ~rhs.low};
}

//=====================================
// Or Operator
//=====================================

template <typename SignedInteger, std::enable_if_t<std::is_integral<SignedInteger>::value && std::is_signed<SignedInteger>::value, bool> = true>
constexpr u128 operator|(const u128 lhs, const SignedInteger rhs) noexcept
{
    return {lhs.high | (rhs < 0 ? ~UINT64_C(0) : UINT64_C(0)), lhs.low | static_cast<std::uint64_t>(rhs)};
}

template <typename SignedInteger, std::enable_if_t<std::is_integral<SignedInteger>::value && std::is_signed<SignedInteger>::value, bool> = true>
constexpr u128 operator|(const SignedInteger lhs, const u128 rhs) noexcept
{
    return {rhs.high | (lhs < 0 ? ~UINT64_C(0) : UINT64_C(0)), rhs.low | static_cast<std::uint64_t>(lhs)};
}

template <typename UnsignedInteger, std::enable_if_t<std::is_integral<UnsignedInteger>::value && std::is_unsigned<UnsignedInteger>::value, bool> = true>
constexpr u128 operator|(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return {lhs.high, lhs.low | static_cast<std::uint64_t>(rhs)};
}

template <typename UnsignedInteger, std::enable_if_t<std::is_integral<UnsignedInteger>::value && std::is_unsigned<UnsignedInteger>::value, bool> = true>
constexpr u128 operator|(const UnsignedInteger lhs, const u128 rhs) noexcept
{
    return {rhs.high, rhs.low | static_cast<std::uint64_t>(lhs)};
}

constexpr u128 operator|(const u128 lhs, const u128 rhs) noexcept
{
    return {lhs.high | rhs.high, lhs.low | rhs.low};
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr u128 operator|(const u128 lhs, const __int128 rhs) noexcept
{
    return lhs | static_cast<u128>(rhs);
}

constexpr u128 operator|(const __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) | rhs;
}

constexpr u128 operator|(const u128 lhs, const unsigned __int128 rhs) noexcept
{
    return lhs | static_cast<u128>(rhs);
}

constexpr u128 operator|(const unsigned __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) | rhs;
}

#endif // BOOST_DECIMAL_HAS_INT128

//=====================================
// And Operator
//=====================================

template <typename SignedInteger, std::enable_if_t<std::is_integral<SignedInteger>::value && std::is_signed<SignedInteger>::value, bool> = true>
constexpr u128 operator&(const u128 lhs, const SignedInteger rhs) noexcept
{
    return {lhs.high & (rhs < 0 ? ~UINT64_C(0) : UINT64_C(0)), lhs.low & static_cast<std::uint64_t>(rhs)};
}

template <typename SignedInteger, std::enable_if_t<std::is_integral<SignedInteger>::value && std::is_signed<SignedInteger>::value, bool> = true>
constexpr u128 operator&(const SignedInteger lhs, const u128 rhs) noexcept
{
    return {rhs.high & (lhs < 0 ? ~UINT64_C(0) : UINT64_C(0)), rhs.low & static_cast<std::uint64_t>(lhs)};
}

template <typename UnsignedInteger, std::enable_if_t<std::is_integral<UnsignedInteger>::value && std::is_unsigned<UnsignedInteger>::value, bool> = true>
constexpr u128 operator&(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return {lhs.high, lhs.low & static_cast<std::uint64_t>(rhs)};
}

template <typename UnsignedInteger, std::enable_if_t<std::is_integral<UnsignedInteger>::value && std::is_unsigned<UnsignedInteger>::value, bool> = true>
constexpr u128 operator&(const UnsignedInteger lhs, const u128 rhs) noexcept
{
    return {rhs.high, rhs.low & static_cast<std::uint64_t>(lhs)};
}

constexpr u128 operator&(const u128 lhs, const u128 rhs) noexcept
{
    return {lhs.high & rhs.high, lhs.low & rhs.low};
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr u128 operator&(const u128 lhs, const __int128 rhs) noexcept
{
    return lhs & static_cast<u128>(rhs);
}

constexpr u128 operator&(const __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) & rhs;
}

constexpr u128 operator&(const u128 lhs, const unsigned __int128 rhs) noexcept
{
    return lhs & static_cast<u128>(rhs);
}

constexpr u128 operator&(const unsigned __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) & rhs;
}

#endif // BOOST_DECIMAL_HAS_INT128

//=====================================
// Xor Operator
//=====================================

template <typename SignedInteger, std::enable_if_t<std::is_integral<SignedInteger>::value && std::is_signed<SignedInteger>::value, bool> = true>
constexpr u128 operator^(const u128 lhs, const SignedInteger rhs) noexcept
{
    return {lhs.high ^ (rhs < 0 ? ~UINT64_C(0) : UINT64_C(0)), lhs.low ^ static_cast<std::uint64_t>(rhs)};
}

template <typename SignedInteger, std::enable_if_t<std::is_integral<SignedInteger>::value && std::is_signed<SignedInteger>::value, bool> = true>
constexpr u128 operator^(const SignedInteger lhs, const u128 rhs) noexcept
{
    return {rhs.high ^ (lhs < 0 ? ~UINT64_C(0) : UINT64_C(0)), rhs.low ^ static_cast<std::uint64_t>(lhs)};
}

template <typename UnsignedInteger, std::enable_if_t<std::is_integral<UnsignedInteger>::value && std::is_unsigned<UnsignedInteger>::value, bool> = true>
constexpr u128 operator^(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return {lhs.high, lhs.low ^ static_cast<std::uint64_t>(rhs)};
}

template <typename UnsignedInteger, std::enable_if_t<std::is_integral<UnsignedInteger>::value && std::is_unsigned<UnsignedInteger>::value, bool> = true>
constexpr u128 operator^(const UnsignedInteger lhs, const u128 rhs) noexcept
{
    return {rhs.high, rhs.low ^ static_cast<std::uint64_t>(lhs)};
}

constexpr u128 operator^(const u128 lhs, const u128 rhs) noexcept
{
    return {lhs.high ^ rhs.high, lhs.low ^ rhs.low};
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr u128 operator^(const u128 lhs, const __int128 rhs) noexcept
{
    return lhs ^ static_cast<u128>(rhs);
}

constexpr u128 operator^(const __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) ^ rhs;
}

constexpr u128 operator^(const u128 lhs, const unsigned __int128 rhs) noexcept
{
    return lhs ^ static_cast<u128>(rhs);
}

constexpr u128 operator^(const unsigned __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) ^ rhs;
}

#endif // BOOST_DECIMAL_HAS_INT128

//=====================================
// Left Shift Operator
//=====================================

template <typename Integer, std::enable_if_t<std::is_integral<Integer>::value, bool> = true>
constexpr u128 operator<<(const u128 lhs, const Integer rhs) noexcept
{
    if (rhs < 0 || rhs >= 128)
    {
        return {0, 0};
    }

    if (rhs == 0)
    {
        return lhs;
    }

    if (rhs == 64)
    {
        return {lhs.low, 0};
    }

    if (rhs > 64)
    {
        return {lhs.low << (rhs - 64), 0};
    }

    return {
        (lhs.high << rhs) | (lhs.low >> (64 - rhs)),
        lhs.low << rhs
    };
}

template <typename Integer, std::enable_if_t<std::is_integral<Integer>::value && (sizeof(Integer) * 8 > 16), bool> = true>
constexpr Integer operator<<(const Integer lhs, const u128 rhs) noexcept
{
    constexpr auto bit_width = sizeof(Integer) * 8;

    if (rhs.high > UINT64_C(0) || rhs.low >= bit_width)
    {
        return 0;
    }

    return lhs << rhs.low;
}

template <typename Integer, std::enable_if_t<std::is_integral<Integer>::value && (sizeof(Integer) * 8 <= 16) && std::is_signed<Integer>::value, bool> = true>
constexpr int operator<<(const Integer lhs, const u128 rhs) noexcept
{
    constexpr auto bit_width = sizeof(Integer) * 8;

    if (rhs.high > UINT64_C(0) || rhs.low >= bit_width)
    {
        return 0;
    }

    return static_cast<int>(lhs) << rhs.low;
}

template <typename Integer, std::enable_if_t<std::is_integral<Integer>::value && (sizeof(Integer) * 8 <= 16) && std::is_unsigned<Integer>::value, bool> = true>
constexpr unsigned operator<<(const Integer lhs, const u128 rhs) noexcept
{
    constexpr auto bit_width = sizeof(Integer) * 8;

    if (rhs.high > UINT64_C(0) || rhs.low >= bit_width)
    {
        return 0;
    }

    return static_cast<unsigned>(lhs) << rhs.low;
}

constexpr u128 operator<<(const u128 lhs, const u128 rhs) noexcept
{
    if (rhs >= 128)
    {
        return {0, 0};
    }

    if (rhs.low == 0)
    {
        return lhs;
    }

    if (rhs.low == 64)
    {
        return {lhs.low, 0};
    }

    if (rhs.low > 64)
    {
        return {lhs.low << (rhs.low - 64), 0};
    }

    return {
        (lhs.high << rhs.low) | (lhs.low >> (64 - rhs.low)),
        lhs.low << rhs.low
    };
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr u128 operator<<(const u128 lhs, const __int128 rhs) noexcept
{
    return lhs << static_cast<u128>(rhs);
}

constexpr __int128 operator<<(const __int128 lhs, const u128 rhs) noexcept
{
    return lhs << static_cast<__int128>(rhs);
}

constexpr u128 operator<<(const u128 lhs, const unsigned __int128 rhs) noexcept
{
    return lhs << static_cast<u128>(rhs);
}

constexpr unsigned __int128 operator<<(const unsigned __int128 lhs, const u128 rhs) noexcept
{
    return lhs << static_cast<unsigned __int128>(rhs);
}

#endif // BOOST_DECIMAL_HAS_INT128

//=====================================
// Right Shift Operator
//=====================================

template <typename Integer, std::enable_if_t<std::is_integral<Integer>::value, bool> = true>
constexpr u128 operator>>(const u128 lhs, const Integer rhs) noexcept
{
    if (rhs < 0 || rhs >= 128)
    {
        return {0, 0};
    }

    if (rhs == 0)
    {
        return lhs;
    }

    if (rhs == 64)
    {
        return {0, lhs.high};
    }

    if (rhs > 64)
    {
        return {0, lhs.high >> (rhs - 64)};
    }

    return {
        lhs.high >> rhs,
        (lhs.low >> rhs) | (lhs.high << (64 - rhs))
    };
}

template <typename Integer, std::enable_if_t<std::is_integral<Integer>::value && (sizeof(Integer) * 8 > 16), bool> = true>
constexpr Integer operator>>(const Integer lhs, const u128 rhs) noexcept
{
    constexpr auto bit_width = sizeof(Integer) * 8;

    if (rhs.high > UINT64_C(0) || rhs.low >= bit_width)
    {
        return 0;
    }

    return lhs >> rhs.low;
}

template <typename Integer, std::enable_if_t<std::is_integral<Integer>::value && (sizeof(Integer) * 8 <= 16) && std::is_signed<Integer>::value, bool> = true>
constexpr int operator>>(const Integer lhs, const u128 rhs) noexcept
{
    constexpr auto bit_width = sizeof(Integer) * 8;

    if (rhs.high > UINT64_C(0) || rhs.low >= bit_width)
    {
        return 0;
    }

    return static_cast<int>(lhs) >> rhs.low;
}

template <typename Integer, std::enable_if_t<std::is_integral<Integer>::value && (sizeof(Integer) * 8 <= 16) && std::is_unsigned<Integer>::value, bool> = true>
constexpr unsigned operator>>(const Integer lhs, const u128 rhs) noexcept
{
    constexpr auto bit_width = sizeof(Integer) * 8;

    if (rhs.high > UINT64_C(0) || rhs.low >= bit_width)
    {
        return 0;
    }

    return static_cast<unsigned>(lhs) >> rhs.low;
}

constexpr u128 operator>>(const u128 lhs, const u128 rhs) noexcept
{
    if (rhs >= 128)
    {
        return {0, 0};
    }

    if (rhs.low == 0)
    {
        return lhs;
    }

    if (rhs.low == 64)
    {
        return {0, lhs.high};
    }

    if (rhs.low > 64)
    {
        return {0, lhs.high >> (rhs.low - UINT64_C(64))};
    }

    return {
        lhs.high >> rhs.low,
        (lhs.low >> rhs.low) | (lhs.high << (UINT64_C(64) - rhs.low))
    };
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr u128 operator>>(const u128 lhs, const __int128 rhs) noexcept
{
    return lhs >> static_cast<u128>(rhs);
}

constexpr __int128 operator>>(const __int128 lhs, const u128 rhs) noexcept
{
    return lhs >> static_cast<__int128>(rhs);
}

constexpr u128 operator>>(const u128 lhs, const unsigned __int128 rhs) noexcept
{
    return lhs >> static_cast<u128>(rhs);
}

constexpr unsigned __int128 operator>>(const unsigned __int128 lhs, const u128 rhs) noexcept
{
    return lhs >> static_cast<unsigned __int128>(rhs);
}

#endif // BOOST_DECIMAL_HAS_INT128

//=====================================
// Addition Operator
//=====================================

namespace impl {

BOOST_DECIMAL_FORCE_INLINE constexpr u128 default_add(const u128 lhs, const std::uint64_t rhs) noexcept
{
    u128 temp {lhs.high, lhs.low + rhs};

    if (temp.low < lhs.low)
    {
        ++temp.high;
    }

    return temp;
}

BOOST_DECIMAL_FORCE_INLINE constexpr u128 default_add(const u128 lhs, const u128 rhs) noexcept
{
    u128 temp {lhs.high + rhs.high, lhs.low + rhs.low};

    if (temp.low < lhs.low)
    {
        ++temp.high;
    }

    return temp;
}

BOOST_DECIMAL_FORCE_INLINE constexpr u128 default_sub(const u128 lhs, const u128 rhs) noexcept
{
    u128 temp {lhs.high - rhs.high, lhs.low - rhs.low};

    // Check for carry
    if (lhs.low < rhs.low)
    {
        --temp.high;
    }

    return temp;
}

BOOST_DECIMAL_FORCE_INLINE constexpr u128 default_sub(const u128 lhs, const std::uint64_t rhs) noexcept
{
    u128 temp {lhs.high, lhs.low - rhs};

    // Check for carry
    if (lhs.low < rhs)
    {
        --temp.high;
    }

    return temp;
}

#if defined(__aarch64__) || defined(_M_ARM64)

// Inline ASM is not constexpr until C++20 so we offload into this function
// in the non-constant evaluated case.
BOOST_DECIMAL_FORCE_INLINE u128 arm_asm_add(const u128 lhs, const u128 rhs) noexcept
{
    std::uint64_t result_low {};
    std::uint64_t result_high {};

    // Use inline assembly to access the carry flag directly
    // Roughly equivalent to the ADX instructions below for x64 platforms
    __asm__ volatile(
        "adds %0, %2, %3\n"  // adds sets carry flag if overflow occurs
        "adc %1, %4, %5\n"   // adc adds with carry from previous operation
        : "=r" (result_low), "=r" (result_high)
        : "r" (lhs.low), "r" (rhs.low), "r" (lhs.high), "r" (rhs.high)
        : "cc"               // clobbering condition codes (flags)
    );

    return {result_high, result_low};
}

BOOST_DECIMAL_FORCE_INLINE u128 arm_asm_add(const u128 lhs, const std::uint64_t rhs) noexcept
{
    std::uint64_t result_low {};
    std::uint64_t result_high {};

    // Use inline assembly to access the carry flag directly
    // Roughly equivalent to the ADX instructions below for x64 platforms
    __asm__ volatile(
        "adds %0, %2, %3\n"  // adds sets carry flag if overflow occurs
        "adc %1, %4, xzr\n"   // add carry to high
        : "=r" (result_low), "=r" (result_high)
        : "r" (lhs.low), "r" (rhs), "r" (lhs.high)
        : "cc"               // clobbering condition codes (flags)
    );

    return {result_high, result_low};
}

BOOST_DECIMAL_FORCE_INLINE u128 arm_asm_sub(const u128 lhs, const u128 rhs) noexcept
{
    std::uint64_t result_low {};
    std::uint64_t result_high {};

    // Use inline assembly to access the carry flag directly
    // Roughly equivalent to the ADX instructions below for x64 platforms
    __asm__ volatile(
        "subs %0, %2, %3\n"  // adds sets carry flag if overflow occurs
        "sbc %1, %4, %5\n"   // adc adds with carry from previous operation
        : "=r" (result_low), "=r" (result_high)
        : "r" (lhs.low), "r" (rhs.low), "r" (lhs.high), "r" (rhs.high)
        : "cc"               // clobbering condition codes (flags)
    );

    return {result_high, result_low};
}

BOOST_DECIMAL_FORCE_INLINE u128 arm_asm_sub(const u128 lhs, const std::uint64_t rhs) noexcept
{
    std::uint64_t result_low {};
    std::uint64_t result_high {};

    // Use inline assembly to access the carry flag directly
    // Roughly equivalent to the ADX instructions below for x64 platforms
    __asm__ volatile(
        "subs %0, %2, %3\n"  // subs sets carry flag if underflow occurs
        "sbc %1, %4, xzr\n"   // subtract carry from high value
        : "=r" (result_low), "=r" (result_high)
        : "r" (lhs.low), "r" (rhs), "r" (lhs.high)
        : "cc"               // clobbering condition codes (flags)
    );

    return {result_high, result_low};
}

#endif // defined(__aarch64__) || defined(_M_ARM64)

#ifdef BOOST_DECIMAL_ADD_CARRY

BOOST_DECIMAL_FORCE_INLINE u128 adx_add(const u128 lhs, const u128 rhs) noexcept
{
    // Intel ADX instructions are specifically for Multi-Precision Arithmetic
    unsigned long long int res_low {};
    unsigned long long int res_high {};

    const unsigned char carry {BOOST_DECIMAL_ADD_CARRY(0, lhs.low, rhs.low, &res_low)};
    BOOST_DECIMAL_ADD_CARRY(carry, lhs.high, rhs.high, &res_high);

    return {res_high, res_low};
}

BOOST_DECIMAL_FORCE_INLINE u128 adx_add(const u128 lhs, const std::uint64_t rhs) noexcept
{
    // Intel ADX instructions are specifically for Multi-Precision Arithmetic
    unsigned long long int res_low {};
    const unsigned char carry {BOOST_DECIMAL_ADD_CARRY(0, lhs.low, rhs, &res_low)};

    // Unconditionally add the carry to lhs.high since we don't have multiple additions here
    return {lhs.high + static_cast<std::uint64_t>(carry), res_low};
}

BOOST_DECIMAL_FORCE_INLINE u128 adx_sub(const u128 lhs, const u128 rhs) noexcept
{
    unsigned long long int res_low {};
    unsigned long long int res_high {};

    const unsigned char carry {BOOST_DECIMAL_SUB_BORROW(0, lhs.low, rhs.low, &res_low)};
    BOOST_DECIMAL_SUB_BORROW(carry, lhs.high, rhs.high, &res_high);

    return {res_high, res_low};
}

BOOST_DECIMAL_FORCE_INLINE u128 adx_sub(const u128 lhs, const std::uint64_t rhs) noexcept
{
    unsigned long long int res_low {};
    const unsigned char carry {BOOST_DECIMAL_SUB_BORROW(0, lhs.low, rhs, &res_low)};

    return {lhs.high - static_cast<std::uint64_t>(carry), res_low};
}

#endif // BOOST_DECIMAL_ADD_CARRY

} // namespace impl

template <typename UnsignedInteger, std::enable_if_t<std::is_unsigned<UnsignedInteger>::value || std::is_same<UnsignedInteger, u128>::value, bool> = true>
constexpr u128 operator+(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    #ifndef BOOST_DECIMAL_NO_CONSTEVAL_DETECTION

    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
    {
        return impl::default_add(lhs, rhs);
    }
    else
    {
        #if defined(__aarch64__) || defined(_M_ARM64)

        return impl::arm_asm_add(lhs, rhs);

        #elif defined(BOOST_DECIMAL_ADD_CARRY)

        return impl::adx_add(lhs, rhs);

        #else

        return impl::default_add(lhs, rhs);

        #endif
    }

    #else

    return impl::default_add(lhs, rhs);

    #endif
}

// Since unsigned addition is trivially commutative we just reverse the order of the operands into the impl functions
template <typename UnsignedInteger, std::enable_if_t<std::is_unsigned<UnsignedInteger>::value, bool> = true>
constexpr u128 operator+(const UnsignedInteger lhs, const u128 rhs) noexcept
{
    #ifndef BOOST_DECIMAL_NO_CONSTEVAL_DETECTION

    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
    {
        return impl::default_add(rhs, lhs);
    }
    else
    {
        #if defined(__aarch64__) || defined(_M_ARM64)

        return impl::arm_asm_add(rhs, lhs);

        #elif defined(BOOST_DECIMAL_ADD_CARRY)

        return impl::adx_add(rhs, lhs);

        #else

        return impl::default_add(rhs, lhs);

        #endif
    }

    #else

    return impl::default_add(rhs, lhs);

    #endif
}

template <typename SignedInteger, std::enable_if_t<std::is_signed<SignedInteger>::value, bool> = true>
constexpr u128 operator+(const u128 lhs, const SignedInteger rhs) noexcept
{
    if (rhs > 0)
    {
        #ifndef BOOST_DECIMAL_NO_CONSTEVAL_DETECTION

        if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
        {
            return impl::default_add(lhs, static_cast<std::uint64_t>(rhs));
        }
        else
        {
            #if defined(__aarch64__) || defined(_M_ARM64)

            return impl::arm_asm_add(lhs, static_cast<std::uint64_t>(rhs));

            #elif defined(BOOST_DECIMAL_ADD_CARRY)

            return impl::adx_add(lhs, static_cast<std::uint64_t>(rhs));

            #else

            return impl::default_add(lhs, static_cast<std::uint64_t>(rhs));

            #endif
        }

        #else

        return impl::default_add(lhs, static_cast<std::uint64_t>(rhs));

        #endif
    }
    else
    {
        const auto unsigned_rhs {detail::make_positive_unsigned(rhs)};
        #ifndef BOOST_DECIMAL_NO_CONSTEVAL_DETECTION

        if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
        {
            return impl::default_sub(lhs, unsigned_rhs);
        }
        else
        {
            #if defined(__aarch64__) || defined(_M_ARM64)

            return impl::arm_asm_sub(lhs, unsigned_rhs);

            #elif defined(BOOST_DECIMAL_ADD_CARRY)

            return impl::adx_sub(lhs, unsigned_rhs);

            #else

            return impl::default_sub(lhs, unsigned_rhs);

            #endif
        }

        #else

        return impl::default_sub(lhs, unsigned_rhs);

        #endif
    }
}

// -a + b == b - a
//  a + b == b + a
template <typename SignedInteger, std::enable_if_t<std::is_signed<SignedInteger>::value, bool> = true>
constexpr u128 operator+(const SignedInteger lhs, const u128 rhs) noexcept
{
    if (lhs > 0)
    {
        #ifndef BOOST_DECIMAL_NO_CONSTEVAL_DETECTION

        if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
        {
            return impl::default_add(rhs, static_cast<std::uint64_t>(lhs));
        }
        else
        {
            #if defined(__aarch64__) || defined(_M_ARM64)

            return impl::arm_asm_add(rhs, static_cast<std::uint64_t>(lhs));

            #elif defined(BOOST_DECIMAL_ADD_CARRY)

            return impl::adx_add(rhs, static_cast<std::uint64_t>(lhs));

            #else

            return impl::default_add(rhs, static_cast<std::uint64_t>(lhs));

            #endif
        }

        #else

        return impl::default_add(rhs, static_cast<std::uint64_t>(lhs));

        #endif
    }
    else
    {
        const auto unsigned_lhs {detail::make_positive_unsigned(lhs)};
        #ifndef BOOST_DECIMAL_NO_CONSTEVAL_DETECTION

        if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
        {
            return impl::default_sub(rhs, unsigned_lhs);
        }
        else
        {
            #if defined(__aarch64__) || defined(_M_ARM64)

            return impl::arm_asm_sub(rhs, unsigned_lhs);

            #elif defined(BOOST_DECIMAL_ADD_CARRY)

            return impl::adx_sub(rhs, unsigned_lhs);

            #else

            return impl::default_sub(rhs, unsigned_lhs);

            #endif
        }

        #else

        return impl::default_sub(rhs, unsigned_lhs);

        #endif
    }
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr u128 operator+(const u128 lhs, const __int128 rhs) noexcept
{
    return lhs + static_cast<u128>(rhs);
}

constexpr u128 operator+(const __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) + rhs;
}

constexpr u128 operator+(const u128 lhs, const unsigned __int128 rhs) noexcept
{
    return lhs + static_cast<u128>(rhs);
}

constexpr u128 operator+(const unsigned __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) + rhs;
}

#endif // BOOST_DECIMAL_HAS_INT128

//=====================================
// Compound Addition Operator
//=====================================

constexpr u128& u128::operator+=(const std::uint8_t rhs) noexcept
{
    *this = *this + rhs;
    return *this;
}

constexpr u128& u128::operator+=(const std::uint16_t rhs) noexcept
{
    *this = *this + rhs;
    return *this;
}

constexpr u128& u128::operator+=(const std::uint32_t rhs) noexcept
{
    *this = *this + rhs;
    return *this;
}

constexpr u128& u128::operator+=(const std::uint64_t rhs) noexcept
{
    *this = *this + rhs;
    return *this;
}

constexpr u128& u128::operator+=(const std::int8_t rhs) noexcept
{
    *this = *this + rhs;
    return *this;
}

constexpr u128& u128::operator+=(const std::int16_t rhs) noexcept
{
    *this = *this + rhs;
    return *this;
}

constexpr u128& u128::operator+=(const std::int32_t rhs) noexcept
{
    *this = *this + rhs;
    return *this;
}

constexpr u128& u128::operator+=(const std::int64_t rhs) noexcept
{
    *this = *this + rhs;
    return *this;
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr u128& u128::operator+=(const __int128 rhs) noexcept
{
    *this = *this + rhs;
    return *this;
}

constexpr u128& u128::operator+=(const unsigned __int128 rhs) noexcept
{
    *this = *this + rhs;
    return *this;
}

#endif

//=====================================
// Subtraction Operator
//=====================================

template <typename UnsignedInteger, std::enable_if_t<std::is_unsigned<UnsignedInteger>::value || std::is_same<UnsignedInteger, u128>::value, bool> = true>
constexpr u128 operator-(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    #ifndef BOOST_DECIMAL_NO_CONSTEVAL_DETECTION

    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
    {
        return impl::default_sub(lhs, rhs);
    }
    else
    {
        #if defined(__aarch64__) || defined(_M_ARM64)

        return impl::arm_asm_sub(lhs, rhs);

        #elif defined(BOOST_DECIMAL_ADD_CARRY)

        return impl::adx_sub(lhs, rhs);

        #else

        return impl::default_sub(lhs, rhs);

        #endif
    }

    #else

    return impl::default_sub(lhs, rhs);

    #endif
}

template <typename UnsignedInteger, std::enable_if_t<std::is_unsigned<UnsignedInteger>::value, bool> = true>
constexpr u128 operator-(const UnsignedInteger lhs, const u128 rhs) noexcept
{
    // The only real viable way to do this is to promote lhs and perform 128-bit sub
    return u128{UINT64_C(0), lhs} - rhs;
}

template <typename SignedInteger, std::enable_if_t<std::is_signed<SignedInteger>::value, bool> = true>
constexpr u128 operator-(const u128 lhs, const SignedInteger rhs) noexcept
{
    if (rhs < 0)
    {
        #ifndef BOOST_DECIMAL_NO_CONSTEVAL_DETECTION

        if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
        {
            return impl::default_add(lhs, static_cast<std::uint64_t>(rhs));
        }
        else
        {
            #if defined(__aarch64__) || defined(_M_ARM64)

            return impl::arm_asm_add(lhs, static_cast<std::uint64_t>(rhs));

            #elif defined(BOOST_DECIMAL_ADD_CARRY)

            return impl::adx_add(lhs, static_cast<std::uint64_t>(rhs));

            #else

            return impl::default_add(lhs, static_cast<std::uint64_t>(rhs));

            #endif
        }

        #else

        return impl::default_add(lhs, static_cast<std::uint64_t>(rhs));

        #endif
    }
    else
    {
        const auto unsigned_rhs {detail::make_positive_unsigned(rhs)};
        #ifndef BOOST_DECIMAL_NO_CONSTEVAL_DETECTION

        if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
        {
            return impl::default_sub(lhs, unsigned_rhs);
        }
        else
        {
            #if defined(__aarch64__) || defined(_M_ARM64)

            return impl::arm_asm_sub(lhs, unsigned_rhs);

            #elif defined(BOOST_DECIMAL_ADD_CARRY)

            return impl::adx_sub(lhs, unsigned_rhs);

            #else

            return impl::default_sub(lhs, unsigned_rhs);

            #endif
        }

        #else

        return impl::default_sub(lhs, unsigned_rhs);

        #endif
    }
}

template <typename SignedInteger, std::enable_if_t<std::is_signed<SignedInteger>::value, bool> = true>
constexpr u128 operator-(const SignedInteger lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) - rhs;
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr u128 operator-(const u128 lhs, const __int128 rhs) noexcept
{
    return lhs - static_cast<u128>(rhs);
}

constexpr u128 operator-(const __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) - rhs;
}

constexpr u128 operator-(const u128 lhs, const unsigned __int128 rhs) noexcept
{
    return lhs - static_cast<u128>(rhs);
}

constexpr u128 operator-(const unsigned __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) - rhs;
}

#endif // BOOST_DECIMAL_HAS_INT128

} // namespace detail
} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_U128_HPP
