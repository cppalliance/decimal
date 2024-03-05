// Copyright 2020-2023 Daniel Lemire
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//
// If the architecture (e.g. 32-bit x86) does not have __int128 we need to emulate it

#ifndef BOOST_DECIMAL_DETAIL_EMULATED128_HPP
#define BOOST_DECIMAL_DETAIL_EMULATED128_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/countl.hpp>
#include <boost/decimal/detail/wide-integer/uintwide_t.hpp>

#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

#if !defined(BOOST_DECIMAL_DISABLE_IOSTREAM)
#include <iomanip>
#include <iosfwd>
#include <ostream>
#endif

namespace boost {
namespace decimal {
namespace detail {

#if __GNUC__ >= 8
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif

#if !defined(BOOST_DECIMAL_ENDIAN_LITTLE_BYTE) && defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wreorder"
#endif

// Compilers might support built-in 128-bit integer types. However, it seems that
// emulating them with a pair of 64-bit integers actually produces a better code,
// so we avoid using those built-ins. That said, they are still useful for
// implementing 64-bit x 64-bit -> 128-bit multiplication.

// Macro replacement lists can not be enclosed in parentheses
struct uint128
{
    #if BOOST_DECIMAL_ENDIAN_LITTLE_BYTE
    std::uint64_t low {};
    std::uint64_t high {};
    #else
    std::uint64_t high {};
    std::uint64_t low {};
    #endif

    // Constructors
    constexpr uint128() noexcept = default;

    constexpr uint128(const uint128& v) noexcept = default;

    constexpr uint128(uint128&& v) noexcept = default;

    #if BOOST_DECIMAL_ENDIAN_LITTLE_BYTE
    constexpr uint128(std::uint64_t high_, std::uint64_t low_) noexcept : low {low_}, high {high_} {}
    #else
    constexpr uint128(std::uint64_t high_, std::uint64_t low_) noexcept : high {high_}, low {low_} {}
    #endif

    #if BOOST_DECIMAL_ENDIAN_LITTLE_BYTE

    #define SIGNED_CONSTRUCTOR(expr) constexpr uint128(expr v) noexcept : low {static_cast<std::uint64_t>(v)}, high {v < 0 ? UINT64_MAX : UINT64_C(0)} {}// NOLINT
    #define UNSIGNED_CONSTRUCTOR(expr) constexpr uint128(expr v) noexcept : low {static_cast<std::uint64_t>(v)}, high {} {} // NOLINT

    #else

    #define SIGNED_CONSTRUCTOR(expr) constexpr uint128(expr v) noexcept : high {v < 0 ? UINT64_MAX : UINT64_C(0)}, low {static_cast<std::uint64_t>(v)} {}// NOLINT
    #define UNSIGNED_CONSTRUCTOR(expr) constexpr uint128(expr v) noexcept : high {}, low {static_cast<std::uint64_t>(v)} {} // NOLINT

    #endif

    SIGNED_CONSTRUCTOR(char)                    // NOLINT
    SIGNED_CONSTRUCTOR(signed char)             // NOLINT
    SIGNED_CONSTRUCTOR(short)                   // NOLINT
    SIGNED_CONSTRUCTOR(int)                     // NOLINT
    SIGNED_CONSTRUCTOR(long)                    // NOLINT
    SIGNED_CONSTRUCTOR(long long)               // NOLINT

    UNSIGNED_CONSTRUCTOR(unsigned char)         // NOLINT
    UNSIGNED_CONSTRUCTOR(unsigned short)        // NOLINT
    UNSIGNED_CONSTRUCTOR(unsigned)              // NOLINT
    UNSIGNED_CONSTRUCTOR(unsigned long)         // NOLINT
    UNSIGNED_CONSTRUCTOR(unsigned long long)    // NOLINT

    #ifdef BOOST_DECIMAL_HAS_INT128
    #  if BOOST_DECIMAL_ENDIAN_LITTLE_BYTE

    constexpr uint128(boost::decimal::detail::int128_t v) noexcept :  // NOLINT : Allow implicit conversions,
         low {static_cast<std::uint64_t>(static_cast<boost::decimal::detail::int128_t>(v) & ~UINT64_C(0))},
         high {static_cast<std::uint64_t>(v >> 64)} {}

    constexpr uint128(boost::decimal::detail::uint128_t v) noexcept : // NOLINT : Allow implicit conversions
        low {static_cast<std::uint64_t>(v & ~UINT64_C(0))},
        high {static_cast<std::uint64_t>(v >> 64)} {}

    #  else

    constexpr uint128(boost::decimal::detail::int128_t v) noexcept :  // NOLINT : Allow implicit conversions,
         high {static_cast<std::uint64_t>(v >> 64)},
         low {static_cast<std::uint64_t>(static_cast<boost::decimal::detail::uint128_t>(v) & ~UINT64_C(0))} {}

    constexpr uint128(boost::decimal::detail::uint128_t v) noexcept : // NOLINT : Allow implicit conversions
        high {static_cast<std::uint64_t>(v >> 64)},
        low {static_cast<std::uint64_t>(v & ~UINT64_C(0))}{}
        
    #  endif
    #endif

    #undef SIGNED_CONSTRUCTOR
    #undef UNSIGNED_CONSTRUCTOR

    // Assignment Operators
    #define   SIGNED_ASSIGNMENT_OPERATOR(expr) constexpr auto operator=(const expr& v) noexcept -> uint128& { high = v < 0 ? UINT64_MAX : UINT64_C(0); low = static_cast<std::uint64_t>(v); return *this; } // NOLINT
    #define UNSIGNED_ASSIGNMENT_OPERATOR(expr) constexpr auto operator=(const expr& v) noexcept -> uint128& { high = 0U; low = static_cast<std::uint64_t>(v); return *this; } // NOLINT

    SIGNED_ASSIGNMENT_OPERATOR(char)                    // NOLINT
    SIGNED_ASSIGNMENT_OPERATOR(signed char)             // NOLINT
    SIGNED_ASSIGNMENT_OPERATOR(short)                   // NOLINT
    SIGNED_ASSIGNMENT_OPERATOR(int)                     // NOLINT
    SIGNED_ASSIGNMENT_OPERATOR(long)                    // NOLINT
    SIGNED_ASSIGNMENT_OPERATOR(long long)               // NOLINT

    UNSIGNED_ASSIGNMENT_OPERATOR(unsigned char)         // NOLINT
    UNSIGNED_ASSIGNMENT_OPERATOR(unsigned short)        // NOLINT
    UNSIGNED_ASSIGNMENT_OPERATOR(unsigned)              // NOLINT
    UNSIGNED_ASSIGNMENT_OPERATOR(unsigned long)         // NOLINT
    UNSIGNED_ASSIGNMENT_OPERATOR(unsigned long long)    // NOLINT

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr auto operator=(const boost::decimal::detail::int128_t&  v) noexcept -> uint128& { *this = uint128(v); return *this; }
    constexpr auto operator=(const boost::decimal::detail::uint128_t& v) noexcept -> uint128& { *this = uint128(v); return *this; }
    #endif

    constexpr uint128& operator=(const uint128&) noexcept = default;

    #undef SIGNED_ASSIGNMENT_OPERATOR
    #undef UNSIGNED_ASSIGNMENT_OPERATOR

    // Conversion Operators
    #define INTEGER_CONVERSION_OPERATOR(expr) explicit constexpr operator expr() const noexcept { return static_cast<expr>(low); }
    #define   FLOAT_CONVERSION_OPERATOR(expr) explicit           operator expr() const noexcept { return std::ldexp(static_cast<expr>(high), 64) + static_cast<expr>(low); }

    INTEGER_CONVERSION_OPERATOR(char)                   // NOLINT
    INTEGER_CONVERSION_OPERATOR(signed char)            // NOLINT
    INTEGER_CONVERSION_OPERATOR(short)                  // NOLINT
    INTEGER_CONVERSION_OPERATOR(int)                    // NOLINT
    INTEGER_CONVERSION_OPERATOR(long)                   // NOLINT
    INTEGER_CONVERSION_OPERATOR(long long)              // NOLINT
    INTEGER_CONVERSION_OPERATOR(unsigned char)          // NOLINT
    INTEGER_CONVERSION_OPERATOR(unsigned short)         // NOLINT
    INTEGER_CONVERSION_OPERATOR(unsigned)               // NOLINT
    INTEGER_CONVERSION_OPERATOR(unsigned long)          // NOLINT
    INTEGER_CONVERSION_OPERATOR(unsigned long long)     // NOLINT

    explicit constexpr operator bool() const noexcept { return high || low; }

    #ifdef BOOST_DECIMAL_HAS_INT128
    explicit constexpr operator int128_t() noexcept { return (static_cast<int128_t>(high) << 64) + low; }
    explicit constexpr operator uint128_t() const noexcept { return (static_cast<uint128_t>(high) << 64) + low; }
    #endif

    #ifdef BOOST_DECIMAL_HAS_FLOAT128
    // TODO(mborland): I don't think the conversion functions are going to be necessary
    // explicit operator __float128() const noexcept { return ldexpq(static_cast<__float128>(high), 64) + static_cast<__float128>(low); }
    #endif

    FLOAT_CONVERSION_OPERATOR(float)        // NOLINT
    FLOAT_CONVERSION_OPERATOR(double)       // NOLINT
    FLOAT_CONVERSION_OPERATOR(long double)  // NOLINT

    #undef INTEGER_CONVERSION_OPERATOR
    #undef FLOAT_CONVERSION_OPERATOR

    // Unary Operators
    constexpr friend auto operator-(uint128 val) noexcept -> uint128;
    constexpr friend auto operator+(uint128 val) noexcept -> uint128;

    // Comparison Operators

    // Equality
    #define          INTEGER_OPERATOR_EQUAL(expr) constexpr friend auto operator==(uint128 lhs, expr rhs) noexcept -> bool { return lhs.high == 0 && rhs >= 0 && lhs.low == static_cast<std::uint64_t>(rhs); } // NOLINT
    #define UNSIGNED_INTEGER_OPERATOR_EQUAL(expr) constexpr friend auto operator==(uint128 lhs, expr rhs) noexcept -> bool { return lhs.high == 0 && lhs.low == static_cast<std::uint64_t>(rhs); } // NOLINT

    INTEGER_OPERATOR_EQUAL(char)                        // NOLINT
    INTEGER_OPERATOR_EQUAL(signed char)                 // NOLINT
    INTEGER_OPERATOR_EQUAL(short)                       // NOLINT
    INTEGER_OPERATOR_EQUAL(int)                         // NOLINT
    INTEGER_OPERATOR_EQUAL(long)                        // NOLINT
    INTEGER_OPERATOR_EQUAL(long long)                   // NOLINT
    UNSIGNED_INTEGER_OPERATOR_EQUAL(unsigned char)      // NOLINT
    UNSIGNED_INTEGER_OPERATOR_EQUAL(unsigned short)     // NOLINT
    UNSIGNED_INTEGER_OPERATOR_EQUAL(unsigned)           // NOLINT
    UNSIGNED_INTEGER_OPERATOR_EQUAL(unsigned long)      // NOLINT
    UNSIGNED_INTEGER_OPERATOR_EQUAL(unsigned long long) // NOLINT

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr friend auto operator==(uint128 lhs, boost::decimal::detail::int128_t  rhs) noexcept -> bool { return lhs == uint128(rhs); }
    constexpr friend auto operator==(uint128 lhs, boost::decimal::detail::uint128_t rhs) noexcept -> bool { return lhs == uint128(rhs); }
    #endif

    constexpr friend auto operator==(uint128 lhs, uint128 rhs) noexcept -> bool;

    #undef INTEGER_OPERATOR_EQUAL
    #undef UNSIGNED_INTEGER_OPERATOR_EQUAL

    // Inequality
    #define INTEGER_OPERATOR_NOTEQUAL(expr) constexpr friend auto operator!=(uint128 lhs, expr rhs) noexcept -> bool { return !(lhs == rhs); } // NOLINT

    INTEGER_OPERATOR_NOTEQUAL(char)                 // NOLINT
    INTEGER_OPERATOR_NOTEQUAL(signed char)          // NOLINT
    INTEGER_OPERATOR_NOTEQUAL(short)                // NOLINT
    INTEGER_OPERATOR_NOTEQUAL(int)                  // NOLINT
    INTEGER_OPERATOR_NOTEQUAL(long)                 // NOLINT
    INTEGER_OPERATOR_NOTEQUAL(long long)            // NOLINT
    INTEGER_OPERATOR_NOTEQUAL(unsigned char)        // NOLINT
    INTEGER_OPERATOR_NOTEQUAL(unsigned short)       // NOLINT
    INTEGER_OPERATOR_NOTEQUAL(unsigned)             // NOLINT
    INTEGER_OPERATOR_NOTEQUAL(unsigned long)        // NOLINT
    INTEGER_OPERATOR_NOTEQUAL(unsigned long long)   // NOLINT

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr friend auto operator!=(uint128 lhs, boost::decimal::detail::int128_t  rhs) noexcept -> bool { return !(lhs == rhs); }
    constexpr friend auto operator!=(uint128 lhs, boost::decimal::detail::uint128_t rhs) noexcept -> bool { return !(lhs == rhs); }
    #endif

    constexpr friend auto operator!=(uint128 lhs, uint128 rhs) noexcept -> bool;

    #undef INTEGER_OPERATOR_NOTEQUAL

    // Less than
    #define          INTEGER_OPERATOR_LESS_THAN(expr) constexpr friend auto operator<(uint128 lhs, expr rhs) noexcept -> bool { return lhs.high == 0U && rhs > 0 && lhs.low < static_cast<std::uint64_t>(rhs); } // NOLINT
    #define UNSIGNED_INTEGER_OPERATOR_LESS_THAN(expr) constexpr friend auto operator<(uint128 lhs, expr rhs) noexcept -> bool { return lhs.high == 0U && lhs.low < static_cast<std::uint64_t>(rhs); } // NOLINT

    INTEGER_OPERATOR_LESS_THAN(char)                            // NOLINT
    INTEGER_OPERATOR_LESS_THAN(signed char)                     // NOLINT
    INTEGER_OPERATOR_LESS_THAN(short)                           // NOLINT
    INTEGER_OPERATOR_LESS_THAN(int)                             // NOLINT
    INTEGER_OPERATOR_LESS_THAN(long)                            // NOLINT
    INTEGER_OPERATOR_LESS_THAN(long long)                       // NOLINT
    UNSIGNED_INTEGER_OPERATOR_LESS_THAN(unsigned char)          // NOLINT
    UNSIGNED_INTEGER_OPERATOR_LESS_THAN(unsigned short)         // NOLINT
    UNSIGNED_INTEGER_OPERATOR_LESS_THAN(unsigned)               // NOLINT
    UNSIGNED_INTEGER_OPERATOR_LESS_THAN(unsigned long)          // NOLINT
    UNSIGNED_INTEGER_OPERATOR_LESS_THAN(unsigned long long)     // NOLINT

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr friend auto operator<(uint128 lhs, boost::decimal::detail::int128_t  rhs) noexcept -> bool { return lhs < uint128(rhs); }
    constexpr friend auto operator<(uint128 lhs, boost::decimal::detail::uint128_t rhs) noexcept -> bool { return lhs < uint128(rhs); }
    #endif

    constexpr friend auto operator<(uint128 lhs, uint128 rhs) noexcept -> bool;

    #undef INTEGER_OPERATOR_LESS_THAN
    #undef UNSIGNED_INTEGER_OPERATOR_LESS_THAN

    // Less than or equal to
    #define          INTEGER_OPERATOR_LESS_THAN_OR_EQUAL_TO(expr) constexpr friend auto operator<=(uint128 lhs, expr rhs) noexcept -> bool { return lhs.high == 0U && rhs >= 0 && lhs.low <= static_cast<std::uint64_t>(rhs); } // NOLINT
    #define UNSIGNED_INTEGER_OPERATOR_LESS_THAN_OR_EQUAL_TO(expr) constexpr friend auto operator<=(uint128 lhs, expr rhs) noexcept -> bool { return lhs.high == 0U && lhs.low <= static_cast<std::uint64_t>(rhs); } // NOLINT

    INTEGER_OPERATOR_LESS_THAN_OR_EQUAL_TO(char)                            // NOLINT
    INTEGER_OPERATOR_LESS_THAN_OR_EQUAL_TO(signed char)                     // NOLINT
    INTEGER_OPERATOR_LESS_THAN_OR_EQUAL_TO(short)                           // NOLINT
    INTEGER_OPERATOR_LESS_THAN_OR_EQUAL_TO(int)                             // NOLINT
    INTEGER_OPERATOR_LESS_THAN_OR_EQUAL_TO(long)                            // NOLINT
    INTEGER_OPERATOR_LESS_THAN_OR_EQUAL_TO(long long)                       // NOLINT
    UNSIGNED_INTEGER_OPERATOR_LESS_THAN_OR_EQUAL_TO(unsigned char)          // NOLINT
    UNSIGNED_INTEGER_OPERATOR_LESS_THAN_OR_EQUAL_TO(unsigned short)         // NOLINT
    UNSIGNED_INTEGER_OPERATOR_LESS_THAN_OR_EQUAL_TO(unsigned)               // NOLINT
    UNSIGNED_INTEGER_OPERATOR_LESS_THAN_OR_EQUAL_TO(unsigned long)          // NOLINT
    UNSIGNED_INTEGER_OPERATOR_LESS_THAN_OR_EQUAL_TO(unsigned long long)     // NOLINT

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr friend auto operator<=(uint128 lhs, boost::decimal::detail::int128_t  rhs) noexcept -> bool { return lhs <= uint128(rhs); }
    constexpr friend auto operator<=(uint128 lhs, boost::decimal::detail::uint128_t rhs) noexcept -> bool { return lhs <= uint128(rhs); }
    #endif

    constexpr friend auto operator<=(uint128 lhs, uint128 rhs) noexcept -> bool ;

    #undef INTEGER_OPERATOR_LESS_THAN_OR_EQUAL_TO
    #undef UNSIGNED_INTEGER_OPERATOR_LESS_THAN_OR_EQUAL_TO

    // Greater than
    #define          INTEGER_OPERATOR_GREATER_THAN(expr) constexpr friend auto operator>(uint128 lhs, expr rhs) noexcept -> bool { return lhs.high > 0U || rhs < 0 || lhs.low > static_cast<std::uint64_t>(rhs); } // NOLINT
    #define UNSIGNED_INTEGER_OPERATOR_GREATER_THAN(expr) constexpr friend auto operator>(uint128 lhs, expr rhs) noexcept -> bool { return lhs.high > 0U || lhs.low > static_cast<std::uint64_t>(rhs); } // NOLINT

    INTEGER_OPERATOR_GREATER_THAN(char)                             // NOLINT
    INTEGER_OPERATOR_GREATER_THAN(signed char)                      // NOLINT
    INTEGER_OPERATOR_GREATER_THAN(short)                            // NOLINT
    INTEGER_OPERATOR_GREATER_THAN(int)                              // NOLINT
    INTEGER_OPERATOR_GREATER_THAN(long)                             // NOLINT
    INTEGER_OPERATOR_GREATER_THAN(long long)                        // NOLINT
    UNSIGNED_INTEGER_OPERATOR_GREATER_THAN(unsigned char)           // NOLINT
    UNSIGNED_INTEGER_OPERATOR_GREATER_THAN(unsigned short)          // NOLINT
    UNSIGNED_INTEGER_OPERATOR_GREATER_THAN(unsigned)                // NOLINT
    UNSIGNED_INTEGER_OPERATOR_GREATER_THAN(unsigned long)           // NOLINT
    UNSIGNED_INTEGER_OPERATOR_GREATER_THAN(unsigned long long)      // NOLINT

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr friend auto operator>(uint128 lhs, boost::decimal::detail::int128_t  rhs) noexcept -> bool { return lhs > uint128(rhs); }
    constexpr friend auto operator>(uint128 lhs, boost::decimal::detail::uint128_t rhs) noexcept -> bool { return lhs > uint128(rhs); }
    #endif

    constexpr friend auto operator>(uint128 lhs, uint128 rhs) noexcept -> bool ;

    #undef INTEGER_OPERATOR_GREATER_THAN
    #undef UNSIGNED_INTEGER_OPERATOR_GREATER_THAN

    // Greater than or equal to
    #define          INTEGER_OPERATOR_GREATER_THAN_OR_EQUAL_TO(expr) constexpr friend auto operator>=(uint128 lhs, expr rhs) noexcept  -> bool { return lhs.high > 0U || rhs < 0 || lhs.low >= static_cast<std::uint64_t>(rhs); } // NOLINT
    #define UNSIGNED_INTEGER_OPERATOR_GREATER_THAN_OR_EQUAL_TO(expr) constexpr friend auto operator>=(uint128 lhs, expr rhs) noexcept  -> bool { return lhs.high > 0U || lhs.low >= static_cast<std::uint64_t>(rhs); } // NOLINT

    INTEGER_OPERATOR_GREATER_THAN_OR_EQUAL_TO(char)                             // NOLINT
    INTEGER_OPERATOR_GREATER_THAN_OR_EQUAL_TO(signed char)                      // NOLINT
    INTEGER_OPERATOR_GREATER_THAN_OR_EQUAL_TO(short)                            // NOLINT
    INTEGER_OPERATOR_GREATER_THAN_OR_EQUAL_TO(int)                              // NOLINT
    INTEGER_OPERATOR_GREATER_THAN_OR_EQUAL_TO(long)                             // NOLINT
    INTEGER_OPERATOR_GREATER_THAN_OR_EQUAL_TO(long long)                        // NOLINT
    UNSIGNED_INTEGER_OPERATOR_GREATER_THAN_OR_EQUAL_TO(unsigned char)           // NOLINT
    UNSIGNED_INTEGER_OPERATOR_GREATER_THAN_OR_EQUAL_TO(unsigned short)          // NOLINT
    UNSIGNED_INTEGER_OPERATOR_GREATER_THAN_OR_EQUAL_TO(unsigned)                // NOLINT
    UNSIGNED_INTEGER_OPERATOR_GREATER_THAN_OR_EQUAL_TO(unsigned long)           // NOLINT
    UNSIGNED_INTEGER_OPERATOR_GREATER_THAN_OR_EQUAL_TO(unsigned long long)      // NOLINT

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr friend auto operator>=(uint128 lhs, boost::decimal::detail::int128_t  rhs) noexcept -> bool { return lhs >= uint128(rhs); }
    constexpr friend auto operator>=(uint128 lhs, boost::decimal::detail::uint128_t rhs) noexcept -> bool { return lhs >= uint128(rhs); }
    #endif

    constexpr friend auto operator>=(uint128 lhs, uint128 rhs) noexcept -> bool;

    #undef INTEGER_OPERATOR_GREATER_THAN_OR_EQUAL_TO
    #undef UNSIGNED_INTEGER_OPERATOR_GREATER_THAN_OR_EQUAL_TO

    // Binary Operators

    // Not
    constexpr friend auto operator~(uint128 v) noexcept -> uint128;

    // Or
    #define INTEGER_BINARY_OPERATOR_OR(expr) constexpr friend auto operator|(uint128 lhs, expr rhs) noexcept -> uint128 { return {lhs.high, lhs.low | static_cast<std::uint64_t>(rhs)}; } // NOLINT

    INTEGER_BINARY_OPERATOR_OR(char)                // NOLINT
    INTEGER_BINARY_OPERATOR_OR(signed char)         // NOLINT
    INTEGER_BINARY_OPERATOR_OR(short)               // NOLINT
    INTEGER_BINARY_OPERATOR_OR(int)                 // NOLINT
    INTEGER_BINARY_OPERATOR_OR(long)                // NOLINT
    INTEGER_BINARY_OPERATOR_OR(long long)           // NOLINT
    INTEGER_BINARY_OPERATOR_OR(unsigned char)       // NOLINT
    INTEGER_BINARY_OPERATOR_OR(unsigned short)      // NOLINT
    INTEGER_BINARY_OPERATOR_OR(unsigned)            // NOLINT
    INTEGER_BINARY_OPERATOR_OR(unsigned long)       // NOLINT
    INTEGER_BINARY_OPERATOR_OR(unsigned long long)  // NOLINT

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr friend auto operator|(uint128 lhs, boost::decimal::detail::int128_t  rhs) noexcept -> uint128 { return lhs | uint128(rhs); }
    constexpr friend auto operator|(uint128 lhs, boost::decimal::detail::uint128_t rhs) noexcept -> uint128 { return lhs | uint128(rhs); }
    #endif

    constexpr friend auto operator|(uint128 lhs, uint128 rhs) noexcept -> uint128;

    constexpr auto operator|=(uint128 v) noexcept -> uint128&;

    #undef INTEGER_BINARY_OPERATOR_OR

    // And
    #define INTEGER_BINARY_OPERATOR_AND(expr) constexpr friend auto operator&(uint128 lhs, expr rhs) noexcept -> uint128 { return {lhs.high, lhs.low & static_cast<std::uint64_t>(rhs)}; } // NOLINT

    INTEGER_BINARY_OPERATOR_AND(char)                   // NOLINT
    INTEGER_BINARY_OPERATOR_AND(signed char)            // NOLINT
    INTEGER_BINARY_OPERATOR_AND(short)                  // NOLINT
    INTEGER_BINARY_OPERATOR_AND(int)                    // NOLINT
    INTEGER_BINARY_OPERATOR_AND(long)                   // NOLINT
    INTEGER_BINARY_OPERATOR_AND(long long)              // NOLINT
    INTEGER_BINARY_OPERATOR_AND(unsigned char)          // NOLINT
    INTEGER_BINARY_OPERATOR_AND(unsigned short)         // NOLINT
    INTEGER_BINARY_OPERATOR_AND(unsigned)               // NOLINT
    INTEGER_BINARY_OPERATOR_AND(unsigned long)          // NOLINT
    INTEGER_BINARY_OPERATOR_AND(unsigned long long)     // NOLINT

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr friend auto operator&(uint128 lhs, boost::decimal::detail::int128_t  rhs) noexcept -> uint128 { return lhs & uint128(rhs); }
    constexpr friend auto operator&(uint128 lhs, boost::decimal::detail::uint128_t rhs) noexcept -> uint128 { return lhs & uint128(rhs); }
    #endif

    constexpr friend auto operator&(uint128 lhs, uint128 rhs) noexcept-> uint128;

    constexpr auto operator&=(uint128 v) noexcept -> uint128&;

    #undef INTEGER_BINARY_OPERATOR_AND

    // Xor
    #define INTEGER_BINARY_OPERATOR_XOR(expr) constexpr friend auto operator^(uint128 lhs, expr rhs) noexcept -> uint128 { return {lhs.high, lhs.low ^ static_cast<std::uint64_t>(rhs)}; } // NOLINT

    INTEGER_BINARY_OPERATOR_XOR(char)                   // NOLINT
    INTEGER_BINARY_OPERATOR_XOR(signed char)            // NOLINT
    INTEGER_BINARY_OPERATOR_XOR(short)                  // NOLINT
    INTEGER_BINARY_OPERATOR_XOR(int)                    // NOLINT
    INTEGER_BINARY_OPERATOR_XOR(long)                   // NOLINT
    INTEGER_BINARY_OPERATOR_XOR(long long)              // NOLINT
    INTEGER_BINARY_OPERATOR_XOR(unsigned char)          // NOLINT
    INTEGER_BINARY_OPERATOR_XOR(unsigned short)         // NOLINT
    INTEGER_BINARY_OPERATOR_XOR(unsigned)               // NOLINT
    INTEGER_BINARY_OPERATOR_XOR(unsigned long)          // NOLINT
    INTEGER_BINARY_OPERATOR_XOR(unsigned long long)     // NOLINT

    #ifdef BOOST_DECIMAL_HAS_INT128
    constexpr friend auto operator^(uint128 lhs, boost::decimal::detail::int128_t  rhs) noexcept -> uint128 { return lhs ^ uint128(rhs); }
    constexpr friend auto operator^(uint128 lhs, boost::decimal::detail::uint128_t rhs) noexcept -> uint128 { return lhs ^ uint128(rhs); }
    #endif

    constexpr friend auto operator^(uint128 lhs, uint128 rhs) noexcept -> uint128;

    constexpr auto operator^=(uint128 v) noexcept -> uint128&;

    #undef INTEGER_BINARY_OPERATOR_XOR

    // Left shift
    #define INTEGER_BINARY_OPERATOR_LEFT_SHIFT(expr)                                            \
    constexpr friend auto operator<<(uint128 lhs, expr rhs) noexcept -> uint128                 \
    {                                                                                           \
        if (rhs >= 64)                                                                          \
        {                                                                                       \
            return {lhs.low << (rhs - 64), 0};                                                  \
        }                                                                                       \
        else if (rhs == 0)                                                                      \
        {                                                                                       \
            return lhs;                                                                         \
        }                                                                                       \
                                                                                                \
        return {(lhs.high << rhs) | (lhs.low >> (64 - rhs)), lhs.low << rhs};                   \
    } // NOLINT

    INTEGER_BINARY_OPERATOR_LEFT_SHIFT(char)                    // NOLINT
    INTEGER_BINARY_OPERATOR_LEFT_SHIFT(signed char)             // NOLINT
    INTEGER_BINARY_OPERATOR_LEFT_SHIFT(short)                   // NOLINT
    INTEGER_BINARY_OPERATOR_LEFT_SHIFT(int)                     // NOLINT
    INTEGER_BINARY_OPERATOR_LEFT_SHIFT(long)                    // NOLINT
    INTEGER_BINARY_OPERATOR_LEFT_SHIFT(long long)               // NOLINT
    INTEGER_BINARY_OPERATOR_LEFT_SHIFT(unsigned char)           // NOLINT
    INTEGER_BINARY_OPERATOR_LEFT_SHIFT(unsigned short)          // NOLINT
    INTEGER_BINARY_OPERATOR_LEFT_SHIFT(unsigned)                // NOLINT
    INTEGER_BINARY_OPERATOR_LEFT_SHIFT(unsigned long)           // NOLINT
    INTEGER_BINARY_OPERATOR_LEFT_SHIFT(unsigned long long)      // NOLINT

    #define INTEGER_BINARY_OPERATOR_EQUALS_LEFT_SHIFT(expr)                     \
    constexpr auto operator<<=(expr amount) noexcept -> uint128&                \
    {                                                                           \
        *this = *this << amount;                                                \
        return *this;                                                           \
    } // NOLINT

    INTEGER_BINARY_OPERATOR_EQUALS_LEFT_SHIFT(char)                     // NOLINT
    INTEGER_BINARY_OPERATOR_EQUALS_LEFT_SHIFT(signed char)              // NOLINT
    INTEGER_BINARY_OPERATOR_EQUALS_LEFT_SHIFT(short)                    // NOLINT
    INTEGER_BINARY_OPERATOR_EQUALS_LEFT_SHIFT(int)                      // NOLINT
    INTEGER_BINARY_OPERATOR_EQUALS_LEFT_SHIFT(long)                     // NOLINT
    INTEGER_BINARY_OPERATOR_EQUALS_LEFT_SHIFT(long long)                // NOLINT
    INTEGER_BINARY_OPERATOR_EQUALS_LEFT_SHIFT(unsigned char)            // NOLINT
    INTEGER_BINARY_OPERATOR_EQUALS_LEFT_SHIFT(unsigned short)           // NOLINT
    INTEGER_BINARY_OPERATOR_EQUALS_LEFT_SHIFT(unsigned)                 // NOLINT
    INTEGER_BINARY_OPERATOR_EQUALS_LEFT_SHIFT(unsigned long)            // NOLINT
    INTEGER_BINARY_OPERATOR_EQUALS_LEFT_SHIFT(unsigned long long)       // NOLINT

    #undef INTEGER_BINARY_OPERATOR_LEFT_SHIFT
    #undef INTEGER_BINARY_OPERATOR_EQUALS_LEFT_SHIFT

    // Right Shift
    #define INTEGER_BINARY_OPERATOR_RIGHT_SHIFT(expr)                                               \
    constexpr friend auto operator>>(uint128 lhs, expr amount) noexcept -> uint128                  \
    {                                                                                               \
        if (amount >= 64)                                                                           \
        {                                                                                           \
            return {0, lhs.high >> (amount - 64)};                                                  \
        }                                                                                           \
        else if (amount == 0)                                                                       \
        {                                                                                           \
            return lhs;                                                                             \
        }                                                                                           \
                                                                                                    \
        return {lhs.high >> amount, (lhs.low >> amount) | (lhs.high << (64 - amount))};             \
    } // NOLINT

    INTEGER_BINARY_OPERATOR_RIGHT_SHIFT(char)                   // NOLINT
    INTEGER_BINARY_OPERATOR_RIGHT_SHIFT(signed char)            // NOLINT
    INTEGER_BINARY_OPERATOR_RIGHT_SHIFT(short)                  // NOLINT
    INTEGER_BINARY_OPERATOR_RIGHT_SHIFT(int)                    // NOLINT
    INTEGER_BINARY_OPERATOR_RIGHT_SHIFT(long)                   // NOLINT
    INTEGER_BINARY_OPERATOR_RIGHT_SHIFT(long long)              // NOLINT
    INTEGER_BINARY_OPERATOR_RIGHT_SHIFT(unsigned char)          // NOLINT
    INTEGER_BINARY_OPERATOR_RIGHT_SHIFT(unsigned short)         // NOLINT
    INTEGER_BINARY_OPERATOR_RIGHT_SHIFT(unsigned)               // NOLINT
    INTEGER_BINARY_OPERATOR_RIGHT_SHIFT(unsigned long)          // NOLINT
    INTEGER_BINARY_OPERATOR_RIGHT_SHIFT(unsigned long long)     // NOLINT

    #define INTEGER_BINARY_OPERATOR_EQUALS_RIGHT_SHIFT(expr)                        \
    constexpr auto operator>>=(expr amount) noexcept -> uint128&                    \
    {                                                                               \
        *this = *this >> amount;                                                    \
        return *this;                                                               \
    } // NOLINT

    INTEGER_BINARY_OPERATOR_EQUALS_RIGHT_SHIFT(char)                    // NOLINT
    INTEGER_BINARY_OPERATOR_EQUALS_RIGHT_SHIFT(signed char)             // NOLINT
    INTEGER_BINARY_OPERATOR_EQUALS_RIGHT_SHIFT(short)                   // NOLINT
    INTEGER_BINARY_OPERATOR_EQUALS_RIGHT_SHIFT(int)                     // NOLINT
    INTEGER_BINARY_OPERATOR_EQUALS_RIGHT_SHIFT(long)                    // NOLINT
    INTEGER_BINARY_OPERATOR_EQUALS_RIGHT_SHIFT(long long)               // NOLINT
    INTEGER_BINARY_OPERATOR_EQUALS_RIGHT_SHIFT(unsigned char)           // NOLINT
    INTEGER_BINARY_OPERATOR_EQUALS_RIGHT_SHIFT(unsigned short)          // NOLINT
    INTEGER_BINARY_OPERATOR_EQUALS_RIGHT_SHIFT(unsigned)                // NOLINT
    INTEGER_BINARY_OPERATOR_EQUALS_RIGHT_SHIFT(unsigned long)           // NOLINT
    INTEGER_BINARY_OPERATOR_EQUALS_RIGHT_SHIFT(unsigned long long)      // NOLINT

    #undef INTEGER_BINARY_OPERATOR_RIGHT_SHIFT
    #undef INTEGER_BINARY_OPERATOR_EQUALS_RIGHT_SHIFT

    // Arithmetic operators (Add, sub, mul, div, mod)
    constexpr auto operator+=(std::uint64_t n) noexcept -> uint128&;

    constexpr friend auto operator+(uint128 lhs, uint128 rhs) noexcept -> uint128;

    constexpr auto operator+=(uint128 v) noexcept -> uint128&;

    constexpr auto operator++() noexcept -> uint128&;

    constexpr auto operator++(int) noexcept -> uint128;

    constexpr friend auto operator-(uint128 lhs, uint128 rhs) noexcept -> uint128;

    constexpr auto operator-=(uint128 v) noexcept -> uint128&;

    constexpr auto operator--() noexcept -> uint128&;

    constexpr auto operator--(int) noexcept -> uint128;

    constexpr friend auto operator*(uint128 lhs, uint128 rhs) noexcept -> uint128;

    constexpr friend auto operator*(uint128 lhs, std::uint64_t rhs) noexcept -> uint128;

    constexpr auto operator*=(uint128 v) noexcept -> uint128&;

    constexpr auto operator*=(std::uint64_t v) noexcept -> uint128&;

    constexpr friend auto operator/(uint128 lhs, uint128 rhs) noexcept -> uint128;

    constexpr friend auto operator/(uint128 lhs, std::uint64_t rhs) noexcept -> uint128;

    constexpr auto operator/=(uint128 v) noexcept -> uint128&;

    constexpr friend auto operator%(uint128 lhs, uint128 rhs) noexcept -> uint128;

    constexpr friend auto operator%(uint128 lhs, std::uint64_t rhs) noexcept -> uint128;

    constexpr auto operator%=(uint128 v) noexcept -> uint128&;

    #if !defined(BOOST_DECIMAL_DISABLE_IOSTREAM)
    template <typename charT, typename traits>
    friend auto operator<<(std::basic_ostream<charT, traits>& os, uint128 val) -> std::basic_ostream<charT, traits>&;
    #endif

    constexpr void add_with_carry(const uint128& other, bool& carry)
    {
        auto previous_low = static_cast<std::uint64_t>(low);

        low += other.low;
        carry = low < previous_low;

        auto previous_high = static_cast<std::uint64_t>(high);
        high += other.high + (carry ? 1 : 0);
        carry = high < previous_high || (carry && high == previous_high);
    }

private:
    constexpr friend auto high_bit(uint128 v) noexcept -> int;

    constexpr friend auto div_impl(uint128 lhs, uint128 rhs, uint128 &quotient, uint128 &remainder) noexcept -> void;
};

struct int128
{
    #if BOOST_DECIMAL_ENDIAN_LITTLE_BYTE
    std::uint64_t low {};
    std::int64_t high {};
    #else
    std::int64_t high {};
    std::uint64_t low {};
    #endif

    // Constructors
    constexpr int128() noexcept = default;
    constexpr int128(const int128& v) noexcept = default;
    constexpr int128(int128&& v) noexcept = default;
    constexpr int128& operator=(const int128& v) = default;

    #if BOOST_DECIMAL_ENDIAN_LITTLE_BYTE
    constexpr int128(std::int64_t high_, std::uint64_t low_) noexcept : low {low_}, high {high_} {}
    constexpr int128(const uint128& v) noexcept : low {v.low}, high {static_cast<std::int64_t>(v.high)} {}
    explicit constexpr int128(std::uint64_t v) noexcept : low {v}, high {} {}
    explicit constexpr int128(std::uint32_t v) noexcept : low {v}, high {} {}
    explicit constexpr int128(std::uint16_t v) noexcept : low {v}, high {} {}
    explicit constexpr int128(std::uint8_t v) noexcept : low {v}, high {} {}
    explicit constexpr int128(std::int64_t v) noexcept : low{static_cast<std::uint64_t>(v)}, high{v < 0 ? -1 : 0} {}
    explicit constexpr int128(std::int32_t v) noexcept : low{static_cast<std::uint64_t>(v)}, high{v < 0 ? -1 : 0} {}
    explicit constexpr int128(std::int16_t v) noexcept : low{static_cast<std::uint64_t>(v)}, high{v < 0 ? -1 : 0} {}
    explicit constexpr int128(std::int8_t v) noexcept : low{static_cast<std::uint64_t>(v)}, high{v < 0 ? -1 : 0} {}
    #else
    constexpr int128(std::int64_t high_, std::uint64_t low_) noexcept : high {high_}, low {low_} {}
    constexpr int128(const uint128& v) noexcept : high {static_cast<std::int64_t>(v.high)}, low {v.low} {}
    explicit constexpr int128(std::uint64_t v) noexcept : high {}, low {v} {}
    explicit constexpr int128(std::uint32_t v) noexcept : high {}, low {v} {}
    explicit constexpr int128(std::uint16_t v) noexcept : high {}, low {v} {}
    explicit constexpr int128(std::uint8_t v) noexcept : high {}, low {v} {}
    explicit constexpr int128(std::int64_t v) noexcept : high{v < 0 ? -1 : 0}, low{static_cast<std::uint64_t>(v)} {}
    explicit constexpr int128(std::int32_t v) noexcept : high{v < 0 ? -1 : 0}, low{static_cast<std::uint64_t>(v)} {}
    explicit constexpr int128(std::int16_t v) noexcept : high{v < 0 ? -1 : 0}, low{static_cast<std::uint64_t>(v)} {}
    explicit constexpr int128(std::int8_t v) noexcept : high{v < 0 ? -1 : 0}, low{static_cast<std::uint64_t>(v)} {}
    #endif

    explicit constexpr operator uint128() const noexcept;

    friend constexpr auto operator-(int128 rhs) noexcept -> int128;

    constexpr auto operator<(int128 rhs) const noexcept -> bool;
    constexpr auto operator>(int128 rhs) const noexcept -> bool;
    constexpr auto operator<=(int128 rhs) const noexcept -> bool;
    constexpr auto operator>=(int128 rhs) const noexcept -> bool;
    constexpr auto operator==(int128 rhs) const noexcept -> bool;
    constexpr auto operator!=(int128 rhs) const noexcept -> bool;
    constexpr auto operator<(std::int64_t rhs) const noexcept -> bool;
    constexpr auto operator==(std::int64_t rhs) const noexcept -> bool;
    constexpr auto operator>(std::int64_t rhs) const noexcept -> bool;

    friend constexpr auto operator+(int128 lhs, int128 rhs) noexcept -> int128;
    friend constexpr auto operator-(int128 lhs, int128 rhs) noexcept -> int128;

    #if !defined(BOOST_DECIMAL_DISABLE_IOSTREAM)
    template <typename charT, typename traits>
    friend auto operator<<(std::basic_ostream<charT, traits>& os, int128 val) -> std::basic_ostream<charT, traits>&;
    #endif
};

#if (__GNUC__ >= 8) || (!defined(BOOST_DECIMAL_ENDIAN_LITTLE_BYTE) && defined(__GNUC__))
#  pragma GCC diagnostic pop
#endif

constexpr auto operator-(uint128 val) noexcept -> uint128
{
    return {~val.high + static_cast<std::uint64_t>(val.low == 0), ~val.low + 1};
}

constexpr auto operator+(uint128 val) noexcept -> uint128
{
    return val;
}

constexpr auto operator==(uint128 lhs, uint128 rhs) noexcept -> bool
{
    return lhs.high == rhs.high && lhs.low == rhs.low;
}

constexpr auto operator!=(uint128 lhs, uint128 rhs) noexcept -> bool
{
    return !(lhs == rhs);
}

constexpr auto operator<(uint128 lhs, uint128 rhs) noexcept -> bool
{
    if (lhs.high == rhs.high)
    {
        return lhs.low < rhs.low;
    }

    return lhs.high < rhs.high;
}

constexpr auto operator<=(uint128 lhs, uint128 rhs) noexcept -> bool
{
    return !(rhs < lhs);
}

constexpr auto operator>(uint128 lhs, uint128 rhs) noexcept -> bool
{
    return rhs < lhs;
}

constexpr auto operator>=(uint128 lhs, uint128 rhs) noexcept -> bool
{
    return !(lhs < rhs);
}

constexpr auto operator~(uint128 v) noexcept -> uint128
{
    return {~v.high, ~v.low};
}

constexpr auto operator|(uint128 lhs, uint128 rhs) noexcept -> uint128
{
    return {lhs.high | rhs.high, lhs.low | rhs.low};
}

constexpr auto uint128::operator|=(uint128 v) noexcept -> uint128&
{
    *this = *this | v;
    return *this;
}

constexpr auto operator&(uint128 lhs, uint128 rhs) noexcept -> uint128
{
    return {lhs.high & rhs.high, lhs.low & rhs.low};
}

constexpr auto uint128::operator&=(uint128 v) noexcept -> uint128&
{
    *this = *this & v;
    return *this;
}

constexpr auto operator^(uint128 lhs, uint128 rhs) noexcept -> uint128
{
    return {lhs.high ^ rhs.high, lhs.low ^ rhs.low};
}

constexpr auto uint128::operator^=(uint128 v) noexcept -> uint128&
{
    *this = *this ^ v;
    return *this;
}

constexpr auto uint128::operator+=(std::uint64_t n) noexcept -> uint128&
{
    auto sum = low + n;
    high += (sum < low ? 1 : 0);
    low = sum;

    return *this;
}

constexpr auto operator+(uint128 lhs, uint128 rhs) noexcept -> uint128
{
    const uint128 temp = {lhs.high + rhs.high, lhs.low + rhs.low};

    // Need to carry a bit into rhs
    if (temp.low < lhs.low)
    {
        return {temp.high + 1, temp.low};
    }

    return temp;
}

constexpr auto uint128::operator+=(uint128 v) noexcept -> uint128&
{
    *this = *this + v;
    return *this;
}

constexpr auto uint128::operator++() noexcept -> uint128&
{
    if (this->low == UINT64_MAX)
    {
        this->low = 0;
        ++this->high;
    }
    else
    {
        ++this->low;
    }

    return *this;
}

constexpr auto uint128::operator++(int) noexcept -> uint128
{
    return ++(*this);
}

constexpr auto operator-(uint128 lhs, uint128 rhs) noexcept -> uint128
{
    const uint128 temp {lhs.high - rhs.high, lhs.low - rhs.low};

    // Check for carry
    if (lhs.low < rhs.low)
    {
        return {temp.high - 1, temp.low};
    }

    return temp;
}

constexpr auto uint128::operator-=(uint128 v) noexcept -> uint128&
{
    *this = *this - v;
    return *this;
}

constexpr auto uint128::operator--() noexcept -> uint128&
{
    if (this->low == 0)
    {
        this->low = UINT64_MAX;
        --this->high;
    }
    else // NOLINT
    {
        --this->low;
    }

    return *this;
}

constexpr auto uint128::operator--(int) noexcept -> uint128
{
    return --(*this);
}

constexpr auto operator*(uint128 lhs, uint128 rhs) noexcept -> uint128
{
    const auto a = static_cast<std::uint64_t>(lhs.low >> 32);
    const auto b = static_cast<std::uint64_t>(lhs.low & UINT32_MAX);
    const auto c = static_cast<std::uint64_t>(rhs.low >> 32);
    const auto d = static_cast<std::uint64_t>(rhs.low & UINT32_MAX);

    uint128 result { lhs.high * rhs.low + lhs.low * rhs.high + a * c, b * d };
    result += uint128(a * d) << 32;
    result += uint128(b * c) << 32;
    return result;
}

// TODO(mborland): Can be replaced by intrinsics at runtime
constexpr auto multiply_64_64(std::uint64_t a, std::uint64_t b) -> uint128
{
    std::uint64_t a_low = a & UINT32_MAX;
    std::uint64_t a_high = a >> 32;
    std::uint64_t b_low = b & UINT32_MAX;
    std::uint64_t b_high = b >> 32;

    std::uint64_t low_product = a_low * b_low;
    std::uint64_t mid_product1 = a_high * b_low;
    std::uint64_t mid_product2 = a_low * b_high;
    std::uint64_t high_product = a_high * b_high;

    std::uint64_t mid_sum = (low_product >> 32) + (mid_product1 & UINT32_MAX) + mid_product2;
    std::uint64_t high = high_product + (mid_product1 >> 32) + (mid_sum >> 32);
    std::uint64_t low = (mid_sum << 32) | (low_product & UINT32_MAX);

    return {high, low};
}

constexpr auto operator*(uint128 lhs, std::uint64_t rhs) noexcept -> uint128
{
    auto low = multiply_64_64(lhs.low, rhs);
    auto high = multiply_64_64(lhs.high, rhs);

    uint128 result;
    result.low = low.low;
    result.high = low.high + high.low;
    if (result.high < low.high)
    {
        high.high += 1; // Handle overflow
    }
    result.high += high.high;

    return result;
}

constexpr auto uint128::operator*=(uint128 v) noexcept -> uint128&
{
    *this = *this * v;
    return *this;
}

constexpr auto uint128::operator*=(std::uint64_t v) noexcept -> uint128&
{
    *this = *this * v;
    return *this;
}

constexpr auto high_bit(uint128 v) noexcept -> int
{
    if (v.high != 0)
    {
        #ifdef BOOST_DECIMAL_HAS_STDBIT
        return 127 - std::countl_zero(v.high);
        #else
        return 127 - countl_zero(v.high);
        #endif
    }
    else if (v.low != 0)
    {
        #ifdef BOOST_DECIMAL_HAS_STDBIT
        return 63 - std::countl_zero(v.low);
        #else
        return 63 - countl_zero(v.low);
        #endif
    }

    return 0;
}

using wide_integer_uint128 = ::boost::decimal::math::wide_integer::uint128_t;

constexpr auto uint128_to_wide_integer(const uint128& src) -> wide_integer_uint128
{
    wide_integer_uint128 dst { };

    using local_limb_type = typename wide_integer_uint128::limb_type;

    static_assert(sizeof(local_limb_type) == static_cast<std::size_t>(UINT8_C(4)) && std::is_same<local_limb_type, std::uint32_t>::value, "Error: Configuration of external wide-integer limbs not OK");

    dst.representation()[static_cast<std::size_t>(UINT8_C(0))] = static_cast<local_limb_type>(src.low);
    dst.representation()[static_cast<std::size_t>(UINT8_C(1))] = static_cast<local_limb_type>(src.low >> static_cast<unsigned>(UINT8_C(32)));
    dst.representation()[static_cast<std::size_t>(UINT8_C(2))] = static_cast<local_limb_type>(src.high);
    dst.representation()[static_cast<std::size_t>(UINT8_C(3))] = static_cast<local_limb_type>(src.high >> static_cast<unsigned>(UINT8_C(32)));

    return dst;
}

constexpr auto wide_integer_to_uint128(const wide_integer_uint128& src) -> uint128
{
    uint128 dst { };

    dst.low =
        static_cast<std::uint64_t>
        (
                                           src.crepresentation()[static_cast<std::size_t>(UINT8_C(0))]
            | static_cast<std::uint64_t>
              (
                static_cast<std::uint64_t>(src.crepresentation()[static_cast<std::size_t>(UINT8_C(1))]) << static_cast<unsigned>(UINT8_C(32))
              )
        );

    dst.high =
        static_cast<std::uint64_t>
        (
                                           src.crepresentation()[static_cast<std::size_t>(UINT8_C(2))]
            | static_cast<std::uint64_t>
              (
                static_cast<std::uint64_t>(src.crepresentation()[static_cast<std::size_t>(UINT8_C(3))]) << static_cast<unsigned>(UINT8_C(32))
              )
        );

    return dst;
}

constexpr auto div_impl(uint128 lhs, uint128 rhs, uint128& quotient, uint128& remainder) noexcept -> void
{
    if ((rhs.high == UINT64_C(0)) && (rhs.low < (static_cast<std::uint64_t>(UINT64_C(0x100000000)))) && (rhs.low > (static_cast<std::uint64_t>(UINT64_C(0x0)))))
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

        current = static_cast<std::uint64_t>(remainder.low << 32U) | static_cast<std::uint32_t>(lhs.low);
        quotient.low |= static_cast<std::uint32_t>(current / rhs32);
        remainder.low = static_cast<std::uint32_t>(current % rhs32);

        remainder.high = UINT64_C(0);
    }
    else
    {
        // Mash-Up: Use Knuth long-division from wide-integer (requires limb-conversions on input/output).

        auto lhs_wide = uint128_to_wide_integer(lhs);

        wide_integer_uint128 rem_wide { };

        lhs_wide.eval_divide_knuth(uint128_to_wide_integer(rhs), rem_wide);

        remainder = wide_integer_to_uint128(rem_wide);
        quotient  = wide_integer_to_uint128(lhs_wide);
    }
}

constexpr auto operator/(uint128 lhs, uint128 rhs) noexcept -> uint128
{
    uint128 quotient {0, 0};
    uint128 remainder {0, 0};
    div_impl(lhs, rhs, quotient, remainder);

    return quotient;
}

constexpr auto operator/(uint128 lhs, std::uint64_t rhs) noexcept -> uint128
{
    uint128 quotient {0, 0};
    uint128 remainder {0, 0};
    div_impl(lhs, uint128(rhs), quotient, remainder);

    return quotient;
}

constexpr auto uint128::operator/=(uint128 v) noexcept -> uint128&
{
    *this = *this / v;
    return *this;
}

constexpr auto operator%(uint128 lhs, uint128 rhs) noexcept -> uint128
{
    uint128 quotient {0, 0};
    uint128 remainder {0, 0};
    div_impl(lhs, rhs, quotient, remainder);

    return remainder;
}

constexpr auto operator%(uint128 lhs, std::uint64_t rhs) noexcept -> uint128
{
    uint128 quotient {0, 0};
    uint128 remainder {0, 0};
    div_impl(lhs, uint128(rhs), quotient, remainder);

    return remainder;
}

constexpr auto uint128::operator%=(uint128 v) noexcept -> uint128&
{
    *this = *this % v;
    return *this;
}

constexpr auto umul64(std::uint32_t x, std::uint32_t y) noexcept -> std::uint64_t
{
    return x * static_cast<std::uint64_t>(y);
}

// Get 128-bit result of multiplication of two 64-bit unsigned integers.
constexpr auto umul128(std::uint64_t x, std::uint64_t y) noexcept -> uint128
{
    #if defined(BOOST_DECIMAL_HAS_INT128)

    auto result = static_cast<boost::decimal::detail::uint128_t>(x) * static_cast<boost::decimal::detail::uint128_t>(y);
    return {static_cast<std::uint64_t>(result >> 64), static_cast<std::uint64_t>(result)};

    #else

    auto a = static_cast<std::uint32_t>(x >> 32);
    auto b = static_cast<std::uint32_t>(x);
    auto c = static_cast<std::uint32_t>(y >> 32);
    auto d = static_cast<std::uint32_t>(y);

    auto ac = umul64(a, c);
    auto bc = umul64(b, c);
    auto ad = umul64(a, d);
    auto bd = umul64(b, d);

    auto intermediate = (bd >> 32) + static_cast<std::uint32_t>(ad) + static_cast<std::uint32_t>(bc);

    return {ac + (intermediate >> 32) + (ad >> 32) + (bc >> 32),
            (intermediate << 32) + static_cast<std::uint32_t>(bd)};
    
    #endif
}

constexpr auto umul128_upper64(std::uint64_t x, std::uint64_t y) noexcept -> std::uint64_t
{
    #if defined(BOOST_DECIMAL_HAS_INT128)
    
    auto result = static_cast<boost::decimal::detail::uint128_t>(x) * static_cast<boost::decimal::detail::uint128_t>(y);
    return static_cast<std::uint64_t>(result >> 64);
    
    #else
    
    auto a = static_cast<std::uint32_t>(x >> 32);
    auto b = static_cast<std::uint32_t>(x);
    auto c = static_cast<std::uint32_t>(y >> 32);
    auto d = static_cast<std::uint32_t>(y);

    auto ac = umul64(a, c);
    auto bc = umul64(b, c);
    auto ad = umul64(a, d);
    auto bd = umul64(b, d);

    auto intermediate = (bd >> 32) + static_cast<std::uint32_t>(ad) + static_cast<std::uint32_t>(bc);

    return ac + (intermediate >> 32) + (ad >> 32) + (bc >> 32);
    
    #endif
}

// Get upper 128-bits of multiplication of a 64-bit unsigned integer and a 128-bit
// unsigned integer.
constexpr auto umul192_upper128(std::uint64_t x, uint128 y) noexcept -> uint128
{
    auto r = umul128(x, y.high);
    r += umul128_upper64(x, y.low);
    return r;
}

// Get upper 64-bits of multiplication of a 32-bit unsigned integer and a 64-bit
// unsigned integer.
constexpr auto umul96_upper64(std::uint32_t x, std::uint64_t y) noexcept -> std::uint64_t
{
    #if defined(BOOST_DECIMAL_HAS_INT128) || defined(BOOST_DECIMAL_HAS_MSVC_64BIT_INTRINSICS)
    
    return umul128_upper64(static_cast<std::uint64_t>(x) << 32, y);
    
    #else
    
    auto yh = static_cast<std::uint32_t>(y >> 32);
    auto yl = static_cast<std::uint32_t>(y);

    auto xyh = umul64(x, yh);
    auto xyl = umul64(x, yl);

    return xyh + (xyl >> 32);

    #endif
}

// Get lower 128-bits of multiplication of a 64-bit unsigned integer and a 128-bit
// unsigned integer.
constexpr auto umul192_lower128(std::uint64_t x, uint128 y) noexcept -> uint128
{
    auto high = x * y.high;
    auto highlow = umul128(x, y.low);
    return {high + highlow.high, highlow.low};
}

// Get lower 64-bits of multiplication of a 32-bit unsigned integer and a 64-bit
// unsigned integer.
constexpr auto umul96_lower64(std::uint32_t x, std::uint64_t y) noexcept -> std::uint64_t
{
    return x * y;
}

inline auto emulated128_to_buffer(char (&buffer)[ 64 ], uint128 v)
{
    char* p = buffer + 64;
    *--p = '\0';

    do
    {
        *--p = "0123456789"[ static_cast<std::size_t>(v % 10) ];
        v /= 10;
    }
    while ( v != 0 );

    return p;
}

#if !defined(BOOST_DECIMAL_DISABLE_IOSTREAM)
template <typename charT, typename traits>
inline auto operator<<(std::basic_ostream<charT, traits>& os, uint128 val) -> std::basic_ostream<charT, traits>&
{
    char buffer[64];

    os << emulated128_to_buffer(buffer, val);

    return os;
}

template <typename charT, typename traits>
inline auto operator<<(std::basic_ostream<charT, traits>& os, int128 val) -> std::basic_ostream<charT, traits>&
{
    char buffer[64];
    char* p;

    if (val >= int128(0))
    {
        p = emulated128_to_buffer(buffer, static_cast<uint128>(val));
    }
    else
    {
        p = emulated128_to_buffer(buffer, static_cast<uint128>(-val));
        *--p = '-';
    }

    os << p;
    return os;
}
#endif

constexpr int128::operator uint128() const noexcept
{
    return {static_cast<std::uint64_t>(this->high), this->low};
}

constexpr auto operator-(int128 rhs) noexcept -> int128
{
    const auto new_low {~rhs.low + 1};
    const auto carry {static_cast<std::int64_t>(new_low == 0)};
    const auto new_high {~rhs.high + carry};
    return int128{new_high, new_low};
}

constexpr auto int128::operator<(int128 rhs) const noexcept -> bool
{
    if (high == rhs.high)
    {
        return low < rhs.low;
    }

    return high < rhs.high;
}

// Greater-than operator
constexpr auto int128::operator>(int128 rhs) const noexcept -> bool
{
    return rhs < *this;
}

// Less-than or equal-to operator
constexpr auto int128::operator<=(int128 rhs) const noexcept -> bool
{
    return !(*this > rhs);
}

// Greater-than or equal-to operator
constexpr auto int128::operator>=(int128 rhs) const noexcept -> bool
{
    return !(*this < rhs);
}

constexpr auto int128::operator==(int128 rhs) const noexcept -> bool
{
    return this->high == rhs.high && this->low == rhs.low;
}

constexpr auto int128::operator!=(int128 rhs) const noexcept -> bool
{
    return !(*this == rhs);
}

constexpr auto int128::operator==(std::int64_t rhs) const noexcept -> bool
{
    if (high == 0 && low == static_cast<std::uint64_t>(rhs))
    {
        return true;
    }
    else if (high == -1 && rhs < 0 && low == static_cast<std::uint64_t>(rhs))
    {
        return true;
    }

    return false;
}

constexpr auto int128::operator<(std::int64_t rhs) const noexcept -> bool
{
    if (high < 0 && rhs >= 0)
    {
        return true;
    }
    else if (high == 0 && rhs >= 0 && low < static_cast<std::uint64_t>(rhs))
    {
        return true;
    }
    else if (high == -1 && rhs < 0 && low < static_cast<std::uint64_t>(rhs))
    {
        return false;  // both are negative, so actually larger
    }

    return false;
}

constexpr auto int128::operator>(std::int64_t rhs) const noexcept -> bool
{
    return !(*this == rhs) && !(*this < rhs);
}

constexpr auto operator+(int128 lhs, int128 rhs) noexcept -> int128
{
    const auto new_low {lhs.low + rhs.low};
    const auto new_high {lhs.high + rhs.high + (new_low < lhs.low)};
    return int128{new_high, new_low};
}

constexpr auto operator-(int128 lhs, int128 rhs) noexcept -> int128
{
    const auto new_low {lhs.low - rhs.low};
    const auto new_high {lhs.high - rhs.high - (lhs.low < rhs.low ? 1 : 0)};
    return int128{new_high, new_low};
}

} // namespace detail
} // namespace decimal
} // namespace boost

// Non-standard libraries may add specializations for library-provided types
namespace std {

template <>
struct numeric_limits<boost::decimal::detail::uint128>
{
    // Member constants
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_specialized = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_signed = false;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_integer = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_exact = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool has_infinity = false;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool has_quiet_NaN = false;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool has_signaling_NaN = false;

    // These members were deprecated in C++23
    #if ((!defined(_MSC_VER) && (__cplusplus <= 202002L)) || (defined(_MSC_VER) && (_MSVC_LANG <= 202002L)))
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr std::float_denorm_style has_denorm = std::denorm_absent;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool has_denorm_loss = false;
    #endif

    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr std::float_round_style round_style = std::round_toward_zero;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_iec559 = false;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_bounded = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool is_modulo = true;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int digits = 128;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int digits10 = 38;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int max_digits10 = 0;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int radix = 2;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int min_exponent = 0;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int min_exponent10 = 0;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int max_exponent = 0;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr int max_exponent10 = 0;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool traps = std::numeric_limits<std::uint64_t>::traps;
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr bool tinyness_before = false;

    // Member functions
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto (min)        () -> boost::decimal::detail::uint128 { return 0; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto lowest       () -> boost::decimal::detail::uint128 { return 0; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto (max)        () -> boost::decimal::detail::uint128 { return {UINT64_MAX, UINT64_MAX}; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto epsilon      () -> boost::decimal::detail::uint128 { return 0; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto round_error  () -> boost::decimal::detail::uint128 { return 0; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto infinity     () -> boost::decimal::detail::uint128 { return 0; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto quiet_NaN    () -> boost::decimal::detail::uint128 { return 0; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto signaling_NaN() -> boost::decimal::detail::uint128 { return 0; }
    BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto denorm_min   () -> boost::decimal::detail::uint128 { return 0; }
};

} // namespace std

#endif // BOOST_DECIMAL_DETAIL_EMULATED128_HPP
