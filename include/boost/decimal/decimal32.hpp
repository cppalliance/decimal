// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DECIMAL32_HPP
#define BOOST_DECIMAL_DECIMAL32_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/config.hpp>
#include <iostream>
#include <limits>
#include <cstdint>
#include <cmath>

namespace boost { namespace decimal {

namespace detail {

// See section 3.5.2
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t inf_mask = 0b11110;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t nan_mask = 0b11111;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t snan_mask = 0b100000;

// Values from IEEE 754-2019 table 3.6
BOOST_ATTRIBUTE_UNUSED static constexpr auto storage_width = 32;
BOOST_ATTRIBUTE_UNUSED static constexpr auto precision = 7;
BOOST_ATTRIBUTE_UNUSED static constexpr auto emax = 96;
BOOST_ATTRIBUTE_UNUSED static constexpr auto bias = 101;
BOOST_ATTRIBUTE_UNUSED static constexpr auto combination_field_width = 11;
BOOST_ATTRIBUTE_UNUSED static constexpr auto trailing_significand_field_width = 20;

// Other useful values
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t max_significand = 9'999'999;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t max_binary_significand = 0b1001'1000100101'1001111111;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t max_hex_significand = 0x98967F;
BOOST_ATTRIBUTE_UNUSED static constexpr auto max_string_length = 15;

// Masks for the combination field since we use the binary encoding for the significand
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t g0_mask = 0b10000;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t g1_mask = 0b01000;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t g2_mask = 0b00100;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t g3_mask = 0b00010;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t g4_mask = 0b00001;

// Masks to update the significand based on the combination field
// In these first three 00, 01, or 10 are the leading 2 bits of the exp
// and the trailing 3 bits are to be concatenated onto the significand (23 bits total)
//
//    Comb.  Exponent          Significand
// s 00 TTT (00)eeeeee (0TTT)[tttttttttt][tttttttttt]
// s 01 TTT (01)eeeeee (0TTT)[tttttttttt][tttttttttt]
// s 10 TTT (10)eeeeee (0TTT)[tttttttttt][tttttttttt]
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t comb_00_mask = 0b00000;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t comb_01_mask = 0b01000;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t comb_10_mask = 0b10000;

// This mask is used to determine if we use the masks above or below since 11 TTT is invalid
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t comb_11_mask = 0b11000;

// For these masks the first two bits of the combination field imply 100 T as the
// leading bits of the significand and then bits 3 and 4 are the exp
//
//    Comb.  Exponent          Significand
// s 1100 T (00)eeeeee (100T)[tttttttttt][tttttttttt]
// s 1101 T (01)eeeeee (100T)[tttttttttt][tttttttttt]
// s 1110 T (10)eeeeee (100T)[tttttttttt][tttttttttt]
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t comb_1100_mask = 0b11000;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t comb_1101_mask = 0b11010;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t comb_1110_mask = 0b11100;

} // Namespace detail


// ISO/IEC DTR 24733
// 3.2.2 class decimal32
class decimal32 final
{
private:

    // MSVC pragma that GCC and clang also support
    #pragma pack(push, 1)
    struct data_layout_
    {
        std::uint32_t sign : 1;
        std::uint32_t combination_field : 5;
        std::uint32_t exponent : 6;
        std::uint32_t significand : 20;
    };
    #pragma pack(pop)

    data_layout_ bits_{};

public:
    // 3.2.2.1 construct/copy/destroy:
    BOOST_DECIMAL_DECL decimal32() noexcept : bits_ {} {}

    // 3.2.5 initialization from coefficient and exponent:
    BOOST_DECIMAL_DECL decimal32(long long coeff, int exp) noexcept;

    BOOST_DECIMAL_DECL friend bool signbit(decimal32 rhs) noexcept;
    BOOST_DECIMAL_DECL friend bool isinf(decimal32 rhs) noexcept;
    BOOST_DECIMAL_DECL friend bool isnan(decimal32 rhs) noexcept;
    BOOST_DECIMAL_DECL friend bool issignaling(decimal32 rhs) noexcept;
    BOOST_DECIMAL_DECL friend bool isfinite(decimal32 rhs) noexcept;

    // 3.2.7 unary arithmetic operators:
    BOOST_DECIMAL_DECL friend decimal32 operator+(decimal32 rhs) noexcept;
    BOOST_DECIMAL_DECL friend decimal32 operator-(decimal32 rhs) noexcept;

    // 3.2.9 comparison operators:
    BOOST_DECIMAL_DECL friend bool operator==(decimal32 lhs, decimal32 rhs) noexcept;
    BOOST_DECIMAL_DECL friend bool operator!=(decimal32 lhs, decimal32 rhs) noexcept;

    // 3.2.11 Formatted output:
    BOOST_DECIMAL_DECL friend std::ostream& operator<<(std::ostream& os, const decimal32& d);
};

}} // Namespace boost::decimal

namespace std {

template <>
struct numeric_limits<boost::decimal::decimal32>
{
    static constexpr bool is_specialized = true;
    static constexpr bool is_signed = true;
    static constexpr bool is_integer = false;
    static constexpr bool is_exact = false;
    static constexpr bool has_infinity = true;
    static constexpr bool has_quiet_NaN = true;
    static constexpr bool has_signaling_NaN = true;
    static constexpr std::float_denorm_style has_denorm = std::denorm_present;
    static constexpr bool has_denorm_loss = true;
    static constexpr std::float_round_style round_style = std::round_indeterminate;
    static constexpr bool is_iec559 = true;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo = false;
    static constexpr int digits = 7;
    static constexpr int digits10 = digits;
    static constexpr int max_digits10 = digits;
    static constexpr int radix = 10;
    static constexpr int min_exponent = -95;
    static constexpr int min_exponent_10 = min_exponent;
    static constexpr int max_exponent = 96;
    static constexpr int max_exponent_10 = max_exponent;
    static constexpr bool traps = numeric_limits<std::uint32_t>::traps;
    static constexpr bool tinyness_before = true;
};

} // Namespace std

#endif // BOOST_DECIMAL_DECIMAL32_HPP
