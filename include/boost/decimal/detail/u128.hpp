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

namespace impl {

template <typename T>
struct signed_integer
{
    static constexpr bool value = std::is_signed<T>::value && std::is_integral<T>::value;
};

template <typename T>
static constexpr bool is_signed_integer_v = signed_integer<T>::value;

template <typename T>
struct unsigned_integer
{
    static constexpr bool value = std::is_unsigned<T>::value && std::is_integral<T>::value;
};

template <typename T>
static constexpr bool is_unsigned_integer_v = unsigned_integer<T>::value;

}

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
    template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
    explicit constexpr u128(const SignedInteger value) noexcept : low {static_cast<std::uint64_t>(value)}, high {value < 0 ? UINT64_MAX : UINT64_C(0)} {}

    #ifdef BOOST_DECIMAL_HAS_INT128
    explicit constexpr u128(const __int128 value) noexcept :
        low {static_cast<std::uint64_t>(value & low_word_mask)},
        high {static_cast<std::uint64_t>(static_cast<unsigned __int128>(value) >> 64U)} {}
    #endif // BOOST_DECIMAL_HAS_INT128

    // Unsigned arithmetic constructors
    template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
    explicit constexpr u128(const UnsignedInteger value) noexcept : low {static_cast<std::uint64_t>(value)}, high {UINT64_C(0)} {}

    #ifdef BOOST_DECIMAL_HAS_INT128
    explicit constexpr u128(const unsigned __int128 value) noexcept :
        low {static_cast<std::uint64_t>(value & low_word_mask)},
        high {static_cast<std::uint64_t>(value >> 64U)} {}
    #endif // BOOST_DECIMAL_HAS_INT128

    // Signed assignment operators
    template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
    constexpr u128& operator=(SignedInteger value) noexcept;

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr u128& operator=(__int128 value) noexcept;
    #endif // BOOST_DECIMAL_HAS_INT128

    // Unsigned assignment operators
    template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
    constexpr u128& operator=(UnsignedInteger value) noexcept;

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr u128& operator=(unsigned __int128 value) noexcept;
    #endif

    // Bool conversion is not explicit so we can do stuff like if (num)
    constexpr operator bool() const noexcept { return low || high; }

    // Conversion to signed integer types
    template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
    explicit constexpr operator SignedInteger() const noexcept { return static_cast<SignedInteger>(low); }

    #ifdef BOOST_DECIMAL_HAS_INT128
    explicit constexpr operator __int128() const noexcept { return (static_cast<__int128>(high) << 64) + low; }
    #endif // BOOST_DECIMAL_HAS_INT128

    // Conversion to unsigned integer types
    template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
    explicit constexpr operator UnsignedInteger() const noexcept { return static_cast<UnsignedInteger>(low); }

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

    // Prefix and postfix increment
    constexpr u128& operator++() noexcept;
    constexpr u128& operator++(int) noexcept;

    // Compound Addition Operators
    template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
    constexpr u128& operator+=(SignedInteger rhs) noexcept;

    template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
    constexpr u128& operator+=(UnsignedInteger rhs) noexcept;

    constexpr u128& operator+=(u128 rhs) noexcept;

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr u128& operator+=(__int128 rhs) noexcept;
    constexpr u128& operator+=(unsigned __int128 rhs) noexcept;
    #endif // BOOST_DECIMAL_HAS_INT128

    // Compound Subtraction Operators
    template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
    constexpr u128& operator-=(SignedInteger rhs) noexcept;

    template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
    constexpr u128& operator-=(UnsignedInteger rhs) noexcept;

    constexpr u128& operator-=(u128 rhs) noexcept;

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr u128& operator-=(__int128 rhs) noexcept;
    constexpr u128& operator-=(unsigned __int128 rhs) noexcept;
    #endif // BOOST_DECIMAL_HAS_INT128

    // Compound Multiplication Operators
    template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
    constexpr u128& operator*=(SignedInteger rhs) noexcept;

    template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
    constexpr u128& operator*=(UnsignedInteger rhs) noexcept;

    constexpr u128& operator*=(u128 rhs) noexcept;

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr u128& operator*=(__int128 rhs) noexcept;
    constexpr u128& operator*=(unsigned __int128 rhs) noexcept;
    #endif // BOOST_DECIMAL_HAS_INT128

    // Compound Division Operators
    template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
    constexpr u128& operator/=(SignedInteger rhs) noexcept;

    template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
    constexpr u128& operator/=(UnsignedInteger rhs) noexcept;

    constexpr u128& operator/=(u128 rhs) noexcept;

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr u128& operator/=(__int128 rhs) noexcept;
    constexpr u128& operator/=(unsigned __int128 rhs) noexcept;
    #endif // BOOST_DECIMAL_HAS_INT128
};

// Signed assignment operators
template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool>>
constexpr u128& u128::operator=(const SignedInteger value) noexcept
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
template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool>>
constexpr u128& u128::operator=(const UnsignedInteger value) noexcept { low = static_cast<std::uint64_t>(value); high = UINT64_C(0); return *this; }

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
// Increment Operators
//=====================================

constexpr u128& u128::operator++() noexcept
{
    if (++low == UINT64_C(0))
    {
        ++high;
    }

    return *this;
}

constexpr u128& u128::operator++(int) noexcept
{
    return ++(*this);
}

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

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr bool operator==(const u128 lhs, const SignedInteger rhs) noexcept
{
    return rhs >= 0 && lhs.high == UINT64_C(0) && lhs.low == static_cast<std::uint64_t>(rhs);
}

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr bool operator==(const SignedInteger lhs, const u128 rhs) noexcept
{
    return lhs >= 0 && rhs.high == UINT64_C(0) && rhs.low == static_cast<std::uint64_t>(lhs);
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
constexpr bool operator==(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return lhs.high == UINT64_C(0) && lhs.low == static_cast<std::uint64_t>(rhs);
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
constexpr bool operator==(const UnsignedInteger lhs, const u128 rhs) noexcept
{
    return rhs.high == UINT64_C(0) && rhs.low == static_cast<std::uint64_t>(lhs);
}

constexpr bool operator==(const u128 lhs, const u128 rhs) noexcept
{
    // Intel and ARM like the values in opposite directions

    #if defined(__aarch64__) || defined(_M_ARM64)

    return lhs.low == rhs.low && lhs.high == rhs.high;

    #else

    return lhs.high == rhs.high && lhs.low == rhs.low;

    #endif
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

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr bool operator!=(const u128 lhs, const SignedInteger rhs) noexcept
{
    return rhs < 0 || lhs.high != UINT64_C(0) || lhs.low != static_cast<std::uint64_t>(rhs);
}

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr bool operator!=(const SignedInteger lhs, const u128 rhs) noexcept
{
    return lhs < 0 || rhs.high != UINT64_C(0) || rhs.low != static_cast<std::uint64_t>(lhs);
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
constexpr bool operator!=(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return lhs.high != UINT64_C(0) || lhs.low != static_cast<std::uint64_t>(rhs);
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
constexpr bool operator!=(const UnsignedInteger lhs, const u128 rhs) noexcept
{
    return rhs.high != UINT64_C(0) || rhs.low != static_cast<std::uint64_t>(lhs);
}

constexpr bool operator!=(const u128 lhs, const u128 rhs) noexcept
{
    #if defined(__aarch64__) || defined(_M_ARM64)

    return lhs.low != rhs.low || lhs.high != rhs.high;

    #else

    return lhs.high != rhs.high || lhs.low != rhs.low;

    #endif
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

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr bool operator<(const u128 lhs, const SignedInteger rhs) noexcept
{
    return rhs > 0 && lhs.high == UINT64_C(0) && lhs.low < static_cast<std::uint64_t>(rhs);
}

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr bool operator<(const SignedInteger lhs, const u128 rhs) noexcept
{
    return lhs < 0 || rhs.high > UINT64_C(0) || static_cast<std::uint64_t>(lhs) < rhs.low;
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
constexpr bool operator<(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return lhs.high == UINT64_C(0) && lhs.low < static_cast<std::uint64_t>(rhs);
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
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

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr bool operator<=(const u128 lhs, const SignedInteger rhs) noexcept
{
    return rhs > 0 && lhs.high == UINT64_C(0) && lhs.low <= static_cast<std::uint64_t>(rhs);
}

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr bool operator<=(const SignedInteger lhs, const u128 rhs) noexcept
{
    return lhs < 0 || rhs.high > UINT64_C(0) || static_cast<std::uint64_t>(lhs) <= rhs.low;
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
constexpr bool operator<=(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return lhs.high == UINT64_C(0) && lhs.low <= static_cast<std::uint64_t>(rhs);
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
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

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr bool operator>(const u128 lhs, const SignedInteger rhs) noexcept
{
    return rhs < 0 || lhs.high > UINT64_C(0) || lhs.low > static_cast<std::uint64_t>(rhs);
}

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr bool operator>(const SignedInteger lhs, const u128 rhs) noexcept
{
    return lhs > 0 && rhs.high == UINT64_C(0) && static_cast<std::uint64_t>(lhs) > rhs.low;
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
constexpr bool operator>(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return lhs.high > UINT64_C(0) || lhs.low > static_cast<std::uint64_t>(rhs);
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
constexpr bool operator>(const UnsignedInteger lhs, const u128 rhs) noexcept
{
    return rhs.high == UINT64_C(0) && static_cast<std::uint64_t>(lhs) > rhs.low;
}

constexpr bool operator>(const u128 lhs, const u128 rhs) noexcept
{
    return lhs.high == rhs.high ? rhs.low < lhs.low : rhs.high < lhs.high;
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

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr bool operator>=(const u128 lhs, const SignedInteger rhs) noexcept
{
    return rhs < 0 || lhs.high > UINT64_C(0) || lhs.low >= static_cast<std::uint64_t>(rhs);
}

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr bool operator>=(const SignedInteger lhs, const u128 rhs) noexcept
{
    return lhs > 0 && rhs.high == UINT64_C(0) && static_cast<std::uint64_t>(lhs) >= rhs.low;
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
constexpr bool operator>=(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return lhs.high > UINT64_C(0) || lhs.low >= static_cast<std::uint64_t>(rhs);
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
constexpr bool operator>=(const UnsignedInteger lhs, const u128 rhs) noexcept
{
    return rhs.high == UINT64_C(0) && static_cast<std::uint64_t>(lhs) >= rhs.low;
}

constexpr bool operator>=(const u128 lhs, const u128 rhs) noexcept
{
    return lhs.high == rhs.high ? rhs.low <= lhs.low : rhs.high <= lhs.high;
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

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr u128 operator|(const u128 lhs, const SignedInteger rhs) noexcept
{
    return {lhs.high | (rhs < 0 ? ~UINT64_C(0) : UINT64_C(0)), lhs.low | static_cast<std::uint64_t>(rhs)};
}

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr u128 operator|(const SignedInteger lhs, const u128 rhs) noexcept
{
    return {rhs.high | (lhs < 0 ? ~UINT64_C(0) : UINT64_C(0)), rhs.low | static_cast<std::uint64_t>(lhs)};
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
constexpr u128 operator|(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return {lhs.high, lhs.low | static_cast<std::uint64_t>(rhs)};
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
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

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr u128 operator&(const u128 lhs, const SignedInteger rhs) noexcept
{
    return {lhs.high & (rhs < 0 ? ~UINT64_C(0) : UINT64_C(0)), lhs.low & static_cast<std::uint64_t>(rhs)};
}

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr u128 operator&(const SignedInteger lhs, const u128 rhs) noexcept
{
    return {rhs.high & (lhs < 0 ? ~UINT64_C(0) : UINT64_C(0)), rhs.low & static_cast<std::uint64_t>(lhs)};
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
constexpr u128 operator&(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return {lhs.high, lhs.low & static_cast<std::uint64_t>(rhs)};
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
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

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr u128 operator^(const u128 lhs, const SignedInteger rhs) noexcept
{
    return {lhs.high ^ (rhs < 0 ? ~UINT64_C(0) : UINT64_C(0)), lhs.low ^ static_cast<std::uint64_t>(rhs)};
}

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr u128 operator^(const SignedInteger lhs, const u128 rhs) noexcept
{
    return {rhs.high ^ (lhs < 0 ? ~UINT64_C(0) : UINT64_C(0)), rhs.low ^ static_cast<std::uint64_t>(lhs)};
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
constexpr u128 operator^(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return {lhs.high, lhs.low ^ static_cast<std::uint64_t>(rhs)};
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
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

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger> && (sizeof(SignedInteger) * 8 <= 16), bool> = true>
constexpr int operator<<(const SignedInteger lhs, const u128 rhs) noexcept
{
    constexpr auto bit_width = sizeof(SignedInteger) * 8;

    if (rhs.high > UINT64_C(0) || rhs.low >= bit_width)
    {
        return 0;
    }

    return static_cast<int>(lhs) << rhs.low;
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger> && (sizeof(UnsignedInteger) * 8 <= 16), bool> = true>
constexpr unsigned operator<<(const UnsignedInteger lhs, const u128 rhs) noexcept
{
    constexpr auto bit_width = sizeof(UnsignedInteger) * 8;

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

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger> && (sizeof(SignedInteger) * 8 <= 16), bool> = true>
constexpr int operator>>(const SignedInteger lhs, const u128 rhs) noexcept
{
    constexpr auto bit_width = sizeof(SignedInteger) * 8;

    if (rhs.high > UINT64_C(0) || rhs.low >= bit_width)
    {
        return 0;
    }

    return static_cast<int>(lhs) >> rhs.low;
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger> && (sizeof(UnsignedInteger) * 8 <= 16), bool> = true>
constexpr unsigned operator>>(UnsignedInteger lhs, const u128 rhs) noexcept
{
    constexpr auto bit_width = sizeof(UnsignedInteger) * 8;

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
    #ifdef __x86_64__

    return static_cast<u128>(static_cast<unsigned __int128>(lhs) - static_cast<unsigned __int128>(rhs));

    #else

    u128 temp {lhs.high - rhs.high, lhs.low - rhs.low};

    // Check for carry
    if (lhs.low < rhs.low)
    {
        --temp.high;
    }

    return temp;

    #endif
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

} // namespace impl

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger> || std::is_same<UnsignedInteger, u128>::value, bool> = true>
constexpr u128 operator+(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return impl::default_add(lhs, rhs);
}

// Since unsigned addition is trivially commutative we just reverse the order of the operands into the impl functions
template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
constexpr u128 operator+(const UnsignedInteger lhs, const u128 rhs) noexcept
{
    return impl::default_add(rhs, lhs);
}

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr u128 operator+(const u128 lhs, const SignedInteger rhs) noexcept
{
    if (rhs > 0)
    {
        return impl::default_add(lhs, static_cast<std::uint64_t>(rhs));
    }
    else
    {
        const auto unsigned_rhs {detail::make_positive_unsigned(rhs)};
        return impl::default_sub(lhs, unsigned_rhs);
    }
}

// -a + b == b - a
//  a + b == b + a
template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr u128 operator+(const SignedInteger lhs, const u128 rhs) noexcept
{
    if (lhs > 0)
    {
        return impl::default_add(rhs, static_cast<std::uint64_t>(lhs));
    }
    else
    {
        const auto unsigned_lhs {detail::make_positive_unsigned(lhs)};
        return impl::default_sub(rhs, unsigned_lhs);
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

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool>>
constexpr u128& u128::operator+=(const UnsignedInteger rhs) noexcept
{
    *this = *this + rhs;
    return *this;
}

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool>>
constexpr u128& u128::operator+=(const SignedInteger rhs) noexcept
{
    *this = *this + rhs;
    return *this;
}

constexpr u128& u128::operator+=(const u128 rhs) noexcept
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

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger> || std::is_same<UnsignedInteger, u128>::value, bool> = true>
constexpr u128 operator-(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return impl::default_sub(lhs, rhs);
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
constexpr u128 operator-(const UnsignedInteger lhs, const u128 rhs) noexcept
{
    // The only real viable way to do this is to promote lhs and perform 128-bit sub
    return u128{UINT64_C(0), lhs} - rhs;
}

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr u128 operator-(const u128 lhs, const SignedInteger rhs) noexcept
{
    if (rhs < 0)
    {
        return impl::default_add(lhs, static_cast<std::uint64_t>(rhs));
    }
    else
    {
        const auto unsigned_rhs {detail::make_positive_unsigned(rhs)};
        return impl::default_sub(lhs, unsigned_rhs);
    }
}

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
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

//=====================================
// Compound Subtraction Operator
//=====================================

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool>>
constexpr u128& u128::operator-=(const SignedInteger rhs) noexcept
{
    *this = *this - rhs;
    return *this;
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool>>
constexpr u128& u128::operator-=(const UnsignedInteger rhs) noexcept
{
    *this = *this - rhs;
    return *this;
}

constexpr u128& u128::operator-=(const u128 rhs) noexcept
{
    *this = *this - rhs;
    return *this;
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr u128& u128::operator-=(const __int128 rhs) noexcept
{
    *this = *this - rhs;
    return *this;
}

constexpr u128& u128::operator-=(const unsigned __int128 rhs) noexcept
{
    *this = *this - rhs;
    return *this;
}

#endif

//=====================================
// Multiplication Operator
//=====================================

namespace impl {

BOOST_DECIMAL_FORCE_INLINE constexpr u128 shift_left_32(const std::uint64_t low) noexcept
{
    return {low >> 32, low << 32};
}

#ifdef __x86_64__

BOOST_DECIMAL_FORCE_INLINE constexpr u128 default_mul(const u128 lhs, const u128 rhs) noexcept
{
    #ifndef BOOST_DECIMAL_NO_CONSTEVAL_DETECTION

    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
    {
        return static_cast<u128>(static_cast<unsigned __int128>(lhs) * static_cast<unsigned __int128>(rhs));
    }
    else
    {
        unsigned __int128 new_lhs {};
        unsigned __int128 new_rhs {};

        std::memcpy(&new_lhs, &lhs, sizeof(new_lhs));
        std::memcpy(&new_rhs, &rhs, sizeof(new_rhs));

        const auto res {new_lhs * new_rhs};

        u128 new_res {};
        std::memcpy(&new_res, &res, sizeof(new_res));

        return new_res;
    }

    #else

    return static_cast<u128>(static_cast<unsigned __int128>(lhs) * static_cast<unsigned __int128>(rhs));

    #endif
}

#else

BOOST_DECIMAL_FORCE_INLINE constexpr u128 default_mul(const u128 lhs, const u128 rhs) noexcept
{
    const auto a = static_cast<std::uint64_t>(lhs.low >> 32);
    const auto b = static_cast<std::uint64_t>(lhs.low & UINT32_MAX);
    const auto c = static_cast<std::uint64_t>(rhs.low >> 32);
    const auto d = static_cast<std::uint64_t>(rhs.low & UINT32_MAX);

    u128 result { lhs.high * rhs.low + lhs.low * rhs.high + a * c, b * d };
    result += shift_left_32(a * d) + shift_left_32(b * c);

    return result;
}

#endif // x64

BOOST_DECIMAL_FORCE_INLINE constexpr u128 default_mul(const u128 lhs, const std::uint64_t rhs) noexcept
{
    const auto c = static_cast<std::uint64_t>(rhs >> 32);
    const auto d = static_cast<std::uint64_t>(rhs & UINT32_MAX);
    const auto a = static_cast<std::uint64_t>(lhs.low >> 32);
    const auto b = static_cast<std::uint64_t>(lhs.low & UINT32_MAX);

    u128 result{lhs.high * rhs, b * d};
    result += shift_left_32(a * d) + shift_left_32(b * c);

    return result;
}

constexpr u128 default_signed_mul(const u128 lhs, const std::int64_t rhs) noexcept
{
    const bool isneg {rhs < 0};
    const auto abs_rhs {detail::make_positive_unsigned(rhs)};

    auto res {default_mul(lhs, abs_rhs)};

    if (isneg)
    {
        res.high = ~res.high;
        res.low = ~res.low;

        ++res.low;
        if (res.low == UINT64_C(0))
        {
            ++res.high;
        }
    }

    return res;
}

} // namespace impl

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger> || std::is_same<UnsignedInteger, u128>::value, bool> = true>
constexpr u128 operator*(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return impl::default_mul(lhs, rhs);
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
constexpr u128 operator*(const UnsignedInteger lhs, const u128 rhs) noexcept
{
    return impl::default_mul(rhs, static_cast<std::uint64_t>(lhs));
}

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr u128 operator*(const u128 lhs, const SignedInteger rhs) noexcept
{
    return impl::default_signed_mul(lhs, static_cast<std::int64_t>(rhs));
}

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr u128 operator*(const SignedInteger lhs, const u128 rhs) noexcept
{
    return impl::default_signed_mul(rhs, static_cast<std::int64_t>(lhs));
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr u128 operator*(const u128 lhs, const __int128 rhs) noexcept
{
    return lhs * static_cast<u128>(rhs);
}

constexpr u128 operator*(const __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) * rhs;
}

constexpr u128 operator*(const u128 lhs, const unsigned __int128 rhs) noexcept
{
    return lhs * static_cast<u128>(rhs);
}

constexpr u128 operator*(const unsigned __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) * rhs;
}

#endif // BOOST_DECIMAL_HAS_INT128

//=====================================
// Compound Multiplication Operator
//=====================================

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool>>
constexpr u128& u128::operator*=(const SignedInteger rhs) noexcept
{
    *this = *this * rhs;
    return *this;
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool>>
constexpr u128& u128::operator*=(const UnsignedInteger rhs) noexcept
{
    *this = *this * rhs;
    return *this;
}

constexpr u128& u128::operator*=(const u128 rhs) noexcept
{
    *this = *this * rhs;
    return *this;
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr u128& u128::operator*=(const __int128 rhs) noexcept
{
    *this = *this * rhs;
    return *this;
}

constexpr u128& u128::operator*=(const unsigned __int128 rhs) noexcept
{
    *this = *this * rhs;
    return *this;
}

#endif // BOOST_DECIMAL_HAS_INT128

//=====================================
// Division Operator
//=====================================

namespace impl {

using wide_integer_uint128 = ::boost::decimal::math::wide_integer::uint128_t;

constexpr auto u128_to_wide_integer(const u128& src) -> wide_integer_uint128
{
    wide_integer_uint128 dst { };

    using local_limb_type = typename wide_integer_uint128::limb_type;

    static_assert(sizeof(local_limb_type) == static_cast<std::size_t>(UINT8_C(4)) && std::is_same<local_limb_type, std::uint32_t>::value, "Error: Configuration of external wide-integer limbs not OK");

    auto rep = dst.representation();

    rep[0] = static_cast<local_limb_type>(src.low);
    rep[1] = static_cast<local_limb_type>(src.low >> 32U);
    rep[2] = static_cast<local_limb_type>(src.high);
    rep[3] = static_cast<local_limb_type>(src.high >> 32U);

    return dst;
}

constexpr auto wide_integer_to_u128(const wide_integer_uint128& src) -> u128
{
    u128 dst {};

    const auto rep = src.crepresentation();

    dst.low = static_cast<std::uint64_t>(rep[0]) | (static_cast<std::uint64_t>(rep[1]) << 32);
    dst.high = static_cast<std::uint64_t>(rep[2]) | (static_cast<std::uint64_t>(rep[3]) << 32U);

    return dst;
}

BOOST_DECIMAL_FORCE_INLINE constexpr void div_mod_impl(const u128& lhs, const u128& rhs, u128& quotient, u128& remainder) noexcept
{
    // Mash-Up: Use Knuth long-division from wide-integer (requires limb-conversions on input/output).

    auto lhs_wide = u128_to_wide_integer(lhs);

    wide_integer_uint128 rem_wide { };

    lhs_wide.eval_divide_knuth(u128_to_wide_integer(rhs), rem_wide);

    remainder = wide_integer_to_u128(rem_wide);
    quotient  = wide_integer_to_u128(lhs_wide);
}

#ifdef __aarch64__

// This is unconditionally better on ARM64
BOOST_DECIMAL_FORCE_INLINE constexpr u128 default_div(const u128 lhs, const std::uint64_t rhs) noexcept
{
    return static_cast<u128>(static_cast<unsigned __int128>(lhs) / rhs);
}

#else

BOOST_DECIMAL_FORCE_INLINE constexpr u128 default_div(const u128 lhs, const std::uint64_t rhs) noexcept
{
    #ifndef BOOST_DECIMAL_NO_CONSTEVAL_DETECTION

    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(rhs))
    {
        return static_cast<u128>(static_cast<unsigned __int128>(lhs) / rhs);
    }
    else if (lhs.high != 0)
    {
        // TODO(mborland): Can we abbreviate Knuth division for this case?
        u128 quotient {};
        u128 remainder {};
        impl::div_mod_impl(lhs, u128{0, rhs}, quotient, remainder);
        return quotient;
    }
    else
    {
        // Windows traps on division by 0
        #ifdef _WIN32
        if (rhs.low != 0)
        {
            return { 0, lhs.low / rhs };
        }
        else
        {
            return { 0, 0 };
        }
        #else

        return {0, lhs.low / rhs};

        #endif
    }

    #else

    return static_cast<u128>(static_cast<unsigned __int128>(lhs) / rhs);

    #endif
}

#endif

}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
constexpr u128 operator/(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return impl::default_div(lhs, static_cast<std::uint64_t>(rhs));
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
constexpr u128 operator/(const UnsignedInteger lhs, const u128 rhs) noexcept
{
    if (rhs.high != 0)
    {
        return {0, 0};
    }
    else
    {
        return {0, lhs / rhs.low};
    }
}

constexpr u128 operator/(const u128 lhs, const u128 rhs) noexcept
{
    // On ARM64 this is unconditionally better
    // On x64 this is only better when both lhs and rhs are two word numbers
    #ifdef __aarch64__

    return static_cast<u128>(static_cast<unsigned __int128>(lhs) / static_cast<unsigned __int128>(rhs));

    #else

    BOOST_DECIMAL_ASSERT_MSG(rhs != 0, "Division by zero");

    if (lhs.high != 0 && rhs.high != 0)
    {
        #ifdef BOOST_DECIMAL_HAS_INT128

        return static_cast<u128>(static_cast<unsigned __int128>(lhs) / static_cast<unsigned __int128>(rhs));

        #else

        u128 quotient {};
        u128 remainder {};
        impl::div_mod_impl(lhs, rhs, quotient, remainder);
        return quotient;

        #endif
    }
    else if (rhs.high == 0)
    {
        // Becomes u128 / std::uint64_t abbreviated division
        return impl::default_div(lhs, rhs.low);
    }
    else
    {
        // This would imply rhs.high != 0 and lhs.high == 0 which is always 0
        return {0, 0};
    }

    #endif
}

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr u128 operator/(const u128 lhs, const SignedInteger rhs) noexcept
{
    return rhs > 0 ? lhs / static_cast<std::uint64_t>(rhs) : lhs / static_cast<u128>(rhs);
}

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr u128 operator/(const SignedInteger lhs, const u128 rhs) noexcept
{
    return lhs > 0 ? static_cast<std::uint64_t>(lhs) / rhs : static_cast<u128>(lhs) / rhs;
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr u128 operator/(const u128 lhs, const __int128 rhs) noexcept
{
    return lhs / static_cast<u128>(rhs);
}

constexpr u128 operator/(const __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) / rhs;
}

constexpr u128 operator/(const u128 lhs, const unsigned __int128 rhs) noexcept
{
    return lhs / static_cast<u128>(rhs);
}

constexpr u128 operator/(const unsigned __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) / rhs;
}

#endif // BOOST_DECIMAL_HAS_INT128

//=====================================
// Compound Division Operator
//=====================================

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool>>
constexpr u128& u128::operator/=(const SignedInteger rhs) noexcept
{
    *this = *this / rhs;
    return *this;
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool>>
constexpr u128& u128::operator/=(const UnsignedInteger rhs) noexcept
{
    *this = *this / rhs;
    return *this;
}

constexpr u128& u128::operator/=(const u128 rhs) noexcept
{
    *this = *this / rhs;
    return *this;
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr u128& u128::operator/=(const __int128 rhs) noexcept
{
    *this = *this / rhs;
    return *this;
}

constexpr u128& u128::operator/=(const unsigned __int128 rhs) noexcept
{
    *this = *this / rhs;
    return *this;
}

#endif // BOOST_DECIMAL_HAS_INT128

} // namespace detail
} // namespace decimal
} // namespace boost

// Non-standard libraries may add specializations for library-provided types
namespace std {

template <>
struct numeric_limits<boost::decimal::detail::u128>
{
    // Member constants
    static constexpr bool is_specialized = true;
    static constexpr bool is_signed = false;
    static constexpr bool is_integer = true;
    static constexpr bool is_exact = true;
    static constexpr bool has_infinity = false;
    static constexpr bool has_quiet_NaN = false;
    static constexpr bool has_signaling_NaN = false;

    // These members were deprecated in C++23
    #if ((!defined(_MSC_VER) && (__cplusplus <= 202002L)) || (defined(_MSC_VER) && (_MSVC_LANG <= 202002L)))
    static constexpr std::float_denorm_style has_denorm = std::denorm_absent;
    static constexpr bool has_denorm_loss = false;
    #endif

    static constexpr std::float_round_style round_style = std::round_toward_zero;
    static constexpr bool is_iec559 = false;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo = true;
    static constexpr int digits = 128;
    static constexpr int digits10 = 38;
    static constexpr int max_digits10 = 0;
    static constexpr int radix = 2;
    static constexpr int min_exponent = 0;
    static constexpr int min_exponent10 = 0;
    static constexpr int max_exponent = 0;
    static constexpr int max_exponent10 = 0;
    static constexpr bool traps = std::numeric_limits<std::uint64_t>::traps;
    static constexpr bool tinyness_before = false;

    // Member functions
    static constexpr auto (min)        () -> boost::decimal::detail::u128 { return {0, 0}; }
    static constexpr auto lowest       () -> boost::decimal::detail::u128 { return {0, 0}; }
    static constexpr auto (max)        () -> boost::decimal::detail::u128 { return {UINT64_MAX, UINT64_MAX}; }
    static constexpr auto epsilon      () -> boost::decimal::detail::u128 { return {0, 0}; }
    static constexpr auto round_error  () -> boost::decimal::detail::u128 { return {0, 0}; }
    static constexpr auto infinity     () -> boost::decimal::detail::u128 { return {0, 0}; }
    static constexpr auto quiet_NaN    () -> boost::decimal::detail::u128 { return {0, 0}; }
    static constexpr auto signaling_NaN() -> boost::decimal::detail::u128 { return {0, 0}; }
    static constexpr auto denorm_min   () -> boost::decimal::detail::u128 { return {0, 0}; }
};

} // namespace std

#endif // BOOST_DECIMAL_DETAIL_U128_HPP
