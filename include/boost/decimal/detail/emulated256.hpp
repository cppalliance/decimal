// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_EMULATED256_HPP
#define BOOST_CHARCONV_DETAIL_EMULATED256_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/emulated128.hpp>
#include <cstdint>
#include <cmath>

namespace boost {
namespace decimal {
namespace detail {

struct uint256
{
    uint128 high {};
    uint128 low {};

    constexpr uint256() = default;
    constexpr uint256& operator=(const uint256& rhs) = default;
    constexpr uint256(const uint256& rhs) = default;
    constexpr uint256(const uint128& rhs) : high {}, low {rhs} {}
    constexpr uint256(const uint128& high_, const uint128& low_) : high {high_}, low {low_} {}

    explicit operator uint128() const noexcept
    { 
        return this->low; 
    }

    explicit operator std::size_t() const noexcept
    {
        return static_cast<std::size_t>(this->low);
    }


    friend constexpr uint256 operator>>(uint256 lhs, int amount) noexcept;

    constexpr uint256 &operator>>=(int amount) noexcept
    {
        *this = *this >> amount;
        return *this;
    }

    friend constexpr uint256 operator<<(uint256 lhs, int amount) noexcept;

    constexpr uint256 &operator<<=(int amount) noexcept
    {
        *this = *this << amount;
        return *this;
    }

    friend constexpr uint256 operator|(uint256 lhs, uint256 rhs) noexcept;

    constexpr uint256 &operator|=(uint256 v) noexcept
    {
        *this = *this | v;
        return *this;
    }

    friend constexpr uint256 operator&(uint256 lhs, uint256 rhs) noexcept;

    friend constexpr uint256 operator&(uint256 lhs, uint128 rhs) noexcept;

    friend constexpr bool operator==(uint256 lhs, uint256 rhs) noexcept;

    friend constexpr bool operator==(uint256 lhs, std::uint64_t rhs) noexcept;

    friend constexpr bool operator!=(uint256 lhs, uint256 rhs) noexcept;

    friend constexpr bool operator<(uint256 lhs, uint256 rhs) noexcept;

    friend constexpr bool operator<=(uint256 lhs, uint256 rhs) noexcept;

    friend constexpr bool operator>(uint256 lhs, uint256 rhs) noexcept;

    friend constexpr bool operator>=(uint256 lhs, uint256 rhs) noexcept;

    friend constexpr uint256 operator+(uint256 lhs, uint256 rhs) noexcept;

    friend constexpr uint256 operator+(uint256 lhs, uint128 rhs) noexcept;

    friend constexpr uint256 operator*(uint256 lhs, uint256 rhs) noexcept;

    friend constexpr uint256 operator-(uint256 lhs, uint256 rhs) noexcept;

    constexpr uint256 &operator-=(uint256 v) noexcept;

    friend constexpr uint256 operator/(uint256 lhs, uint256 rhs) noexcept;

    friend constexpr uint256 operator/(uint256 lhs, std::uint64_t rhs) noexcept;

    constexpr uint256& operator/=(std::uint64_t rhs) noexcept;

    friend constexpr uint256 operator%(uint256 lhs, uint256 rhs) noexcept;

    friend constexpr uint256 operator%(uint256 lhs, std::uint64_t rhs) noexcept;

    template <typename charT, typename traits>
    friend auto operator<<(std::basic_ostream<charT, traits>& os, uint256 val) -> std::basic_ostream<charT, traits>&;

private:
    friend constexpr int high_bit(uint256 v) noexcept;

    friend constexpr void div_impl(uint256 lhs, uint256 rhs, uint256 &quotient, uint256 &remainder) noexcept;
};

constexpr uint256 operator>>(uint256 lhs, int amount) noexcept
{
    if (amount >= 128)
    {
        return {0, lhs.high >> (amount - 128)};
    }
    else if (amount == 0)
    {
        return lhs;
    }

    return {lhs.high >> amount, (lhs.low >> amount) | (lhs.high << (128 - amount))};
}

constexpr uint256 operator<<(uint256 lhs, int amount) noexcept
{
    if (amount >= 128)
    {
        return {lhs.low << (amount - 128), 0};
    }
    else if (amount == 0)
    {
        return lhs;
    }

    return {(lhs.high << amount) | (lhs.low >> (128 - amount)), lhs.low << amount};
}

constexpr uint256 operator|(uint256 lhs, uint256 rhs) noexcept
{
    return {lhs.high | rhs.high, lhs.low | rhs.low};
}

constexpr uint256 operator&(uint256 lhs, uint256 rhs) noexcept
{
    return {lhs.high & rhs.high, lhs.low & rhs.low};
}

constexpr uint256 operator&(uint256 lhs, uint128 rhs) noexcept
{
    return {lhs.high, lhs.low & rhs.low};
}

constexpr bool operator==(uint256 lhs, uint256 rhs) noexcept
{
    return lhs.high == rhs.high && lhs.low == rhs.low;
}

constexpr bool operator==(uint256 lhs, std::uint64_t rhs) noexcept
{
    return lhs.high == 0 && rhs != 0 && lhs.low == rhs;
}

constexpr bool operator!=(uint256 lhs, uint256 rhs) noexcept
{
    return !(lhs.high == rhs.high && lhs.low == rhs.low);
}

constexpr bool operator<(uint256 lhs, uint256 rhs) noexcept
{
    if (lhs.high == rhs.high)
    {
        return lhs.low < rhs.low;
    }

    return lhs.high < rhs.high;
}

constexpr bool operator<=(uint256 lhs, uint256 rhs) noexcept
{
    return !(rhs < lhs);
}

constexpr bool operator>(uint256 lhs, uint256 rhs) noexcept
{
    return rhs < lhs;
}

constexpr bool operator>=(uint256 lhs, uint256 rhs) noexcept
{
    return !(lhs < rhs);
}

constexpr uint256 operator+(uint256 lhs, uint256 rhs) noexcept
{
    const uint256 temp = {lhs.high + rhs.high, lhs.low + rhs.low};

    // Need to carry a bit into hrs
    if (temp.low < lhs.low)
    {
        return {temp.high + 1, temp.low};
    }

    return temp;
}

constexpr uint256 operator*(uint256 lhs, uint256 rhs) noexcept
{
    uint256 result{};

    uint128 products[4];

    // Multiply the low parts.
    uint128 product_low = static_cast<uint128>(lhs.low.low) * rhs.low.low;
    products[0] = {uint64_t(product_low >> 64), uint64_t(product_low)};

    // Multiply the mixed parts.
    uint128 product_mid_low = (uint128)lhs.low.high * rhs.low.low;
    uint128 product_mid_high = (uint128)lhs.low.low * rhs.low.high;
    products[1] = {uint64_t((product_mid_low >> 64) + (product_mid_high >> 64)), uint64_t(product_mid_low + product_mid_high)};

    // Multiply the high parts.
    uint128 product_high = (uint128)lhs.low.high * rhs.low.high;
    products[2] = {uint64_t(product_high >> 64), uint64_t(product_high)};

    // Add the products, taking care of the carries.
    bool carry = false;
    result.low.add_with_carry(products[0], carry);
    result.low.add_with_carry(uint128(products[1].low) << 64, carry);
    result.high.add_with_carry(uint128(products[1].high, products[1].low) >> 64, carry);
    result.high.add_with_carry(products[2], carry);

    // Note: This does not handle overflow beyond 256 bits.

    return result;
}

constexpr uint256 operator+(uint256 lhs, uint128 rhs) noexcept
{
    const uint256 temp = {lhs.high, lhs.low + rhs};

    if (temp.low < lhs.low)
    {
        return {temp.high + 1, temp.low};
    }

    return temp;
}

constexpr uint256 operator-(uint256 lhs, uint256 rhs) noexcept
{
    const uint256 temp {lhs.high - rhs.high, lhs.low - rhs.low};

    // Check for carry
    if (lhs.low < rhs.low)
    {
        return {temp.high - 1, temp.low};
    }

    return temp;
}

constexpr uint256 &uint256::operator-=(uint256 v) noexcept
{
    *this = *this - v;
    return *this;
}

constexpr uint256 operator/(uint256 lhs, uint256 rhs) noexcept
{
    uint256 quotient;
    uint256 remainder;
    div_impl(lhs, rhs, quotient, remainder);

    return quotient;
}

constexpr uint256 operator/(uint256 lhs, std::uint64_t rhs) noexcept
{
    uint256 quotient;
    uint256 remainder;
    uint256 big_rhs = {0, rhs};

    div_impl(lhs, big_rhs, quotient, remainder);

    return quotient;
}

constexpr uint256& uint256::operator/=(std::uint64_t rhs) noexcept
{
    *this = *this / rhs;
    return *this;
}

constexpr uint256 operator%(uint256 lhs, uint256 rhs) noexcept
{
    uint256 quotient;
    uint256 remainder;
    div_impl(lhs, rhs, quotient, remainder);

    return remainder;
}

constexpr uint256 operator%(uint256 lhs, std::uint64_t rhs) noexcept
{
    uint256 quotient;
    uint256 remainder;
    uint256 big_rhs = {0, rhs};

    div_impl(lhs, big_rhs, quotient, remainder);

    return remainder;
}

constexpr int high_bit(uint256 v) noexcept
{
    if (v.high != 0)
    {
        return 255 - high_bit(v.high);
    }
    else if (v.low != 0)
    {
        return 127 - high_bit(v.low);
    }

    return 0;
}

constexpr void div_impl(uint256 lhs, uint256 rhs, uint256 &quotient, uint256 &remainder) noexcept
{
    if (rhs > lhs)
    {
        quotient = {0, 0};
        remainder = {0, 0};
    }
    else if (lhs == rhs)
    {
        quotient = {0, 1};
        remainder = {0, 0};
    }

    quotient = {0, 0};

    for (int i = 255; i >= 0; --i)
    {
        // Shift quotient to the left by 1 bit
        quotient <<= 1;
        // Shift remainder to the left by 1 bit
        remainder.high <<= 1;
        remainder.low = (remainder.low << 1) | (remainder.high >> 127);
        remainder.high = remainder.high << 1;

        if (remainder >= rhs)
        {
            remainder = remainder - rhs;
            // Set the current bit of quotient
            quotient.low.low |= 1;
        }
    }
}

// Get the 256-bit result of multiplication of two 128-bit unsigned integers
constexpr uint256 umul256_impl(std::uint64_t a, std::uint64_t b, std::uint64_t c, std::uint64_t d) noexcept
{
    const auto ac = umul128(a, c);
    const auto bc = umul128(b, c);
    const auto ad = umul128(a, d);
    const auto bd = umul128(b, d);

    const auto intermediate = (bd >> 64) + static_cast<std::uint64_t>(ad) + static_cast<std::uint64_t>(bc);

    return {ac + (intermediate >> 64) + (ad >> 64) + (bc >> 64),
            (intermediate << 64) + static_cast<std::uint64_t>(bd)};
}

template<typename T>
constexpr uint256 umul256(const T &x, const uint128 &y) noexcept
{
    static_assert(sizeof(T) == 16 && (!std::numeric_limits<T>::is_signed
            #ifdef BOOST_CHARCONV_HAS_INT128
            // May not have numeric_limits specialization without gnu mode
                                      || std::is_same<T, boost::uint128_type>::value
            #endif
    ), "This function is only for 128-bit unsigned types");

    const auto a = static_cast<std::uint64_t>(x >> 64);
    const auto b = static_cast<std::uint64_t>(x);

    return umul256_impl(a, b, y.high, y.low);
}

constexpr uint256 umul256(const uint128 &x, const uint128 &y) noexcept
{
    return umul256_impl(x.high, x.low, y.high, y.low);
}

// Returns only the high 256 bits of a 256x256 multiplication
constexpr uint256 umul512_high256(const uint256 &x, const uint256 &y) noexcept
{
    const auto a = x.high;
    const auto b = x.low;
    const auto c = y.high;
    const auto d = y.low;

    const auto ac = umul256(a, c);
    const auto bc = umul256(b, c);
    const auto ad = umul256(a, d);
    const auto bd = umul256(b, d);

    const auto intermediate = (bd >> 128) + ad.high + bc.high;

    return ac + (intermediate >> 128) + (ad >> 128) + (bc >> 128);
}

auto emulated256_to_buffer(char (&buffer)[ 128 ], uint256 v)
{
    constexpr uint256 zero {0, 0};

    char* p = buffer + 128;
    *--p = '\0';

    do
    {
        *--p = "0123456789"[ static_cast<std::size_t>(v % UINT64_C(10)) ];
        v /= UINT64_C(10);
    }
    while ( v != zero );

    return p;
}

template <typename charT, typename traits>
auto operator<<(std::basic_ostream<charT, traits>& os, uint256 val) -> std::basic_ostream<charT, traits>&
{
    char buffer[128];

    os << emulated256_to_buffer(buffer, val);

    return os;
}

} //namespace detail
} //namespace decimal
} //namespace boost

namespace std {

template <>
struct numeric_limits<boost::decimal::detail::uint256>
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
    static constexpr int digits = 256;
    static constexpr int digits10 = 76;
    static constexpr int max_digits10 = 0;
    static constexpr int radix = 2;
    static constexpr int min_exponent = 0;
    static constexpr int min_exponent10 = 0;
    static constexpr int max_exponent = 0;
    static constexpr int max_exponent10 = 0;
    static constexpr bool traps = std::numeric_limits<std::uint64_t>::traps;
    static constexpr bool tinyness_before = false;

    // Member functions
    static constexpr boost::decimal::detail::uint256 (min)() { return {0, 0}; }
    static constexpr boost::decimal::detail::uint256 lowest() { return {0, 0}; }
    static constexpr boost::decimal::detail::uint256 (max)() { return {{UINT64_MAX, UINT64_MAX}, {UINT64_MAX, UINT64_MAX}}; }
    static constexpr boost::decimal::detail::uint256 epsilon() { return {0, 0}; }
    static constexpr boost::decimal::detail::uint256 round_error() { return {0, 0}; }
    static constexpr boost::decimal::detail::uint256 infinity() { return {0, 0}; }
    static constexpr boost::decimal::detail::uint256 quiet_NaN() { return {0, 0}; }
    static constexpr boost::decimal::detail::uint256 signaling_NaN() { return {0, 0}; }
    static constexpr boost::decimal::detail::uint256 denorm_min() { return {0, 0}; }
};

} // Namespace std

#endif // BOOST_CHARCONV_DETAIL_EMULATED256_HPP
