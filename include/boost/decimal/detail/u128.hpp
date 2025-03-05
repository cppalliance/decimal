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

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger> || std::is_same<UnsignedInteger, u128>::value, bool> = true>
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
template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
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

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
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
template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
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

constexpr u128 default_mul(const u128 lhs, const u128 rhs) noexcept
{
    const auto a = static_cast<std::uint64_t>(lhs.low >> 32);
    const auto b = static_cast<std::uint64_t>(lhs.low & UINT32_MAX);
    const auto c = static_cast<std::uint64_t>(rhs.low >> 32);
    const auto d = static_cast<std::uint64_t>(rhs.low & UINT32_MAX);

    u128 result { lhs.high * rhs.low + lhs.low * rhs.high + a * c, b * d };
    result += u128{0, a * d} << 32;
    result += u128{0, b * c} << 32;

    return result;
}

constexpr u128 default_mul(const u128 lhs, const std::uint64_t rhs) noexcept
{
    const auto c = static_cast<std::uint64_t>(rhs >> 32);
    const auto d = static_cast<std::uint64_t>(rhs & UINT32_MAX);
    const auto a = static_cast<std::uint64_t>(lhs.low >> 32);
    const auto b = static_cast<std::uint64_t>(lhs.low & UINT32_MAX);

    u128 result{lhs.high * rhs, b * d};
    result += u128{0, a * d} << 32;
    result += u128{0, b * c} << 32;

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

#if defined(__aarch64__) || defined(_M_ARM64)

BOOST_DECIMAL_FORCE_INLINE u128 arm_asm_mul(const u128 lhs, const u128 rhs) noexcept
{
    u128 result;

    // Does not compensate or track overflow of the high word
    __asm__ volatile(
        // Multiply lhs.low * rhs.low
        "mul %[result_low], %[lhs_low], %[rhs_low]\n"       // result.low = lhs.low * rhs.low (low 64 bits)
        "umulh %[result_high], %[lhs_low], %[rhs_low]\n"    // result.high = lhs.low * rhs.low (high 64 bits)

        // Multiply lhs.high * rhs.low and add to result.high
        "mul x8, %[lhs_high], %[rhs_low]\n"                 // x8 = lhs.high * rhs.low
        "adds %[result_high], %[result_high], x8\n"         // result.high += x8 (with carry)

        // Multiply lhs.low * rhs.high and add to result.high
        "mul x8, %[lhs_low], %[rhs_high]\n"                 // x8 = lhs.low * rhs.high
        "adds %[result_high], %[result_high], x8\n"         // result.high += x8 (with carry)

        // Output operands
        : [result_low] "=&r" (result.low), [result_high] "=&r" (result.high)
        // Input operands
        : [lhs_low] "r" (lhs.low), [lhs_high] "r" (lhs.high),
          [rhs_low] "r" (rhs.low), [rhs_high] "r" (rhs.high)
        // Clobbered registers
        : "x8", "cc"
    );

    return result;
}

BOOST_DECIMAL_FORCE_INLINE u128 arm_asm_mul(const u128 lhs, const std::uint64_t rhs) noexcept
{
    u128 result;

    // Does not compensate or track overflow of the high word
    __asm__ volatile(
        // Multiply lhs.low * rhs
        "mul %[result_low], %[lhs_low], %[rhs]\n"       // result.low = lhs.low * rhs (low 64 bits)
        "umulh %[result_high], %[lhs_low], %[rhs]\n"    // result.high = lhs.low * rhs (high 64 bits)

        // Multiply lhs.high * rhs and add to result.high
        "mul x8, %[lhs_high], %[rhs]\n"                 // x8 = lhs.high * rhs
        "adds %[result_high], %[result_high], x8\n"     // result.high += x8 (with carry)

        // Output operands
        : [result_low] "=&r" (result.low), [result_high] "=&r" (result.high)
        // Input operands
        : [lhs_low] "r" (lhs.low), [lhs_high] "r" (lhs.high),
          [rhs] "r" (rhs)
        // Clobbered registers
        : "x8", "cc"
    );

    return result;
}

BOOST_DECIMAL_FORCE_INLINE u128 arm_asm_signed_mul(const u128 lhs, const std::int64_t rhs) noexcept
{
    u128 result;

    __asm__ volatile(
        // Compute abs(rhs) and save sign bit
        "mov x9, %[rhs]\n"                     // x9 = rhs
        "asr x10, x9, #63\n"                   // x10 = sign extension (0 if rhs >= 0, -1 if rhs < 0)
        "eor x9, x9, x10\n"                    // Flip bits if negative
        "sub x9, x9, x10\n"                    // Add 1 if negative (complete two's complement)

        // Perform unsigned multiplication
        "mul %[result_low], %[lhs_low], x9\n"     // result.low = lhs.low * abs(rhs)
        "umulh %[result_high], %[lhs_low], x9\n"  // result.high = high part of lhs.low * abs(rhs)
        "mul x8, %[lhs_high], x9\n"               // x8 = lhs.high * abs(rhs)
        "adds %[result_high], %[result_high], x8\n" // result.high += x8

        // Conditionally negate the result based on sign bit
        "eor %[result_low], %[result_low], x10\n"    // Flip bits if rhs < 0
        "eor %[result_high], %[result_high], x10\n"  // Flip bits if rhs < 0

        // Extract just 1 or 0 from the sign mask for addition
        "and x8, x10, #1\n"                         // x8 = 1 if rhs < 0, 0 otherwise
        "adds %[result_low], %[result_low], x8\n"   // Add 1 if rhs < 0
        "adc %[result_high], %[result_high], xzr\n" // Add carry

        : [result_low] "=&r" (result.low), [result_high] "=&r" (result.high)
        : [lhs_low] "r" (lhs.low), [lhs_high] "r" (lhs.high),
          [rhs] "r" (rhs)
        : "x8", "x9", "x10", "cc"
    );

    return result;
}

#elif defined(BOOST_DECIMAL_HAS_X64_INTRINSICS)

BOOST_DECIMAL_FORCE_INLINE u128 x64_mul(const u128 lhs, const u128 rhs) noexcept
{
    u128 result;

    __asm__ volatile(
        // Multiply lhs.low * rhs.low
        "movq %[lhs_low], %%rax\n"
        "mulq %[rhs_low]\n"          // rdx:rax = lhs.low * rhs.low
        "movq %%rax, %[result_low]\n"
        "movq %%rdx, %%r8\n"         // Save high part in r8

        // Multiply lhs.high * rhs.low and add to r8
        "movq %[lhs_high], %%rax\n"
        "mulq %[rhs_low]\n"          // rdx:rax = lhs.high * rhs.low
        "addq %%rax, %%r8\n"
        "adcq $0, %%rdx\n"           // Add with carry

        // Multiply lhs.low * rhs.high and add to r8:rdx
        "movq %[lhs_low], %%rax\n"
        "mulq %[rhs_high]\n"         // rdx:rax = lhs.low * rhs.high
        "addq %%rax, %%r8\n"
        "movq %%r8, %[result_high]\n"

        : [result_low] "=m" (result.low), [result_high] "=m" (result.high)
        : [lhs_low] "m" (lhs.low), [lhs_high] "m" (lhs.high),
          [rhs_low] "m" (rhs.low), [rhs_high] "m" (rhs.high)
        : "rax", "rdx", "r8", "cc"
    );

    return result;
}

BOOST_DECIMAL_FORCE_INLINE u128 x64_mul(const u128 lhs, const std::uint64_t rhs) noexcept
{
    u128 result;

    __asm__ volatile(
        // Multiply lhs.low * rhs
        "movq %[lhs_low], %%rax\n"
        "mulq %[rhs]\n"              // rdx:rax = lhs.low * rhs
        "movq %%rax, %[result_low]\n"
        "movq %%rdx, %%r8\n"         // Save high part in r8

        // Multiply lhs.high * rhs and add to r8
        "movq %[lhs_high], %%rax\n"
        "mulq %[rhs]\n"              // rdx:rax = lhs.high * rhs
        "addq %%rax, %%r8\n"
        "movq %%r8, %[result_high]\n"

        : [result_low] "=m" (result.low), [result_high] "=m" (result.high)
        : [lhs_low] "m" (lhs.low), [lhs_high] "m" (lhs.high),
          [rhs] "m" (rhs)
        : "rax", "rdx", "r8", "cc"
    );

    return result;
}

BOOST_DECIMAL_FORCE_INLINE u128 x64_signed_mul(const u128 lhs, const std::int64_t rhs) noexcept
{
    u128 result;

    __asm__ volatile(
        // Prepare abs(rhs) and save sign
        "movq %[rhs], %%r10\n"              // r10 = rhs
        "movq %%r10, %%r11\n"               // r11 = rhs (save original)
        "sarq $63, %%r11\n"                 // r11 = 0 if rhs >= 0, r11 = -1 if rhs < 0
        "xorq %%r11, %%r10\n"               // Conditionally invert bits
        "subq %%r11, %%r10\n"               // Add 1 if negative (two's complement)

        // Multiply lhs.low * abs(rhs)
        "movq %[lhs_low], %%rax\n"
        "mulq %%r10\n"                      // rdx:rax = lhs.low * abs(rhs)
        "movq %%rax, %%r8\n"                // r8 = low result
        "movq %%rdx, %%r9\n"                // r9 = high part

        // Multiply lhs.high * abs(rhs)
        "movq %[lhs_high], %%rax\n"
        "mulq %%r10\n"                      // rdx:rax = lhs.high * abs(rhs)
        "addq %%rax, %%r9\n"                // r9 += rax

        // Conditionally negate the result
        "xorq %%r11, %%r8\n"                // Conditionally invert bits of low part
        "xorq %%r11, %%r9\n"                // Conditionally invert bits of high part
        "subq %%r11, %%r8\n"                // Add 1 if needed (low part)
        "sbbq %%r11, %%r9\n"                // Subtract borrow (high part)

        // Store the result
        "movq %%r8, %[result_low]\n"
        "movq %%r9, %[result_high]\n"

        : [result_low] "=&r" (result.low), [result_high] "=&r" (result.high)
        : [lhs_low] "rm" (lhs.low), [lhs_high] "rm" (lhs.high),
          [rhs] "rm" (rhs)
        : "rax", "rdx", "r8", "r9", "r10", "r11", "cc"
    );

    return result;
}

#elif defined(BOOST_DECIMAL_HAS_MSVC_64BIT_INTRINSICS)

BOOST_DECIMAL_FORCE_INLINE u128 x64_mul(const u128 lhs, const u128 rhs) noexcept
{
    u128 result;

    // Multiply lhs.low * rhs.low (full 128-bit result)
    result.low = _umul128(lhs.low, rhs.low, &result.high);

    // Add lhs.high * rhs.low to result.high
    unsigned char carry = BOOST_DECIMAL_ADD_CARRY(0, result.high, lhs.high * rhs.low, &result.high);

    // Add lhs.low * rhs.high to result.high
    BOOST_DECIMAL_ADD_CARRY(carry, result.high, lhs.low * rhs.high, &result.high);

    return result;
}

BOOST_DECIMAL_FORCE_INLINE u128 x64_mul(const u128 lhs, const std::uint64_t rhs) noexcept
{
    u128 result;

    // Multiply lhs.low * rhs (full 128-bit result)
    result.low = _umul128(lhs.low, rhs, &result.high);

    // Add lhs.high * rhs to result.high
    BOOST_DECIMAL_ADD_CARRY(0, result.high, lhs.high * rhs, &result.high);

    return result;
}

BOOST_DECIMAL_FORCE_INLINE u128 x64_signed_mul(const u128 lhs, const std::int64_t rhs) noexcept
{
    const bool is_negative = rhs < 0;

    const std::uint64_t abs_rhs = is_negative ? static_cast<std::uint64_t>(-rhs) : static_cast<std::uint64_t>(rhs);

    u128 result;
    result.low = _umul128(lhs.low, abs_rhs, &result.high);
    BOOST_DECIMAL_ADD_CARRY(0, result.high, lhs.high * abs_rhs, &result.high);

    if (is_negative)
    {
        // Two's complement negation
        result.high = ~result.high;
        result.low = ~result.low;

        // Add 1 with carry handling
        unsigned char carry = BOOST_DECIMAL_ADD_CARRY(0, result.low, 1ULL, &result.low);
        BOOST_DECIMAL_ADD_CARRY(carry, result.high, 0ULL, &result.high);
    }

    return result;
}

#endif

} // namespace impl

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger> || std::is_same<UnsignedInteger, u128>::value, bool> = true>
constexpr u128 operator*(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    #ifndef BOOST_DECIMAL_NO_CONSTEVAL_DETECTION

    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
    {
        return impl::default_mul(lhs, rhs);
    }
    else
    {
        #if defined(__aarch64__) || defined(_M_ARM64)

        return impl::arm_asm_mul(lhs, rhs);

        #elif defined(BOOST_DECIMAL_HAS_X64_INTRINSICS) || defined(BOOST_DECIMAL_HAS_MSVC_64BIT_INTRINSICS)

        return impl::x64_mul(lhs, rhs);

        #else

        return impl::default_mul(lhs, rhs);

        #endif
    }

    #else

    return impl::default_mul(lhs, rhs);

    #endif
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
constexpr u128 operator*(const UnsignedInteger lhs, const u128 rhs) noexcept
{
    #ifndef BOOST_DECIMAL_NO_CONSTEVAL_DETECTION

    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
    {
        return impl::default_mul(rhs, static_cast<std::uint64_t>(lhs));
    }
    else
    {
        #if defined(__aarch64__) || defined(_M_ARM64)

        return impl::arm_asm_mul(rhs, static_cast<std::uint64_t>(lhs));

        #elif defined(BOOST_DECIMAL_HAS_X64_INTRINSICS) || defined(BOOST_DECIMAL_HAS_MSVC_64BIT_INTRINSICS)

        return impl::x64_mul(rhs, static_cast<std::uint64_t>(lhs));

        #else

        return impl::default_mul(rhs, static_cast<std::uint64_t>(lhs));

        #endif
    }

    #else

    return impl::default_mul(rhs, static_cast<std::uint64_t>(lhs));

    #endif
}

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr u128 operator*(const u128 lhs, const SignedInteger rhs) noexcept
{
    #ifndef BOOST_DECIMAL_NO_CONSTEVAL_DETECTION

    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
    {
        return impl::default_signed_mul(lhs, static_cast<std::int64_t>(rhs));
    }
    else
    {
        #if defined(__aarch64__) || defined(_M_ARM64)

        return impl::arm_asm_signed_mul(lhs, static_cast<std::int64_t>(rhs));

        #elif defined(BOOST_DECIMAL_HAS_X64_INTRINSICS) || defined(BOOST_DECIMAL_HAS_MSVC_64BIT_INTRINSICS)

        return impl::x64_signed_mul(lhs, static_cast<std::int64_t>(rhs));

        #else

        return impl::default_signed_mul(lhs, static_cast<std::int64_t>(rhs));

        #endif
    }

    #else

    return impl::default_signed_mul(lhs, static_cast<std::int64_t>(rhs));

    #endif
}

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr u128 operator*(const SignedInteger lhs, const u128 rhs) noexcept
{
    #ifndef BOOST_DECIMAL_NO_CONSTEVAL_DETECTION

    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(lhs))
    {
        return impl::default_signed_mul(rhs, static_cast<std::int64_t>(lhs));
    }
    else
    {
        #if defined(__aarch64__) || defined(_M_ARM64)

        return impl::arm_asm_signed_mul(rhs, static_cast<std::int64_t>(lhs));

        #elif defined(BOOST_DECIMAL_HAS_X64_INTRINSICS) || defined(BOOST_DECIMAL_HAS_MSVC_64BIT_INTRINSICS)

        return impl::x64_signed_mul(rhs, static_cast<std::int64_t>(lhs));

        #else

        return impl::default_signed_mul(rhs, static_cast<std::int64_t>(lhs));

        #endif
    }

    #else

    return impl::default_signed_mul(rhs, static_cast<std::int64_t>(lhs));

    #endif
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

} // namespace detail
} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_U128_HPP
