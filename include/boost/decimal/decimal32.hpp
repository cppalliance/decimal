// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DECIMAL32_HPP
#define BOOST_DECIMAL_DECIMAL32_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <boost/decimal/config.hpp>
#include <iostream>
#include <limits>
#include <cstdint>
#include <cmath>
#include <cassert>

namespace boost { namespace decimal {

namespace detail {

// See section 3.5.2
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t inf_mask =   0b0'11110'000000'0000000000'0000000000;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t nan_mask =   0b0'11111'000000'0000000000'0000000000;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t snan_mask =  0b0'11111'100000'0000000000'0000000000;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t comb_inf_mask = 0b11110;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t comb_nan_mask = 0b11111;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t exp_snan_mask = 0b100000;

// Values from IEEE 754-2019 table 3.6
BOOST_ATTRIBUTE_UNUSED static constexpr auto storage_width = 32;
BOOST_ATTRIBUTE_UNUSED static constexpr auto precision = 7;
BOOST_ATTRIBUTE_UNUSED static constexpr auto bias = 101;
BOOST_ATTRIBUTE_UNUSED static constexpr auto emax = 96;
BOOST_ATTRIBUTE_UNUSED static constexpr auto emin = -95;
BOOST_ATTRIBUTE_UNUSED static constexpr auto etiny = -bias;
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
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t comb_11_exp_bits = 0b00110;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t comb_11_significand_bits = 0b00001;

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

// Powers of 2 used to determine the size of the significand
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t no_combination = 0b1111111111'1111111111;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t big_combination = 0b0111'1111111111'1111111111;

// Significand field
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t significand_20_mask = no_combination;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t significand_21_mask = 0b0001'0000000000'0000000000;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t comb_1_significand_mask = 0b00001;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t significand_22_mask = 0b0010'0000000000'0000000000;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t comb_2_significand_mask = 0b00010;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t significand_23_mask = 0b0100'0000000000'0000000000;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t comb_3_significand_mask = 0b00100;

// Exponent fields
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t max_exp_no_combination = 0b111111;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t exp_combination_field_mask = max_exp_no_combination;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t exp_one_combination = 0b1'111111;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t max_biased_exp = 0b10'111111;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t small_combination_field_mask = 0b0000'0000'0111'0000'0000'0000'0000'0000;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t big_combination_field_mask = 0b0000'0000'0001'0000'0000'0000'0000'0000;

// Constexpr construction from an uint32_t without having to memcpy
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t construct_sign_mask = 0b1'00000'000000'0000000000'0000000000;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t construct_combination_mask = 0b0'11111'000000'0000000000'0000000000;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t construct_exp_mask = 0b0'00000'111111'0000000000'0000000000;
BOOST_ATTRIBUTE_UNUSED static constexpr std::uint32_t construct_significand_mask = no_combination;

} // Namespace detail

constexpr void normalize(std::uint32_t& significand, std::uint32_t& exp) noexcept
{
    auto digits = detail::num_digits(significand);

    while (digits < detail::precision)
    {
        significand *= 10;
        --exp;
        ++digits;
    }
}

// ISO/IEC DTR 24733
// 3.2.2 class decimal32
class decimal32 final
{
private:

    // MSVC pragma that GCC and clang also support
    #pragma pack(push, 1)
    struct data_layout_
    {
        #ifdef BOOST_DECIMAL_ENDIAN_LITTLE_BYTE

        std::uint32_t significand : 20;
        std::uint32_t exponent : 6;
        std::uint32_t combination_field : 5;
        std::uint32_t sign : 1;

        #else

        std::uint32_t sign : 1;
        std::uint32_t combination_field : 5;
        std::uint32_t exponent : 6;
        std::uint32_t significand : 20;

        #endif
    };
    #pragma pack(pop)

    data_layout_ bits_{};

    constexpr std::uint32_t full_exponent() const noexcept;
    constexpr std::uint32_t full_significand() const noexcept;

public:
    // 3.2.2.1 construct/copy/destroy:
    BOOST_DECIMAL_DECL constexpr decimal32() noexcept : bits_ {} {}

    // 3.2.5 initialization from coefficient and exponent:
    template <typename T>
    BOOST_DECIMAL_DECL constexpr decimal32(T coeff, int exp) noexcept;

    BOOST_DECIMAL_DECL constexpr decimal32(std::uint32_t bits) noexcept;

    BOOST_DECIMAL_DECL friend constexpr bool signbit(decimal32 rhs) noexcept;
    BOOST_DECIMAL_DECL friend constexpr bool isinf(decimal32 rhs) noexcept;
    BOOST_DECIMAL_DECL friend constexpr bool isnan(decimal32 rhs) noexcept;
    BOOST_DECIMAL_DECL friend constexpr bool issignaling(decimal32 rhs) noexcept;
    BOOST_DECIMAL_DECL friend constexpr bool isfinite(decimal32 rhs) noexcept;

    // 3.2.7 unary arithmetic operators:
    BOOST_DECIMAL_DECL friend constexpr decimal32 operator+(decimal32 rhs) noexcept;
    BOOST_DECIMAL_DECL friend constexpr decimal32 operator-(decimal32 rhs) noexcept;

    // 3.2.9 comparison operators:
    BOOST_DECIMAL_DECL friend constexpr bool operator==(decimal32 lhs, decimal32 rhs) noexcept;
    BOOST_DECIMAL_DECL friend constexpr bool operator!=(decimal32 lhs, decimal32 rhs) noexcept;
    BOOST_DECIMAL_DECL friend constexpr bool operator<(decimal32 lhs, decimal32 rhs) noexcept;
    BOOST_DECIMAL_DECL friend constexpr bool operator<=(decimal32 lhs, decimal32 rhs) noexcept;
    BOOST_DECIMAL_DECL friend constexpr bool operator>(decimal32 lhs, decimal32 rhs) noexcept;
    BOOST_DECIMAL_DECL friend constexpr bool operator>=(decimal32 lhs, decimal32 rhs) noexcept;

    // 3.2.11 Formatted output:
    BOOST_DECIMAL_DECL friend std::ostream& operator<<(std::ostream& os, const decimal32& d);

    // Debug bit pattern
    BOOST_DECIMAL_DECL friend std::uint32_t to_bits(decimal32 rhs) noexcept;
};

template <typename T>
constexpr decimal32::decimal32(T coeff, int exp) noexcept
{
    bits_.sign = coeff < 0;
    std::uint32_t unsigned_coeff = bits_.sign ? -(static_cast<std::uint32_t>(coeff)) : static_cast<std::uint32_t>(coeff);

    // If the coeff is not in range make it so
    while (unsigned_coeff > detail::max_significand)
    {
        unsigned_coeff /= 10;
        ++exp;
    }

    // zero the combination field, so we can mask in the following
    bits_.combination_field = 0;
    bits_.significand = 0;
    bool big_combination = false;

    if (unsigned_coeff < detail::no_combination)
    {
        // If the coefficient fits directly we don't need to use the combination field
        bits_.significand = unsigned_coeff;
    }
    else if (unsigned_coeff < detail::big_combination)
    {
        // Break the number into 3 bits for the combination field and 20 bits for the significand field

        // Use the least significant 20 bits to set the significand
        bits_.significand = unsigned_coeff & detail::no_combination;

        // Now set the combination field (maximum of 3 bits)
        uint32_t remaining_bits = unsigned_coeff & detail::small_combination_field_mask;
        remaining_bits >>= 20;
        assert(remaining_bits <= 3); // Only allowed 00, 01, or 10

        bits_.combination_field |= remaining_bits;
    }
    else
    {
        // Have to use the full combination field
        bits_.combination_field |= detail::comb_11_mask;
        big_combination = true;

        bits_.significand = unsigned_coeff & detail::no_combination;
        const uint32_t remaining_bit = unsigned_coeff & detail::big_combination_field_mask;
        assert(remaining_bit <= 1);

        if (remaining_bit)
        {
            bits_.combination_field |= 1U;
        }
    }

    // If the exponent fits we do not need to use the combination field
    const std::uint32_t biased_exp = exp + detail::bias;
    const std::uint32_t biased_exp_low_six = biased_exp & detail::exp_combination_field_mask;
    if (biased_exp <= detail::max_exp_no_combination)
    {
        bits_.exponent = biased_exp;
    }
    else if (biased_exp < detail::exp_one_combination)
    {
        if (big_combination)
        {
            bits_.combination_field |= detail::comb_1101_mask;
        }
        else
        {
            bits_.combination_field |= detail::comb_01_mask;
        }

        bits_.exponent = biased_exp_low_six;
    }
    else if (biased_exp <= detail::max_biased_exp)
    {
        if (big_combination)
        {
            bits_.combination_field |= detail::comb_1110_mask;
        }
        else
        {
            bits_.combination_field |= detail::comb_10_mask;
        }

        bits_.exponent = biased_exp_low_six;
    }
    else
    {
        // The value is infinity
        bits_.combination_field = detail::comb_inf_mask;
    }

}

constexpr decimal32::decimal32(std::uint32_t bits) noexcept
{
    bits_.exponent = (bits & detail::construct_sign_mask) >> 31;
    bits_.combination_field = (bits & detail::construct_combination_mask) >> 26;
    bits_.exponent = (bits & detail::construct_exp_mask) >> 20;
    bits_.significand = bits & detail::construct_significand_mask;
}

constexpr bool signbit(decimal32 rhs) noexcept
{
    return rhs.bits_.sign;
}

constexpr bool isinf(decimal32 rhs) noexcept
{
    return (rhs.bits_.combination_field & detail::comb_inf_mask) == detail::comb_inf_mask;
}

constexpr bool isnan(decimal32 rhs) noexcept
{
    return (rhs.bits_.combination_field & detail::comb_nan_mask) == detail::comb_nan_mask;
}

constexpr bool issignaling(decimal32 rhs) noexcept
{
    return isnan(rhs) && (rhs.bits_.exponent & detail::exp_snan_mask) == detail::exp_snan_mask;
}

constexpr bool isfinite(decimal32 rhs) noexcept
{
    return !isinf(rhs) && !isnan(rhs);
}

constexpr decimal32 operator+(decimal32 rhs) noexcept
{
    return rhs;
}

constexpr decimal32 operator-(decimal32 rhs) noexcept
{
    rhs.bits_.sign ^= 1;
    return rhs;
}

constexpr bool operator==(decimal32 lhs, decimal32 rhs) noexcept
{
    if (lhs.bits_.sign != rhs.bits_.sign)
    {
        return false;
    }

    std::uint32_t lhs_real_exp = lhs.full_exponent();
    std::uint32_t rhs_real_exp = rhs.full_exponent();
    std::uint32_t lhs_significand = lhs.full_significand();
    std::uint32_t rhs_significand = rhs.full_significand();

    // Normalize the significands
    normalize(lhs_significand, lhs_real_exp);
    normalize(rhs_significand, rhs_real_exp);

    return lhs_real_exp == rhs_real_exp &&
           lhs_significand == rhs_significand;
}

constexpr bool operator!=(decimal32 lhs, decimal32 rhs) noexcept
{
    return !(lhs == rhs);
}

constexpr bool operator<(decimal32 lhs, decimal32 rhs) noexcept
{
    if (isnan(lhs) || isnan(rhs))
    {
        return false;
    }
    else if (lhs.bits_.sign && !rhs.bits_.sign)
    {
        return true;
    }
    else if (!lhs.bits_.sign && rhs.bits_.sign)
    {
        return false;
    }
    else if (isfinite(lhs) && isinf(rhs))
    {
        if (!signbit(rhs))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    std::uint32_t lhs_real_exp = lhs.full_exponent();
    std::uint32_t rhs_real_exp = rhs.full_exponent();
    std::uint32_t lhs_significand = lhs.full_significand();
    std::uint32_t rhs_significand = rhs.full_significand();

    // Normalize the significands
    normalize(lhs_significand, lhs_real_exp);
    normalize(rhs_significand, rhs_real_exp);

    if (lhs_real_exp < rhs_real_exp)
    {
        return true;
    }
    else if (lhs_real_exp > rhs_real_exp)
    {
        return false;
    }

    // exponents are equal
    return lhs_significand < rhs_significand;
}

constexpr bool operator<=(decimal32 lhs, decimal32 rhs) noexcept
{
    return !(rhs < lhs);
}

constexpr bool operator>(decimal32 lhs, decimal32 rhs) noexcept
{
    return rhs < lhs;
}

constexpr bool operator>=(decimal32 lhs, decimal32 rhs) noexcept
{
    return !(lhs < rhs);
}

constexpr std::uint32_t decimal32::full_exponent() const noexcept
{
    std::uint32_t exp = 0;

    if ((bits_.combination_field & detail::comb_11_mask) == 0b11000)
    {
        // bits 2 and 3 are the exp part of the combination field
        exp |= (bits_.combination_field & detail::comb_11_exp_bits) << 5;
    }
    else
    {
        // bits 0 and 1 are the exp part of the combination field
        exp |= (bits_.combination_field & detail::comb_11_mask) << 3;
    }

    exp |= bits_.exponent;

    return exp;
}

constexpr std::uint32_t decimal32::full_significand() const noexcept
{
    std::uint32_t significand = 0;

    if ((bits_.combination_field & detail::comb_11_mask) == 0b11000)
    {
        // Only need the one bit of T because the other 3 are implied
        if (bits_.combination_field & 1U)
        {
            significand = 0b1001'0000000000'0000000000;
        }
        else
        {
            significand = 0b1000'0000000000'0000000000;
        }
    }
    else
    {
        significand |= ((bits_.combination_field & 0b00111) << 20);
    }

    significand |= bits_.significand;

    return significand;
}

}} // Namespace boost::decimal

namespace std {

template <>
struct numeric_limits<boost::decimal::decimal32>
{
    BOOST_ATTRIBUTE_UNUSED static constexpr bool is_specialized = true;
    BOOST_ATTRIBUTE_UNUSED static constexpr bool is_signed = true;
    BOOST_ATTRIBUTE_UNUSED static constexpr bool is_integer = false;
    BOOST_ATTRIBUTE_UNUSED static constexpr bool is_exact = false;
    BOOST_ATTRIBUTE_UNUSED static constexpr bool has_infinity = true;
    BOOST_ATTRIBUTE_UNUSED static constexpr bool has_quiet_NaN = true;
    BOOST_ATTRIBUTE_UNUSED static constexpr bool has_signaling_NaN = true;
    BOOST_ATTRIBUTE_UNUSED static constexpr std::float_denorm_style has_denorm = std::denorm_present;
    BOOST_ATTRIBUTE_UNUSED static constexpr bool has_denorm_loss = true;
    BOOST_ATTRIBUTE_UNUSED static constexpr std::float_round_style round_style = std::round_indeterminate;
    BOOST_ATTRIBUTE_UNUSED static constexpr bool is_iec559 = true;
    BOOST_ATTRIBUTE_UNUSED static constexpr bool is_bounded = true;
    BOOST_ATTRIBUTE_UNUSED static constexpr bool is_modulo = false;
    BOOST_ATTRIBUTE_UNUSED static constexpr int digits = 7;
    BOOST_ATTRIBUTE_UNUSED static constexpr int digits10 = digits;
    BOOST_ATTRIBUTE_UNUSED static constexpr int max_digits10 = digits;
    BOOST_ATTRIBUTE_UNUSED static constexpr int radix = 10;
    BOOST_ATTRIBUTE_UNUSED static constexpr int min_exponent = -95;
    BOOST_ATTRIBUTE_UNUSED static constexpr int min_exponent_10 = min_exponent;
    BOOST_ATTRIBUTE_UNUSED static constexpr int max_exponent = 96;
    BOOST_ATTRIBUTE_UNUSED static constexpr int max_exponent_10 = max_exponent;
    BOOST_ATTRIBUTE_UNUSED static constexpr bool traps = numeric_limits<std::uint32_t>::traps;
    BOOST_ATTRIBUTE_UNUSED static constexpr bool tinyness_before = true;

    // Member functions
    BOOST_ATTRIBUTE_UNUSED static constexpr boost::decimal::decimal32 (min)() { return {1, min_exponent}; }
    BOOST_ATTRIBUTE_UNUSED static constexpr boost::decimal::decimal32 (max)() { return {9999999, max_exponent}; }
    BOOST_ATTRIBUTE_UNUSED static constexpr boost::decimal::decimal32 lowest() { return {-9999999, max_exponent}; }
    BOOST_ATTRIBUTE_UNUSED static constexpr boost::decimal::decimal32 epsilon() { return {1, -7}; }
    BOOST_ATTRIBUTE_UNUSED static constexpr boost::decimal::decimal32 round_error() { return epsilon(); }
    BOOST_ATTRIBUTE_UNUSED static constexpr boost::decimal::decimal32 infinity() { return {boost::decimal::detail::inf_mask}; }
    BOOST_ATTRIBUTE_UNUSED static constexpr boost::decimal::decimal32 quiet_NaN() { return {boost::decimal::detail::nan_mask}; }
    BOOST_ATTRIBUTE_UNUSED static constexpr boost::decimal::decimal32 signaling_NaN() { return {boost::decimal::detail::snan_mask}; }
    BOOST_ATTRIBUTE_UNUSED static constexpr boost::decimal::decimal32 denorm_min() { return {1, boost::decimal::detail::etiny}; }
};

} // Namespace std

#endif // BOOST_DECIMAL_DECIMAL32_HPP
