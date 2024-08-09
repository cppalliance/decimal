// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_EMULATED256_HPP
#define BOOST_DECIMAL_DETAIL_EMULATED256_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/emulated128.hpp>
#include <boost/decimal/detail/tuple.hpp>
#include <boost/decimal/detail/wide-integer/uintwide_t.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <cstdint>
#include <cmath>
#endif

namespace boost {
namespace decimal {
namespace detail {

#ifdef BOOST_DECIMAL_ENABLE_CUDA
#  pragma nv_diag_suppress 20012
#endif

struct uint256_t
{
    uint128 high {};
    uint128 low {};

    BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t() = default;
    BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t& operator=(const uint256_t& rhs) = default;
    BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t(const uint256_t& rhs) = default;
    BOOST_DECIMAL_GPU_ENABLED explicit constexpr uint256_t(const uint128& rhs) : high {}, low {rhs} {}
    BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t(const uint128& high_, const uint128& low_) : high {high_}, low {low_} {}

    BOOST_DECIMAL_GPU_ENABLED explicit operator uint128() const noexcept
    { 
        return this->low; 
    }

    BOOST_DECIMAL_GPU_ENABLED explicit operator std::size_t() const noexcept
    {
        return static_cast<std::size_t>(this->low);
    }


    BOOST_DECIMAL_GPU_ENABLED friend constexpr uint256_t operator>>(uint256_t lhs, int amount) noexcept;

    BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t &operator>>=(int amount) noexcept
    {
        *this = *this >> amount;
        return *this;
    }

    BOOST_DECIMAL_GPU_ENABLED friend constexpr uint256_t operator<<(uint256_t lhs, int amount) noexcept;

    BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t &operator<<=(int amount) noexcept
    {
        *this = *this << amount;
        return *this;
    }

    BOOST_DECIMAL_GPU_ENABLED friend constexpr uint256_t operator|(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t &operator|=(uint256_t v) noexcept
    {
        *this = *this | v;
        return *this;
    }

    BOOST_DECIMAL_GPU_ENABLED friend constexpr uint256_t operator&(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    BOOST_DECIMAL_GPU_ENABLED friend constexpr uint256_t operator&(uint256_t lhs, uint128 rhs) noexcept;

    BOOST_DECIMAL_GPU_ENABLED friend constexpr bool operator==(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    BOOST_DECIMAL_GPU_ENABLED friend constexpr bool operator==(uint256_t lhs, std::uint64_t rhs) noexcept;

    BOOST_DECIMAL_GPU_ENABLED friend constexpr bool operator!=(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    BOOST_DECIMAL_GPU_ENABLED friend constexpr bool operator<(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    BOOST_DECIMAL_GPU_ENABLED friend constexpr bool operator<=(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    BOOST_DECIMAL_GPU_ENABLED friend constexpr bool operator>(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    BOOST_DECIMAL_GPU_ENABLED friend constexpr bool operator>=(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    BOOST_DECIMAL_GPU_ENABLED friend constexpr uint256_t operator+(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    BOOST_DECIMAL_GPU_ENABLED friend constexpr uint256_t operator+(uint256_t lhs, uint128 rhs) noexcept;

    BOOST_DECIMAL_GPU_ENABLED friend constexpr uint256_t operator*(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    BOOST_DECIMAL_GPU_ENABLED friend constexpr uint256_t operator*(const uint256_t& lhs, const std::uint64_t rhs) noexcept;

    BOOST_DECIMAL_GPU_ENABLED friend constexpr uint256_t operator-(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t &operator-=(uint256_t v) noexcept;

    BOOST_DECIMAL_GPU_ENABLED friend constexpr uint256_t operator/(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    BOOST_DECIMAL_GPU_ENABLED friend constexpr uint256_t operator/(const uint256_t& lhs, std::uint64_t rhs) noexcept;

    BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t& operator/=(std::uint64_t rhs) noexcept;

    BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t& operator/=(const uint256_t& rhs) noexcept;

    BOOST_DECIMAL_GPU_ENABLED friend constexpr uint256_t operator%(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    BOOST_DECIMAL_GPU_ENABLED friend constexpr uint256_t operator%(uint256_t lhs, std::uint64_t rhs) noexcept;

    #if !defined(BOOST_DECIMAL_DISABLE_IOSTREAM)
    template <typename charT, typename traits>
    friend auto operator<<(std::basic_ostream<charT, traits>& os, uint256_t val) -> std::basic_ostream<charT, traits>&;
    #endif

private:
    BOOST_DECIMAL_GPU_ENABLED friend constexpr int high_bit(uint256_t v) noexcept;
};

#ifdef BOOST_DECIMAL_ENABLE_CUDA
#  pragma nv_diag_default 20012
#endif

BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t operator>>(uint256_t lhs, int amount) noexcept
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

BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t operator<<(uint256_t lhs, int amount) noexcept
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

BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t operator|(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    return {lhs.high | rhs.high, lhs.low | rhs.low};
}

BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t operator&(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    return {lhs.high & rhs.high, lhs.low & rhs.low};
}

BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t operator&(uint256_t lhs, uint128 rhs) noexcept
{
    return {lhs.high, lhs.low & rhs.low};
}

BOOST_DECIMAL_GPU_ENABLED constexpr bool operator==(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    return lhs.high == rhs.high && lhs.low == rhs.low;
}

BOOST_DECIMAL_GPU_ENABLED constexpr bool operator==(uint256_t lhs, std::uint64_t rhs) noexcept
{
    return lhs.high == 0 && rhs != 0 && lhs.low == rhs;
}

BOOST_DECIMAL_GPU_ENABLED constexpr bool operator!=(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    return !(lhs.high == rhs.high && lhs.low == rhs.low);
}

BOOST_DECIMAL_GPU_ENABLED constexpr bool operator<(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    if (lhs.high == rhs.high)
    {
        return lhs.low < rhs.low;
    }

    return lhs.high < rhs.high;
}

BOOST_DECIMAL_GPU_ENABLED constexpr bool operator<=(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    return !(rhs < lhs);
}

BOOST_DECIMAL_GPU_ENABLED constexpr bool operator>(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    return rhs < lhs;
}

BOOST_DECIMAL_GPU_ENABLED constexpr bool operator>=(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    return !(lhs < rhs);
}

BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t operator+(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    const uint256_t temp = {lhs.high + rhs.high, lhs.low + rhs.low};

    // Need to carry a bit into hrs
    if (temp.low < lhs.low)
    {
        return {temp.high + 1, temp.low};
    }

    return temp;
}

BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t operator+(uint256_t lhs, uint128 rhs) noexcept
{
    const uint256_t temp = {lhs.high, lhs.low + rhs};

    if (temp.low < lhs.low)
    {
        return {temp.high + 1, temp.low};
    }

    return temp;
}

BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t operator-(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    const uint256_t temp {lhs.high - rhs.high, lhs.low - rhs.low};

    // Check for carry
    if (lhs.low < rhs.low)
    {
        return {temp.high - 1, temp.low};
    }

    return temp;
}

BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t &uint256_t::operator-=(uint256_t v) noexcept
{
    *this = *this - v;
    return *this;
}

BOOST_DECIMAL_GPU_ENABLED constexpr int high_bit(uint256_t v) noexcept
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

// Function to compare two uint256_t numbers (returns -1, 0, or 1)
BOOST_DECIMAL_GPU_ENABLED constexpr int compare(const uint256_t& a, const uint256_t& b)
{
    if (a.high < b.high || (a.high == b.high && a.low < b.low))
    {
        return -1;
    }
    else if (a.high == b.high && a.low == b.low)
    {
        return 0;
    }

    return 1;
}

// The following are all needed for the division algorithm
// Function to subtract two uint256_t numbers
BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t subtract(const uint256_t& a, const uint256_t& b)
{
    uint256_t result;
    result.low = a.low - b.low;
    result.high = a.high - b.high;
    if (a.low < b.low)
    {
        result.high--;
    }

    return result;
}

// Function to left shift a uint256_t by one bit
BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t left_shift(const uint256_t& a)
{
    uint256_t result;
    result.high = (a.high << 1) | (a.low >> (sizeof(uint128) * 8 - 1));
    result.low = a.low << 1;
    return result;
}

// Function to set a specific bit of a uint256_t
BOOST_DECIMAL_GPU_ENABLED constexpr void set_bit(uint256_t& a, int bit)
{
    if (bit >= 0 && bit < 128)
    {
        a.low |= (uint128(1) << bit);
    }
    else if (bit >= 128 && bit < 256)
    {
        a.high |= (uint128(1) << (bit - 128));
    }
}

using wide_integer_uint256 = ::boost::decimal::math::wide_integer::uint256_t;

BOOST_DECIMAL_GPU_ENABLED constexpr auto uint256_to_wide_integer(const uint256_t& src) -> wide_integer_uint256
{
    wide_integer_uint256 dst { };

    using local_limb_type = typename wide_integer_uint256::limb_type;

    static_assert(sizeof(local_limb_type) == static_cast<std::size_t>(UINT8_C(4)) && std::is_same<local_limb_type, std::uint32_t>::value, "Error: Configuration of external wide-integer limbs not OK");

    dst.representation()[static_cast<std::size_t>(UINT8_C(0))] = static_cast<local_limb_type>(src.low.low);
    dst.representation()[static_cast<std::size_t>(UINT8_C(1))] = static_cast<local_limb_type>(src.low.low >> static_cast<unsigned>(UINT8_C(32)));
    dst.representation()[static_cast<std::size_t>(UINT8_C(2))] = static_cast<local_limb_type>(src.low.high);
    dst.representation()[static_cast<std::size_t>(UINT8_C(3))] = static_cast<local_limb_type>(src.low.high >> static_cast<unsigned>(UINT8_C(32)));
    dst.representation()[static_cast<std::size_t>(UINT8_C(4))] = static_cast<local_limb_type>(src.high.low);
    dst.representation()[static_cast<std::size_t>(UINT8_C(5))] = static_cast<local_limb_type>(src.high.low >> static_cast<unsigned>(UINT8_C(32)));
    dst.representation()[static_cast<std::size_t>(UINT8_C(6))] = static_cast<local_limb_type>(src.high.high);
    dst.representation()[static_cast<std::size_t>(UINT8_C(7))] = static_cast<local_limb_type>(src.high.high >> static_cast<unsigned>(UINT8_C(32)));

    return dst;
}

BOOST_DECIMAL_GPU_ENABLED constexpr auto wide_integer_to_uint256(const wide_integer_uint256& src) -> uint256_t
{
    uint256_t dst { };

    dst.low.low =
        static_cast<std::uint64_t>
        (
                                           src.crepresentation()[static_cast<std::size_t>(UINT8_C(0))]
            | static_cast<std::uint64_t>
              (
                static_cast<std::uint64_t>(src.crepresentation()[static_cast<std::size_t>(UINT8_C(1))]) << static_cast<unsigned>(UINT8_C(32))
              )
        );

    dst.low.high =
        static_cast<std::uint64_t>
        (
                                           src.crepresentation()[static_cast<std::size_t>(UINT8_C(2))]
            | static_cast<std::uint64_t>
              (
                static_cast<std::uint64_t>(src.crepresentation()[static_cast<std::size_t>(UINT8_C(3))]) << static_cast<unsigned>(UINT8_C(32))
              )
        );

    dst.high.low =
        static_cast<std::uint64_t>
        (
                                           src.crepresentation()[static_cast<std::size_t>(UINT8_C(4))]
            | static_cast<std::uint64_t>
              (
                static_cast<std::uint64_t>(src.crepresentation()[static_cast<std::size_t>(UINT8_C(5))]) << static_cast<unsigned>(UINT8_C(32))
              )
        );

    dst.high.high =
        static_cast<std::uint64_t>
        (
                                           src.crepresentation()[static_cast<std::size_t>(UINT8_C(6))]
            | static_cast<std::uint64_t>
              (
                static_cast<std::uint64_t>(src.crepresentation()[static_cast<std::size_t>(UINT8_C(7))]) << static_cast<unsigned>(UINT8_C(32))
              )
        );

    return dst;
}

BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t operator*(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    using local_unsigned_fast_type = ::boost::decimal::math::wide_integer::detail::unsigned_fast_type;

    // Mash-Up: Use unrolled school-multiplication from wide-integer (requires limb-conversions on input/output).

    auto lhs_wide = uint256_to_wide_integer(lhs);
    auto rhs_wide = uint256_to_wide_integer(rhs);

    wide_integer_uint256 result_wide { };

    wide_integer_uint256::eval_multiply_n_by_n_to_lo_part_256(result_wide.representation().begin(),
                                                              lhs_wide.crepresentation().cbegin(),
                                                              rhs_wide.crepresentation().cbegin(),
                                                              static_cast<local_unsigned_fast_type>(UINT8_C(8)));

    return wide_integer_to_uint256(result_wide);
}

BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t operator*(const uint256_t& lhs, const std::uint64_t rhs) noexcept
{
    using local_unsigned_fast_type = ::boost::decimal::math::wide_integer::detail::unsigned_fast_type;

    const auto rhs_high = static_cast<std::uint32_t>(rhs >> static_cast<unsigned>(UINT8_C(32)));

    wide_integer_uint256 result_wide { };

    const auto lhs_wide = uint256_to_wide_integer(lhs);

    if (rhs_high == UINT32_C(0))
    {
        wide_integer_uint256::eval_multiply_1d(result_wide.representation().begin(),
                                               lhs_wide.crepresentation().cbegin(),
                                               static_cast<std::uint32_t>(rhs),
                                               static_cast<local_unsigned_fast_type>(UINT8_C(8)));
    }
    else
    {
        // Mash-Up: Use unrolled school-multiplication from wide-integer (requires limb-conversions on input/output).

        auto rhs_wide = uint256_to_wide_integer(uint256_t(rhs));

        wide_integer_uint256::eval_multiply_n_by_n_to_lo_part_256(result_wide.representation().begin(),
                                                                  lhs_wide.crepresentation().cbegin(),
                                                                  rhs_wide.crepresentation().cbegin(),
                                                                  static_cast<local_unsigned_fast_type>(UINT8_C(8)));
    }

    return wide_integer_to_uint256(result_wide);
}

// Forward declaration of specialized division 256-bits / 64-bits.
BOOST_DECIMAL_GPU_ENABLED constexpr boost::decimal::tuple<uint256_t, uint256_t> divide_with_rem(const uint256_t& dividend, const std::uint64_t& divisor) noexcept;

// The division algorithm
BOOST_DECIMAL_GPU_ENABLED constexpr boost::decimal::tuple<uint256_t, uint256_t> divide(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    if ((rhs.high.high == UINT64_C(0)) && (rhs.high.low == UINT64_C(0)) && (rhs.low.high == UINT64_C(0)) && (rhs.low.low < (static_cast<std::uint64_t>(UINT64_C(0x100000000)))) && (rhs.low.low > (static_cast<std::uint64_t>(UINT64_C(0)))))
    {
        return divide_with_rem(lhs, rhs.low.low);
    }
    else
    {
        // Mash-Up: Use Knuth long-division from wide-integer (requires limb-conversions on input/output).

        auto lhs_wide = uint256_to_wide_integer(lhs);

        wide_integer_uint256 rem_wide { };

        lhs_wide.eval_divide_knuth(uint256_to_wide_integer(rhs), rem_wide);

        return
        {
            wide_integer_to_uint256(lhs_wide),
            wide_integer_to_uint256(rem_wide)
        };
    }
}

BOOST_DECIMAL_GPU_ENABLED constexpr boost::decimal::tuple<uint256_t, uint256_t> divide_with_rem(const uint256_t& dividend, const std::uint64_t& divisor) noexcept
{
    uint256_t quotient { { 0U, 0U }, { 0U, 0U }};

    uint128 current = dividend.high.high;
    quotient.high.high = static_cast<std::uint64_t>(current / divisor);
    auto remainder = static_cast<std::uint64_t>(current % divisor);

    current = static_cast<uint128>(remainder) << 64U | dividend.high.low;
    quotient.high.low = static_cast<std::uint64_t>(current / divisor);
    remainder = static_cast<std::uint64_t>(current % divisor);

    current = static_cast<uint128>(remainder) << 64U | dividend.low.high;
    quotient.low.high = static_cast<std::uint64_t>(current / divisor);
    remainder = static_cast<std::uint64_t>(current % divisor);

    current = static_cast<uint128>(remainder) << 64U | dividend.low.low;
    quotient.low.low = static_cast<std::uint64_t>(current / divisor);

    return
    {
        quotient,
        static_cast<uint256_t>(static_cast<std::uint64_t>(current % divisor))
    };
}

BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t operator/(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    const auto res {divide(lhs, rhs)};
    return std::get<0>(res);
}

BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t operator/(const uint256_t& lhs, std::uint64_t rhs) noexcept
{
    // Same code as divide_with_rem but skips the modulus step

    uint256_t quotient { { 0U, 0U }, { 0U, 0U }};

    uint128 current = lhs.high.high;
    quotient.high.high = static_cast<std::uint64_t>(current / rhs);
    auto remainder = static_cast<std::uint64_t>(current % rhs);

    current = static_cast<uint128>(remainder) << 64U | lhs.high.low;
    quotient.high.low = static_cast<std::uint64_t>(current / rhs);
    remainder = static_cast<std::uint64_t>(current % rhs);

    current = static_cast<uint128>(remainder) << 64U | lhs.low.high;
    quotient.low.high = static_cast<std::uint64_t>(current / rhs);
    remainder = static_cast<std::uint64_t>(current % rhs);

    current = static_cast<uint128>(remainder) << 64U | lhs.low.low;
    quotient.low.low = static_cast<std::uint64_t>(current / rhs);

    return quotient;
}

BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t& uint256_t::operator/=(std::uint64_t rhs) noexcept
{
    *this = *this / rhs;
    return *this;
}

BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t& uint256_t::operator/=(const uint256_t& rhs) noexcept
{
    *this = *this / rhs;
    return *this;
}

BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t operator%(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    const auto res {divide(lhs, rhs)};
    return std::get<1>(res);
}

BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t operator%(uint256_t lhs, std::uint64_t rhs) noexcept
{
    const auto res {divide(lhs, uint256_t(rhs))};
    return std::get<1>(res);
}

// Get the 256-bit result of multiplication of two 128-bit unsigned integers
BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t umul256_impl(std::uint64_t a_high, std::uint64_t a_low, std::uint64_t b_high, std::uint64_t b_low) noexcept
{
    #ifdef BOOST_DECIMAL_HAS_INT128
    using unsigned_int128_type = boost::decimal::detail::uint128_t;
    #else
    using unsigned_int128_type = boost::decimal::detail::uint128;
    #endif

    const auto low_product {static_cast<unsigned_int128_type>(a_low) * b_low};
    const auto mid_product1 {static_cast<unsigned_int128_type>(a_low) * b_high};
    const auto mid_product2 {static_cast<unsigned_int128_type>(a_high) * b_low};
    const auto high_product {static_cast<unsigned_int128_type>(a_high) * b_high};

    std::uint64_t carry {};

    const auto mid_combined {mid_product1 + mid_product2};
    if (mid_combined < mid_product1)
    {
        carry = 1;
    }

    const auto mid_combined_high {mid_combined >> 64};
    const auto mid_combined_low {mid_combined << 64};

    const auto low_sum {low_product + mid_combined_low};
    if (low_sum < low_product)
    {
        carry += 1;
    }

    uint256_t result {};
    result.low = low_sum;
    result.high = high_product + mid_combined_high + carry;

    return result;
}

template<typename T>
BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t umul256(const T &x, const uint128 &y) noexcept
{
    static_assert(sizeof(T) == 16 && (!std::numeric_limits<T>::is_signed
            #ifdef BOOST_DECIMAL_HAS_INT128
            // May not have numeric_limits specialization without gnu mode
                                      || std::is_same<T, uint128_t>::value
            #endif
    ), "This function is only for 128-bit unsigned types");

    const auto a = static_cast<std::uint64_t>(x >> 64);
    const auto b = static_cast<std::uint64_t>(x);

    return umul256_impl(a, b, y.high, y.low);
}

BOOST_DECIMAL_GPU_ENABLED constexpr uint256_t umul256(const uint128 &x, const uint128 &y) noexcept
{
    return umul256_impl(x.high, x.low, y.high, y.low);
}

inline auto emulated256_to_buffer(char (&buffer)[ 128 ], uint256_t v)
{
    constexpr uint256_t zero {0, 0};

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

#if !defined(BOOST_DECIMAL_DISABLE_IOSTREAM)
template <typename charT, typename traits>
inline auto operator<<(std::basic_ostream<charT, traits>& os, uint256_t val) -> std::basic_ostream<charT, traits>&
{
    char buffer[128];

    os << emulated256_to_buffer(buffer, val);

    return os;
}
#endif

} //namespace detail
} //namespace decimal
} //namespace boost

namespace std {

template <>
struct numeric_limits<boost::decimal::detail::uint256_t>
{
    // Member constants
    BOOST_DECIMAL_GPU_ENABLED static constexpr bool is_specialized = true;
    BOOST_DECIMAL_GPU_ENABLED static constexpr bool is_signed = false;
    BOOST_DECIMAL_GPU_ENABLED static constexpr bool is_integer = true;
    BOOST_DECIMAL_GPU_ENABLED static constexpr bool is_exact = true;
    BOOST_DECIMAL_GPU_ENABLED static constexpr bool has_infinity = false;
    BOOST_DECIMAL_GPU_ENABLED static constexpr bool has_quiet_NaN = false;
    BOOST_DECIMAL_GPU_ENABLED static constexpr bool has_signaling_NaN = false;

    // These members were deprecated in C++23
    #if ((!defined(_MSC_VER) && (__cplusplus <= 202002L)) || (defined(_MSC_VER) && (_MSVC_LANG <= 202002L)))
    BOOST_DECIMAL_GPU_ENABLED static constexpr std::float_denorm_style has_denorm = std::denorm_absent;
    BOOST_DECIMAL_GPU_ENABLED static constexpr bool has_denorm_loss = false;
    #endif

    BOOST_DECIMAL_GPU_ENABLED static constexpr std::float_round_style round_style = std::round_toward_zero;
    BOOST_DECIMAL_GPU_ENABLED static constexpr bool is_iec559 = false;
    BOOST_DECIMAL_GPU_ENABLED static constexpr bool is_bounded = true;
    BOOST_DECIMAL_GPU_ENABLED static constexpr bool is_modulo = true;
    BOOST_DECIMAL_GPU_ENABLED static constexpr int digits = 256;
    BOOST_DECIMAL_GPU_ENABLED static constexpr int digits10 = 76;
    BOOST_DECIMAL_GPU_ENABLED static constexpr int max_digits10 = 0;
    BOOST_DECIMAL_GPU_ENABLED static constexpr int radix = 2;
    BOOST_DECIMAL_GPU_ENABLED static constexpr int min_exponent = 0;
    BOOST_DECIMAL_GPU_ENABLED static constexpr int min_exponent10 = 0;
    BOOST_DECIMAL_GPU_ENABLED static constexpr int max_exponent = 0;
    BOOST_DECIMAL_GPU_ENABLED static constexpr int max_exponent10 = 0;
    BOOST_DECIMAL_GPU_ENABLED static constexpr bool traps = std::numeric_limits<std::uint64_t>::traps;
    BOOST_DECIMAL_GPU_ENABLED static constexpr bool tinyness_before = false;

    // Member functions
    BOOST_DECIMAL_GPU_ENABLED static constexpr boost::decimal::detail::uint256_t (min)() { return {0, 0}; }
    BOOST_DECIMAL_GPU_ENABLED static constexpr boost::decimal::detail::uint256_t lowest() { return {0, 0}; }
    BOOST_DECIMAL_GPU_ENABLED static constexpr boost::decimal::detail::uint256_t (max)() { return {{UINT64_MAX, UINT64_MAX}, {UINT64_MAX, UINT64_MAX}}; }
    BOOST_DECIMAL_GPU_ENABLED static constexpr boost::decimal::detail::uint256_t epsilon() { return {0, 0}; }
    BOOST_DECIMAL_GPU_ENABLED static constexpr boost::decimal::detail::uint256_t round_error() { return {0, 0}; }
    BOOST_DECIMAL_GPU_ENABLED static constexpr boost::decimal::detail::uint256_t infinity() { return {0, 0}; }
    BOOST_DECIMAL_GPU_ENABLED static constexpr boost::decimal::detail::uint256_t quiet_NaN() { return {0, 0}; }
    BOOST_DECIMAL_GPU_ENABLED static constexpr boost::decimal::detail::uint256_t signaling_NaN() { return {0, 0}; }
    BOOST_DECIMAL_GPU_ENABLED static constexpr boost::decimal::detail::uint256_t denorm_min() { return {0, 0}; }
};

} // Namespace std

#endif // BOOST_DECIMAL_DETAIL_EMULATED256_HPP
