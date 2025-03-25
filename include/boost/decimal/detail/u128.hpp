// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// If the architecture (e.g. 32-bit x86) does not have __int128 we need to emulate it

#ifndef BOOST_DECIMAL_DETAIL_U128_HPP
#define BOOST_DECIMAL_DETAIL_U128_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/locale_conversion.hpp>
#include <boost/decimal/detail/strlen.hpp>

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

// This is used frequently enough that I'd rather not have this spaghetti everytime
// These older clang and GCC versions return incorrect numerical results or sometimes hang
#define BOOST_DECIMAL_OLD_NON_GNU_COMPILER ((defined(__clang__) && __clang_major__ < 11) || \
                                            (defined(__GNUC__) && !defined(__clang__) && \
                                            (__GNUC__ < 10 || (defined(__STRICT_ANSI__) && __GNUC__ >= 10))))

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

    #ifdef BOOST_DECIMAL_ENDIAN_BIG_BYTE
    #  if defined(__GNUC__)
    #    pragma GCC diagnostic push
    #    pragma GCC diagnostic ignored "-Wreorder"
    #    define BOOST_DECIMAL_PUSHED_BIG_WARNING
    #  endif
    #endif // Big endian

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

    #ifdef BOOST_DECIMAL_PUSHED_BIG_WARNING
    #  pragma GCC diagnostic pop
    #  undef BOOST_DECIMAL_PUSHED_BIG_WARNING
    #endif

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

    // Compound Modulo Operators
    template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
    constexpr u128& operator%=(SignedInteger rhs) noexcept;

    template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
    constexpr u128& operator%=(UnsignedInteger rhs) noexcept;

    constexpr u128& operator%=(u128 rhs) noexcept;

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr u128& operator%=(__int128 rhs) noexcept;
    constexpr u128& operator%=(unsigned __int128 rhs) noexcept;
    #endif // BOOST_DECIMAL_HAS_INT128

    // Compound And
    template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
    constexpr u128& operator&=(SignedInteger rhs) noexcept;

    template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
    constexpr u128& operator&=(UnsignedInteger rhs) noexcept;

    constexpr u128& operator&=(u128 rhs) noexcept;

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr u128& operator&=(__int128 rhs) noexcept;
    constexpr u128& operator&=(unsigned __int128 rhs) noexcept;
    #endif

    // Compound Or
    template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
    constexpr u128& operator|=(SignedInteger rhs) noexcept;

    template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
    constexpr u128& operator|=(UnsignedInteger rhs) noexcept;

    constexpr u128& operator|=(u128 rhs) noexcept;

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr u128& operator|=(__int128 rhs) noexcept;
    constexpr u128& operator|=(unsigned __int128 rhs) noexcept;
    #endif

    // Compound XOR
    template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
    constexpr u128& operator^=(SignedInteger rhs) noexcept;

    template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
    constexpr u128& operator^=(UnsignedInteger rhs) noexcept;

    constexpr u128& operator^=(u128 rhs) noexcept;

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr u128& operator^=(__int128 rhs) noexcept;
    constexpr u128& operator^=(unsigned __int128 rhs) noexcept;
    #endif

    // Compound Left Shift
    template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
    constexpr u128& operator<<=(SignedInteger rhs) noexcept;

    template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
    constexpr u128& operator<<=(UnsignedInteger rhs) noexcept;

    constexpr u128& operator<<=(u128 rhs) noexcept;

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr u128& operator<<=(__int128 rhs) noexcept;
    constexpr u128& operator<<=(unsigned __int128 rhs) noexcept;
    #endif

    // Compound Right Shift
    template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
    constexpr u128& operator>>=(SignedInteger rhs) noexcept;

    template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
    constexpr u128& operator>>=(UnsignedInteger rhs) noexcept;

    constexpr u128& operator>>=(u128 rhs) noexcept;

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr u128& operator>>=(__int128 rhs) noexcept;
    constexpr u128& operator>>=(unsigned __int128 rhs) noexcept;
    #endif
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

//=====================================
// Float Conversion Operators
//=====================================

// The most correct way to do this would be std::ldexp(static_cast<T>(high), 64) + static_cast<T>(low);
// Since std::ldexp is not constexpr until C++23 we can work around this by multiplying the high word
// by 0xFFFFFFFF in order to generally replicate what ldexp is doing in the constexpr context.
// We also avoid pulling in <quadmath.h> for the __float128 case where we would need ldexpq
namespace impl {

template <typename T>
static constexpr T offset_value_v = static_cast<T>(std::numeric_limits<std::uint64_t>::max());

}

constexpr u128::operator float() const noexcept
{
    return static_cast<float>(high) * impl::offset_value_v<float> + static_cast<float>(low);
}

constexpr u128::operator double() const noexcept
{
    return static_cast<double>(high) * impl::offset_value_v<double> + static_cast<double>(low);
}

constexpr u128::operator long double() const noexcept
{
    return static_cast<long double>(high) * impl::offset_value_v<long double> + static_cast<long double>(low);
}

#ifdef BOOST_DECIMAL_HAS_FLOAT128

constexpr u128::operator __float128() const noexcept
{
    return static_cast<__float128>(high) * impl::offset_value_v<__float128> + static_cast<__float128>(low);
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

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wsign-conversion"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

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
// Compound OR Operator
//=====================================

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool>>
constexpr u128& u128::operator|=(const SignedInteger rhs) noexcept
{
    *this = *this | rhs;
    return *this;
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool>>
constexpr u128& u128::operator|=(const UnsignedInteger rhs) noexcept
{
    *this = *this | rhs;
    return *this;
}

constexpr u128& u128::operator|=(const u128 rhs) noexcept
{
    *this = *this | rhs;
    return *this;
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr u128& u128::operator|=(const __int128 rhs) noexcept
{
    *this = *this | rhs;
    return *this;
}

constexpr u128& u128::operator|=(const unsigned __int128 rhs) noexcept
{
    *this = *this | rhs;
    return *this;
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
// Compound And Operator
//=====================================

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool>>
constexpr u128& u128::operator&=(const SignedInteger rhs) noexcept
{
    *this = *this & rhs;
    return *this;
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool>>
constexpr u128& u128::operator&=(const UnsignedInteger rhs) noexcept
{
    *this = *this & rhs;
    return *this;
}

constexpr u128& u128::operator&=(const u128 rhs) noexcept
{
    *this = *this & rhs;
    return *this;
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr u128& u128::operator&=(const __int128 rhs) noexcept
{
    *this = *this & rhs;
    return *this;
}

constexpr u128& u128::operator&=(const unsigned __int128 rhs) noexcept
{
    *this = *this & rhs;
    return *this;
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
// Compound XOR Operator
//=====================================

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool>>
constexpr u128& u128::operator^=(const SignedInteger rhs) noexcept
{
    *this = *this ^ rhs;
    return *this;
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool>>
constexpr u128& u128::operator^=(const UnsignedInteger rhs) noexcept
{
    *this = *this ^ rhs;
    return *this;
}

constexpr u128& u128::operator^=(const u128 rhs) noexcept
{
    *this = *this ^ rhs;
    return *this;
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr u128& u128::operator^=(const __int128 rhs) noexcept
{
    *this = *this ^ rhs;
    return *this;
}

constexpr u128& u128::operator^=(const unsigned __int128 rhs) noexcept
{
    *this = *this ^ rhs;
    return *this;
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
// Compound Left Shift Operator
//=====================================

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool>>
constexpr u128& u128::operator<<=(const SignedInteger rhs) noexcept
{
    *this = *this << rhs;
    return *this;
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool>>
constexpr u128& u128::operator<<=(const UnsignedInteger rhs) noexcept
{
    *this = *this << rhs;
    return *this;
}

constexpr u128& u128::operator<<=(const u128 rhs) noexcept
{
    *this = *this << rhs;
    return *this;
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr u128& u128::operator<<=(const __int128 rhs) noexcept
{
    *this = *this << rhs;
    return *this;
}

constexpr u128& u128::operator<<=(const unsigned __int128 rhs) noexcept
{
    *this = *this << rhs;
    return *this;
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
// Compound Right Shift Operator
//=====================================

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool>>
constexpr u128& u128::operator>>=(const SignedInteger rhs) noexcept
{
    *this = *this >> rhs;
    return *this;
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool>>
constexpr u128& u128::operator>>=(const UnsignedInteger rhs) noexcept
{
    *this = *this >> rhs;
    return *this;
}

constexpr u128& u128::operator>>=(const u128 rhs) noexcept
{
    *this = *this >> rhs;
    return *this;
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr u128& u128::operator>>=(const __int128 rhs) noexcept
{
    *this = *this >> rhs;
    return *this;
}

constexpr u128& u128::operator>>=(const unsigned __int128 rhs) noexcept
{
    *this = *this >> rhs;
    return *this;
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
    #if defined(__x86_64__) && defined(BOOST_DECIMAL_HAS_INT128) && !defined(BOOST_DECIMAL_OLD_NON_GNU_COMPILER)

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
        return impl::default_add(lhs, static_cast<std::uint64_t>(-rhs));
    }
    else
    {
        return impl::default_sub(lhs, static_cast<std::uint64_t>(rhs));
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

#if defined(__GNUC__) && __GNUC__ >= 8
#  pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif

namespace impl {

BOOST_DECIMAL_FORCE_INLINE constexpr u128 shift_left_32(const std::uint64_t low) noexcept
{
    return {low >> 32, low << 32};
}

#if (defined(__x86_64__) || (defined(__aarch64__) && !defined(__APPLE__)) || (defined(__ppc64__) || defined(__PPC64__) || defined(__ppc64le__) || defined(__PPC64LE__))) && defined(BOOST_DECIMAL_HAS_INT128) \
    && !defined(BOOST_DECIMAL_OLD_NON_GNU_COMPILER)

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

    auto& rep = dst.representation();

    rep[0] = static_cast<local_limb_type>(src.low);
    rep[1] = static_cast<local_limb_type>(src.low >> 32U);
    rep[2] = static_cast<local_limb_type>(src.high);
    rep[3] = static_cast<local_limb_type>(src.high >> 32U);

    return dst;
}

constexpr auto wide_integer_to_u128(const wide_integer_uint128& src) -> u128
{
    u128 dst {};

    const auto& rep = src.crepresentation();

    dst.low = static_cast<std::uint64_t>(rep[0]) | (static_cast<std::uint64_t>(rep[1]) << 32);
    dst.high = static_cast<std::uint64_t>(rep[2]) | (static_cast<std::uint64_t>(rep[3]) << 32U);

    return dst;
}

// Adapted from ckormanyos/wide-integer
//
// Use Knuth's long division algorithm.
// The loop-ordering of indices in Knuth's original
// algorithm has been reversed due to the data format
// used here. Several optimizations and combinations
// of logic have been carried out in the source code.
//
// See also:
// D.E. Knuth, "The Art of Computer Programming, Volume 2:
// Seminumerical Algorithms", Addison-Wesley (1998),
// Section 4.3.1 Algorithm D and Exercise 16.

template <std::size_t u_size, std::size_t v_size, std::size_t q_size>
constexpr void divide_knuth_core(std::uint32_t (&u)[u_size], 
                                 std::uint32_t (&v)[v_size], 
                                 std::uint32_t (&q)[q_size]) noexcept
{
    const auto d { u_size - v_size - 1 };

    for (std::size_t j = d; j >= 0; --j)
    {
        const auto next_digits{ static_cast<std::uint64_t>(u[j + v_size] << 32) | static_cast<std::uint64_t>(u[j + v_size - 1]) };
        auto q_hat{ next_digits / v[v_size - 1] };
        auto r_hat{ next_digits % v[v_size - 1] };

        while ((q_hat >> 32) != 0 || ( q_hat * v[v_size - 2] > (r_hat << 32 | u[j + v_size - 2])))
        {
            --q_hat;
            r_hat += v[v_size - 1];
            if (r_hat >> 32 != 0)
            {
                break;
            }
        }

        std::int64_t k{};
        std::int64_t t{};

        for (std::size_t i = 0; i < v_size; ++i)
        {
            const auto product{ static_cast<std::uint64_t>(q_hat) * static_cast<std::uint64_t>(v[i]) };
            t = u[i + j] - k - product;
            u[i + j] = static_cast<std::uint32_t>(t);
            k = static_cast<std::int64_t>(product >> 32) - (t >> 32);
        }

        t = static_cast<std::int64_t>(u[j + v_size]) - k;
        u[j + v_size] = static_cast<std::uint32_t>(t);
        q[j] = static_cast<std::uint32_t>(q_hat);

        if (t < 0)
        {
            --q[j];
            k = 0;

            for (std::size_t i = 0; i < v_size; ++i)
            {
                t = static_cast<std::int64_t>(u[i + j]) + k + static_cast<std::int64_t>(v[i]);
                u[i + j] = static_cast<std::uint32_t>(t);
                k = t >> 32;
            }
            
            u[j + v_size] += static_cast<std::uint32_t>(k);
        }
    }
}

BOOST_DECIMAL_FORCE_INLINE constexpr void div_mod_impl(const u128& lhs, const std::uint64_t rhs, u128& quotient, u128& remainder) noexcept
{
    // If rhs is greater than 2^32 the result is trivial to find
    if (rhs >= UINT32_MAX)
    {
        #if !defined(BOOST_DECIMAL_NO_CONSTEVAL_DETECTION) && defined(BOOST_DECIMAL_HAS_MSVC_X64_INTRINSICS)
        if (!BOOST_DECIMAL_IS_CONSTANT_EVALUATED(rhs))
        {
            quotient.low = _udiv128(lhs.high, lhs.low, rhs, &remainder.low);
        }
        else
        #endif
        {
            remainder.low = (lhs.high << 32U) | (lhs.low >> 32U);
            auto res = remainder.low / rhs;
            remainder.low = (remainder.low % rhs) << 32 | lhs.low;
            res = (res << 32) | (remainder.low / rhs);
            remainder.low %= rhs;
        }
    }

    // Setup for Knuth Division
    const auto offset { countl_zero(rhs >> 32) };

    std::uint32_t u[5]{};

    if (offset != 0)
    {
        u[0] = static_cast<std::uint32_t>(lhs.low << offset);
        u[1] = static_cast<std::uint32_t>(lhs.low >> (32 - offset));
        u[2] = static_cast<std::uint32_t>(lhs.high << offset) |
               static_cast<std::uint32_t>(lhs.low >> (64 - offset));
        u[3] = static_cast<std::uint32_t>(lhs.high >> (32 - offset));
        u[4] = static_cast<std::uint32_t>(lhs.high >> (64 - offset));
    }
    else
    {
        u[0] = static_cast<std::uint32_t>(lhs.low);
        u[1] = static_cast<std::uint32_t>(lhs.low >> 32);
        u[2] = static_cast<std::uint32_t>(lhs.high);
        u[3] = static_cast<std::uint32_t>(lhs.high >> 32);
    }

    std::uint32_t v[2]
    {
        static_cast<std::uint32_t>(rhs << offset),
        static_cast<std::uint32_t>(rhs >> (32 - offset))
    };

    std::uint32_t q[3]{};

    divide_knuth_core(u, v, q);

    quotient.low = static_cast<std::uint64_t>(q[1] << 32) | q[0];
    quotient.high = q[3];
    remainder.low = static_cast<std::uint64_t>(u[1] << (32 - offset)) | static_cast<std::uint64_t>(u[0] >> offset);
}

BOOST_DECIMAL_FORCE_INLINE constexpr void div_mod_impl(const u128& lhs, const u128& rhs, u128& quotient, u128& remainder) noexcept
{
    // Mash-Up: Use Knuth long-division from wide-integer (requires limb-conversions on input/output).
    if (rhs.high == UINT64_C(0) && rhs.low < UINT64_C(0x100000000))
    {
        const auto rhs32 = static_cast<std::uint32_t>(rhs.low);

        auto current = static_cast<std::uint64_t>(lhs.high >> 32U);
        quotient.high = static_cast<std::uint64_t>(static_cast<std::uint64_t>(static_cast<std::uint32_t>(current / rhs32)) << 32U);
        remainder.low = static_cast<std::uint64_t>(current % rhs32);

        current = static_cast<std::uint64_t>(remainder.low << 32U) | static_cast<std::uint32_t>(lhs.high);
        quotient.high |= static_cast<std::uint32_t>(current / rhs32);
        remainder.low = static_cast<std::uint64_t>(current % rhs32);

        current = static_cast<std::uint64_t>(remainder.low << 32U) | static_cast<std::uint32_t>(lhs.low >> 32U);
        quotient.low = static_cast<std::uint64_t>(static_cast<std::uint64_t>(static_cast<std::uint32_t>(current / rhs32)) << 32U);
        remainder.low = static_cast<std::uint64_t>(current % rhs32);

        current = remainder.low << 32U | static_cast<std::uint32_t>(lhs.low);
        quotient.low |= static_cast<std::uint32_t>(current / rhs32);
        remainder.low = static_cast<std::uint32_t>(current % rhs32);

        remainder.high = UINT64_C(0);
    }
    #ifdef BOOST_DECIMAL_HAS_MSVC_X64_INTRINSICS
    else if (rhs.high == UINT64_C(0))
    {
        // Since low is at least 2^32 the quotient will be less than 2^64
        quotient.low = _udiv128(lhs.high, lhs.low, rhs.low, &remainder.low);
    }
    #endif
    else
    {
        auto lhs_wide = u128_to_wide_integer(lhs);

        wide_integer_uint128 rem_wide { };

        lhs_wide.eval_divide_knuth(u128_to_wide_integer(rhs), rem_wide);

        remainder = wide_integer_to_u128(rem_wide);
        quotient  = wide_integer_to_u128(lhs_wide);
    }
}

#if (defined(__x86_64__) || (defined(__aarch64__) && !defined(__APPLE__)) || (defined(__ppc64__) || defined(__PPC64__) || defined(__ppc64le__) || defined(__PPC64LE__))) && defined(BOOST_DECIMAL_HAS_INT128) \
    && !defined(BOOST_DECIMAL_OLD_NON_GNU_COMPILER)

// This is unconditionally better on ARM64, PPC64LE, and S390X
BOOST_DECIMAL_FORCE_INLINE constexpr u128 default_div(const u128 lhs, const std::uint64_t rhs) noexcept
{
    return static_cast<u128>(static_cast<unsigned __int128>(lhs) / rhs);
}

#elif defined(BOOST_DECIMAL_HAS_INT128)

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
        return {0, lhs.low / rhs};
    }

    #else

    return static_cast<u128>(static_cast<unsigned __int128>(lhs) / rhs);

    #endif
}

#else 

BOOST_DECIMAL_FORCE_INLINE constexpr u128 default_div(const u128 lhs, const std::uint64_t rhs) noexcept
{
    #ifndef BOOST_DECIMAL_NO_CONSTEVAL_DETECTION

    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(rhs))
    {
        u128 quotient{};
        u128 remainder{};
        impl::div_mod_impl(lhs, rhs, quotient, remainder);
        return quotient;
    }
    else if (lhs.high != 0)
    {
        u128 quotient{};
        u128 remainder{};
        impl::div_mod_impl(lhs, rhs, quotient, remainder);
        return quotient;
    }
    else
    {
        if (rhs == 0)
        {
            return { 0, 0 };
        }

        return { 0, lhs.low / rhs };
    }

    #else

    u128 quotient{};
    u128 remainder{};
    impl::div_mod_impl(lhs, u128{ 0, rhs }, quotient, remainder);
    return quotient;

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
    // On ARM64 and PPC64LE this is unconditionally better
    // On x64 this is only better when both lhs and rhs are two word numbers
    #if defined(__aarch64__) || (defined(__ppc64__) || defined(__PPC64__) || defined(__ppc64le__) || defined(__PPC64LE__)) || defined(__s390x__) \
        && !defined(BOOST_DECIMAL_OLD_NON_GNU_COMPILER)

    return static_cast<u128>(static_cast<unsigned __int128>(lhs) / static_cast<unsigned __int128>(rhs));

    #else

    BOOST_DECIMAL_ASSERT_MSG(rhs != 0, "Division by zero");

    // The is best x64 path assuming the user has consteval detection
    #if !defined(BOOST_DECIMAL_NO_CONSTEVAL_DETECTION) && defined(BOOST_DECIMAL_HAS_INT128) && !defined(BOOST_DECIMAL_OLD_NON_GNU_COMPILER)

    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(rhs))
    {
        return static_cast<u128>(static_cast<unsigned __int128>(lhs) / static_cast<unsigned __int128>(rhs));
    }
    else if (lhs.high != 0 && rhs.high != 0)
    {
        unsigned __int128 new_lhs {};
        unsigned __int128 new_rhs {};

        std::memcpy(&new_lhs, &lhs, sizeof(new_lhs));
        std::memcpy(&new_rhs, &rhs, sizeof(new_rhs));

        const auto res {new_lhs / new_rhs};

        u128 new_res {};
        std::memcpy(&new_res, &res, sizeof(new_res));

        return new_res;
    }
    else if (rhs > lhs)
    {
        return {0, 0};
    }
    else
    {
        u128 quotient {};
        u128 remainder {};
        impl::div_mod_impl(lhs, rhs, quotient, remainder);
        return quotient;
    }

    #else

    if (lhs.high != 0 && rhs.high != 0)
    {
        #if defined(BOOST_DECIMAL_HAS_INT128) && !defined(BOOST_DECIMAL_OLD_NON_GNU_COMPILER)

        return static_cast<u128>(static_cast<unsigned __int128>(lhs) / static_cast<unsigned __int128>(rhs));

        #else

        u128 quotient {};
        u128 remainder {};
        impl::div_mod_impl(lhs, rhs, quotient, remainder);
        return quotient;

        #endif
    }
    else if (rhs > lhs)
    {
        // This would imply rhs.high != 0 and lhs.high == 0 which is always 0
        return {0, 0};
    }
    else
    {
        u128 quotient {};
        u128 remainder {};
        impl::div_mod_impl(lhs, rhs, quotient, remainder);
        return quotient;
    }
    
    #endif

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

//=====================================
// Modulo Operator
//=====================================

namespace impl {

#if defined(__aarch64__) || (defined(__ppc64__) || defined(__PPC64__) || defined(__ppc64le__) || defined(__PPC64LE__)) || defined(__s390x__) \
    && !defined(BOOST_DECIMAL_OLD_NON_GNU_COMPILER)

// This is unconditionally better on ARM64, PPC64LE, and S390X
BOOST_DECIMAL_FORCE_INLINE constexpr u128 default_mod(const u128 lhs, const std::uint64_t rhs) noexcept
{
    return static_cast<u128>(static_cast<unsigned __int128>(lhs) % rhs);
}

#elif defined(BOOST_DECIMAL_HAS_INT128)

BOOST_DECIMAL_FORCE_INLINE constexpr u128 default_mod(const u128 lhs, const std::uint64_t rhs) noexcept
{
    #ifndef BOOST_DECIMAL_NO_CONSTEVAL_DETECTION

    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(rhs))
    {
        return static_cast<u128>(static_cast<unsigned __int128>(lhs) % rhs);
    }
    else if (lhs.high != 0)
    {
        // TODO(mborland): Can we abbreviate Knuth division for this case?
        u128 quotient {};
        u128 remainder {};
        impl::div_mod_impl(lhs, u128{0, rhs}, quotient, remainder);
        return remainder;
    }
    else
    {
        return {0, lhs.low % rhs};
    }

    #else

    return static_cast<u128>(static_cast<unsigned __int128>(lhs) % rhs);

    #endif
}

#else

BOOST_DECIMAL_FORCE_INLINE constexpr u128 default_mod(const u128 lhs, const std::uint64_t rhs) noexcept
{
    #ifndef BOOST_DECIMAL_NO_CONSTEVAL_DETECTION

    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(rhs))
    {
        u128 quotient{};
        u128 remainder{};
        impl::div_mod_impl(lhs, u128{ 0, rhs }, quotient, remainder);
        return remainder;
    }
    else if (lhs.high != 0)
    {
        // TODO(mborland): Can we abbreviate Knuth division for this case?
        u128 quotient{};
        u128 remainder{};
        impl::div_mod_impl(lhs, u128{ 0, rhs }, quotient, remainder);
        return remainder;
    }
    else
    {
        if (rhs == 0)
        {
            return { 0, 0 };
        }

        return { 0, lhs.low % rhs };
    }

    #else

    u128 quotient{};
    u128 remainder{};
    impl::div_mod_impl(lhs, u128{ 0, rhs }, quotient, remainder);
    return remainder;

    #endif
}

#endif

} // namespace impl

constexpr u128 operator%(const u128 lhs, const u128 rhs) noexcept
{
    // On ARM64 and PPC64LE this is unconditionally better
    // On x64 this is only better when both lhs and rhs are two word numbers
    #if defined(__aarch64__) || (defined(__ppc64__) || defined(__PPC64__) || defined(__ppc64le__) || defined(__PPC64LE__)) || defined(__s390x__)

    return static_cast<u128>(static_cast<unsigned __int128>(lhs) % static_cast<unsigned __int128>(rhs));

    #else

    BOOST_DECIMAL_ASSERT_MSG(rhs != 0, "Division by zero");

    // The is best x64 path assuming the user has consteval detection
    #if !defined(BOOST_DECIMAL_NO_CONSTEVAL_DETECTION) && defined(BOOST_DECIMAL_HAS_INT128)

    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(rhs))
    {
        return static_cast<u128>(static_cast<unsigned __int128>(lhs) % static_cast<unsigned __int128>(rhs));
    }
    else if (lhs.high != 0 && rhs.high != 0)
    {
        unsigned __int128 new_lhs {};
        unsigned __int128 new_rhs {};

        std::memcpy(&new_lhs, &lhs, sizeof(new_lhs));
        std::memcpy(&new_rhs, &rhs, sizeof(new_rhs));

        const auto res {new_lhs % new_rhs};

        u128 new_res {};
        std::memcpy(&new_res, &res, sizeof(new_res));

        return new_res;
    }
    else if (rhs > lhs)
    {
        return lhs;
    }
    else
    {
        u128 quotient {};
        u128 remainder {};
        impl::div_mod_impl(lhs, rhs, quotient, remainder);
        return remainder;
    }

    #else

    if (lhs.high != 0 && rhs.high != 0)
    {
        #ifdef BOOST_DECIMAL_HAS_INT128

        return static_cast<u128>(static_cast<unsigned __int128>(lhs) % static_cast<unsigned __int128>(rhs));

        #else

        u128 quotient {};
        u128 remainder {};
        impl::div_mod_impl(lhs, rhs, quotient, remainder);
        return remainder;

        #endif
    }
    else if (rhs > lhs)
    {
        return lhs;
    }
    else
    {
        u128 quotient {};
        u128 remainder {};
        impl::div_mod_impl(lhs, rhs, quotient, remainder);
        return remainder;
    }

    #endif

    #endif
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
constexpr u128 operator%(const u128 lhs, const UnsignedInteger rhs) noexcept
{
    return impl::default_mod(lhs, static_cast<std::uint64_t>(rhs));
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool> = true>
constexpr u128 operator%(const UnsignedInteger lhs, const u128 rhs) noexcept
{
    if (rhs.high != 0)
    {
        return {0, static_cast<std::uint64_t>(lhs)};
    }
    else
    {
        if (rhs.low == 0)
        {
            return {0, 0};
        }
        else
        {
            return {0, lhs % rhs.low};
        }
    }
}

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr u128 operator%(const u128 lhs, const SignedInteger rhs) noexcept
{
    if (rhs > 0)
    {
        return impl::default_mod(lhs, static_cast<std::uint64_t>(rhs));
    }
    else
    {
        return lhs % static_cast<u128>(rhs);
    }
}

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool> = true>
constexpr u128 operator%(const SignedInteger lhs, const u128 rhs) noexcept
{
    if (lhs > 0)
    {
        if (rhs.high == 0)
        {
            return {0, static_cast<std::uint64_t>(lhs) % rhs.low};
        }
        else
        {
            return {0, static_cast<std::uint64_t>(lhs)};
        }
    }
    else
    {
        return static_cast<u128>(lhs) % rhs;
    }
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr u128 operator%(const u128 lhs, const __int128 rhs) noexcept
{
    return lhs % static_cast<u128>(rhs);
}

constexpr u128 operator%(const __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) % rhs;
}

constexpr u128 operator%(const u128 lhs, const unsigned __int128 rhs) noexcept
{
    return lhs % static_cast<u128>(rhs);
}

constexpr u128 operator%(const unsigned __int128 lhs, const u128 rhs) noexcept
{
    return static_cast<u128>(lhs) % rhs;
}

#endif // BOOST_DECIMAL_HAS_INT128

//=====================================
// Compound Modulo Operator
//=====================================

template <typename SignedInteger, std::enable_if_t<impl::is_signed_integer_v<SignedInteger>, bool>>
constexpr u128& u128::operator%=(const SignedInteger rhs) noexcept
{
    *this = *this % rhs;
    return *this;
}

template <typename UnsignedInteger, std::enable_if_t<impl::is_unsigned_integer_v<UnsignedInteger>, bool>>
constexpr u128& u128::operator%=(const UnsignedInteger rhs) noexcept
{
    *this = *this % rhs;
    return *this;
}

constexpr u128& u128::operator%=(const u128 rhs) noexcept
{
    *this = *this % rhs;
    return *this;
}

#ifdef BOOST_DECIMAL_HAS_INT128

constexpr u128& u128::operator%=(const __int128 rhs) noexcept
{
    *this = *this % rhs;
    return *this;
}

constexpr u128& u128::operator%=(const unsigned __int128 rhs) noexcept
{
    *this = *this % rhs;
    return *this;
}

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

#endif // BOOST_DECIMAL_HAS_INT128

#if !defined(BOOST_DECIMAL_DISABLE_IOSTREAM)

namespace impl {

inline char* u128_to_dec(char (&buffer)[ 64 ], u128 v)
{
    char* p = buffer + 64;
    *--p = '\0';

    do
    {
        *--p = "0123456789"[ static_cast<std::size_t>(v % UINT64_C(10)) ];
        v /= UINT64_C(10);
    } while ( v != 0 );

    return p;
}

inline char* u128_to_hex(char (&buffer)[ 64 ], u128 v, bool upper_case)
{
    const auto alphabet = upper_case ? "0123456789ABCDEF" : "0123456789abcdef";

    char* p = buffer + 64;
    *--p = '\0';

    do
    {
        const auto last_byte = v.low & 0xF;
        *--p = alphabet[ static_cast<std::size_t>(last_byte)];
        v >>= 4;
    } while (v != 0);

    return p;
}

inline char* u128_to_octal(char (&buffer)[ 64 ], u128 v)
{
    char* p = buffer + 64;
    *--p = '\0';

    do
    {
        const auto last_octet = v.low & 0x7;
        *--p = "01234567"[ static_cast<std::size_t>(last_octet)];
        v >>= 3;
    } while (v != 0);

    return p;
}

static constexpr unsigned char uchar_values[] =
{
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9, 255, 255, 255, 255, 255, 255,
  255,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
   25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35, 255, 255, 255, 255, 255,
  255,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
   25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};

static_assert(sizeof(uchar_values) == 256, "uchar_values should represent all 256 values of unsigned char");

constexpr unsigned char digit_from_char(char val) noexcept
{
    return uchar_values[static_cast<unsigned char>(val)];
}

template <std::size_t N>
void base_to_u128(char (&buffer)[N], u128& v, unsigned base)
{
    const char* end = buffer + N;
    char* next = buffer;

    const unsigned char first_digit = digit_from_char(*next);

    if (static_cast<unsigned>(first_digit) >= base)
    {
        return;
    }

    unsigned char current_digit = first_digit;
    while (next < end && current_digit != 255)
    {
        v = static_cast<u128>(v * base + current_digit);
        ++next;
        current_digit = digit_from_char(*next);
    }
}

} // namespace impl

template <typename charT, typename traits>
std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& os, u128 v)
{
    char buffer[64];

    auto os_flags {os.flags()};

    switch (os_flags & std::ios::basefield)
    {
        case std::ios::dec:
            os << impl::u128_to_dec(buffer, v);
            break;
        case std::ios::hex:
            os << impl::u128_to_hex(buffer, v, os_flags & std::ios::uppercase);
            break;
        case std::ios::oct:
            os << impl::u128_to_octal(buffer, v);
            break;
        // LCOV_EXCL_START
        default:
            BOOST_DECIMAL_UNREACHABLE;
        // LCOV_EXCL_STOP
    }

    return os;
}

#ifdef _MSC_VER
#  pragma warning (push)
#  pragma warning (disable: 4127) // conditional expression is constant
#endif

template <typename charT, typename traits>
std::basic_istream<charT, traits>& operator>>(std::basic_istream<charT, traits>& is, u128& v)
{
    charT t_buffer[1024] {};
    is >> t_buffer;

    char buffer[1024 * (sizeof(charT) / sizeof(char))] {};

    BOOST_DECIMAL_IF_CONSTEXPR (!std::is_same<charT, char>::value)
    {
        auto first = buffer;
        auto t_first = t_buffer;
        const auto t_buffer_end = t_buffer + generic_strlen(t_buffer);

        while (t_first != t_buffer_end)
        {
            *first++ = static_cast<char>(*t_first++);
        }
    }
    else
    {
        static_assert(sizeof(buffer) == sizeof(t_buffer), "Need to be equivalent");
        std::memcpy(buffer, t_buffer, sizeof(t_buffer));
    }

    convert_string_to_c_locale(buffer);

    auto is_flags {is.flags()};

    switch (is_flags & std::ios::basefield)
    {
        case std::ios::dec:
            impl::base_to_u128(buffer, v, 10U);
            break;
        case std::ios::hex:
            impl::base_to_u128(buffer, v, 16U);
            break;
        case std::ios::oct:
            impl::base_to_u128(buffer, v, 8U);
            break;
        // LCOV_EXCL_START
        default:
            BOOST_DECIMAL_UNREACHABLE;
        // LCOV_EXCL_STOP
    }

    return is;
}

#ifdef _MSC_VER
#  pragma warning (pop)
#endif

#endif // BOOST_DECIMAL_DISABLE_IOSTREAM

template <>
constexpr int countl_zero<u128>(u128 x) noexcept
{
    if (x.high == 0)
    {
        return 64 + countl_zero(x.low);
    }

    return countl_zero(x.high);
}

} // namespace detail
} // namespace decimal
} // namespace boost


// Non-standard libraries may add specializations for library-provided types
template <>
#ifdef _MSC_VER
class std::numeric_limits<boost::decimal::detail::u128>
#else
struct std::numeric_limits<boost::decimal::detail::u128>
#endif
{

#ifdef _MSC_VER
public:
#endif

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

#undef BOOST_DECIMAL_OLD_NON_GNU_COMPILER

#endif // BOOST_DECIMAL_DETAIL_U128_HPP
