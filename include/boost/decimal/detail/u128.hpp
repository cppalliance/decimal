// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// If the architecture (e.g. 32-bit x86) does not have __int128 we need to emulate it

#ifndef BOOST_DECIMAL_DETAIL_U128_HPP
#define BOOST_DECIMAL_DETAIL_U128_HPP

#include <boost/decimal/detail/config.hpp>

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
    constexpr u128& operator=(const std::int8_t value) noexcept { low = static_cast<std::uint64_t>(value); high = value < 0 ? UINT64_MAX : UINT64_C(0); return *this; }
    constexpr u128& operator=(const std::int16_t value) noexcept { low = static_cast<std::uint64_t>(value); high = value < 0 ? UINT64_MAX : UINT64_C(0); return *this; }
    constexpr u128& operator=(const std::int32_t value) noexcept { low = static_cast<std::uint64_t>(value); high = value < 0 ? UINT64_MAX : UINT64_C(0); return *this; }
    constexpr u128& operator=(const std::int64_t value) noexcept { low = static_cast<std::uint64_t>(value); high = value < 0 ? UINT64_MAX : UINT64_C(0); return *this; }

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr u128& operator=(const __int128 value) noexcept { low = static_cast<std::uint64_t>(value & low_word_mask); high = static_cast<std::uint64_t>(static_cast<unsigned __int128>(value) >> 64U); return *this; }
    #endif // BOOST_DECIMAL_HAS_INT128

    // Unsigned assignment operators
    constexpr u128& operator=(const std::uint8_t value) noexcept { low = static_cast<std::uint64_t>(value); high = UINT64_C(0); return *this; }
    constexpr u128& operator=(const std::uint16_t value) noexcept { low = static_cast<std::uint64_t>(value); high = UINT64_C(0); return *this; }
    constexpr u128& operator=(const std::uint32_t value) noexcept { low = static_cast<std::uint64_t>(value); high = UINT64_C(0); return *this; }
    constexpr u128& operator=(const std::uint64_t value) noexcept { low = static_cast<std::uint64_t>(value); high = UINT64_C(0); return *this; }

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr u128& operator=(const unsigned __int128 value) noexcept { low = static_cast<std::uint64_t>(value & low_word_mask); high = static_cast<std::uint64_t>(value >> 64U); return *this; }
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
};

} // namespace detail
} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_U128_HPP
