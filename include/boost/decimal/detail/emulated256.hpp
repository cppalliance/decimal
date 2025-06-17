// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_EMULATED256_HPP
#define BOOST_DECIMAL_DETAIL_EMULATED256_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/wide-integer/uintwide_t.hpp>
#include <boost/int128.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <cstdint>
#include <cmath>
#include <tuple>
#endif

namespace boost {
namespace decimal {
namespace detail {

struct uint256_t
{
    boost::int128::uint128_t high {};
    boost::int128::uint128_t low {};

    constexpr uint256_t() = default;
    constexpr uint256_t& operator=(const uint256_t& rhs) = default;
    constexpr uint256_t(const uint256_t& rhs) = default;
    explicit constexpr uint256_t(const boost::int128::uint128_t& rhs) : high {}, low {rhs} {}
    constexpr uint256_t(const boost::int128::uint128_t& high_, const boost::int128::uint128_t& low_) : high {high_}, low {low_} {}

    explicit operator boost::int128::uint128_t() const noexcept
    { 
        return this->low; 
    }

    explicit operator std::size_t() const noexcept
    {
        return static_cast<std::size_t>(this->low);
    }


    friend constexpr uint256_t operator>>(uint256_t lhs, int amount) noexcept;

    constexpr uint256_t &operator>>=(int amount) noexcept
    {
        *this = *this >> amount;
        return *this;
    }

    friend constexpr uint256_t operator<<(uint256_t lhs, int amount) noexcept;

    constexpr uint256_t &operator<<=(int amount) noexcept
    {
        *this = *this << amount;
        return *this;
    }

    friend constexpr uint256_t operator|(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    constexpr uint256_t &operator|=(uint256_t v) noexcept
    {
        *this = *this | v;
        return *this;
    }

    friend constexpr uint256_t operator&(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    friend constexpr uint256_t operator&(uint256_t lhs, boost::int128::uint128_t rhs) noexcept;

    friend constexpr bool operator==(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    friend constexpr bool operator==(uint256_t lhs, std::uint64_t rhs) noexcept;

    friend constexpr bool operator!=(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    friend constexpr bool operator<(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    friend constexpr bool operator<=(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    friend constexpr bool operator>(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    friend constexpr bool operator>=(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    friend constexpr uint256_t operator+(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    friend constexpr uint256_t operator+(uint256_t lhs, boost::int128::uint128_t rhs) noexcept;

    friend constexpr uint256_t operator*(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    friend constexpr uint256_t operator*(const uint256_t& lhs, const std::uint64_t rhs) noexcept;

    friend constexpr uint256_t operator-(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    constexpr uint256_t &operator-=(uint256_t v) noexcept;

    friend constexpr uint256_t operator/(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    friend constexpr uint256_t operator/(const uint256_t& lhs, std::uint64_t rhs) noexcept;

    constexpr uint256_t& operator/=(std::uint64_t rhs) noexcept;

    constexpr uint256_t& operator/=(const uint256_t& rhs) noexcept;

    friend constexpr uint256_t operator%(const uint256_t& lhs, const uint256_t& rhs) noexcept;

    friend constexpr uint256_t operator%(uint256_t lhs, std::uint64_t rhs) noexcept;

    #if !defined(BOOST_DECIMAL_DISABLE_IOSTREAM)
    template <typename charT, typename traits>
    friend auto operator<<(std::basic_ostream<charT, traits>& os, uint256_t val) -> std::basic_ostream<charT, traits>&;
    #endif

private:
    friend constexpr int high_bit(uint256_t v) noexcept;
};

constexpr uint256_t operator>>(uint256_t lhs, int amount) noexcept
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

constexpr uint256_t operator<<(uint256_t lhs, int amount) noexcept
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

constexpr uint256_t operator|(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    return {lhs.high | rhs.high, lhs.low | rhs.low};
}

constexpr uint256_t operator&(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    return {lhs.high & rhs.high, lhs.low & rhs.low};
}

constexpr uint256_t operator&(uint256_t lhs, boost::int128::uint128_t rhs) noexcept
{
    return {lhs.high, lhs.low & rhs.low};
}

constexpr bool operator==(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    return lhs.high == rhs.high && lhs.low == rhs.low;
}

constexpr bool operator==(uint256_t lhs, std::uint64_t rhs) noexcept
{
    return lhs.high == 0 && rhs != 0 && lhs.low == rhs;
}

constexpr bool operator!=(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    return !(lhs.high == rhs.high && lhs.low == rhs.low);
}

constexpr bool operator<(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    if (lhs.high == rhs.high)
    {
        return lhs.low < rhs.low;
    }

    return lhs.high < rhs.high;
}

constexpr bool operator<=(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    return !(rhs < lhs);
}

constexpr bool operator>(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    return rhs < lhs;
}

constexpr bool operator>=(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    return !(lhs < rhs);
}

constexpr uint256_t operator+(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    const uint256_t temp = {lhs.high + rhs.high, lhs.low + rhs.low};

    // Need to carry a bit into hrs
    if (temp.low < lhs.low)
    {
        return {temp.high + 1, temp.low};
    }

    return temp;
}

constexpr uint256_t operator+(uint256_t lhs, boost::int128::uint128_t rhs) noexcept
{
    const uint256_t temp = {lhs.high, lhs.low + rhs};

    if (temp.low < lhs.low)
    {
        return {temp.high + 1, temp.low};
    }

    return temp;
}

constexpr uint256_t operator-(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    const uint256_t temp {lhs.high - rhs.high, lhs.low - rhs.low};

    // Check for carry
    if (lhs.low < rhs.low)
    {
        return {temp.high - 1, temp.low};
    }

    return temp;
}

constexpr uint256_t &uint256_t::operator-=(uint256_t v) noexcept
{
    *this = *this - v;
    return *this;
}

constexpr auto high_bit(const boost::int128::uint128_t v) noexcept -> int
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

constexpr int high_bit(uint256_t v) noexcept
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
constexpr int compare(const uint256_t& a, const uint256_t& b)
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
constexpr uint256_t subtract(const uint256_t& a, const uint256_t& b)
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
constexpr uint256_t left_shift(const uint256_t& a)
{
    uint256_t result;
    result.high = (a.high << 1) | (a.low >> (sizeof(boost::int128::uint128_t) * 8 - 1));
    result.low = a.low << 1;
    return result;
}

// Function to set a specific bit of a uint256_t
constexpr void set_bit(uint256_t& a, int bit)
{
    if (bit >= 0 && bit < 128)
    {
        a.low |= (boost::int128::uint128_t(1) << bit);
    }
    else if (bit >= 128 && bit < 256)
    {
        a.high |= (boost::int128::uint128_t(1) << (bit - 128));
    }
}

using wide_integer_uint256 = ::boost::decimal::math::wide_integer::uint256_t;

constexpr auto uint256_to_wide_integer(const uint256_t& src) -> wide_integer_uint256
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

constexpr auto wide_integer_to_uint256(const wide_integer_uint256& src) -> uint256_t
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

constexpr uint256_t operator*(const uint256_t& lhs, const uint256_t& rhs) noexcept
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

constexpr uint256_t operator*(const uint256_t& lhs, const std::uint64_t rhs) noexcept
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
constexpr std::tuple<uint256_t, uint256_t> divide_with_rem(const uint256_t& dividend, const std::uint64_t& divisor) noexcept;

// The division algorithm
constexpr std::tuple<uint256_t, uint256_t> divide(const uint256_t& lhs, const uint256_t& rhs) noexcept
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

constexpr std::tuple<uint256_t, uint256_t> divide_with_rem(const uint256_t& dividend, const std::uint64_t& divisor) noexcept
{
    uint256_t quotient { { 0U, 0U }, { 0U, 0U }};

    boost::int128::uint128_t current = dividend.high.high;
    quotient.high.high = static_cast<std::uint64_t>(current / divisor);
    auto remainder = static_cast<std::uint64_t>(current % divisor);

    current = static_cast<boost::int128::uint128_t>(remainder) << 64U | dividend.high.low;
    quotient.high.low = static_cast<std::uint64_t>(current / divisor);
    remainder = static_cast<std::uint64_t>(current % divisor);

    current = static_cast<boost::int128::uint128_t>(remainder) << 64U | dividend.low.high;
    quotient.low.high = static_cast<std::uint64_t>(current / divisor);
    remainder = static_cast<std::uint64_t>(current % divisor);

    current = static_cast<boost::int128::uint128_t>(remainder) << 64U | dividend.low.low;
    quotient.low.low = static_cast<std::uint64_t>(current / divisor);

    return
    {
        quotient,
        static_cast<uint256_t>(static_cast<std::uint64_t>(current % divisor))
    };
}

constexpr uint256_t operator/(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    const auto res {divide(lhs, rhs)};
    return std::get<0>(res);
}

constexpr uint256_t operator/(const uint256_t& lhs, std::uint64_t rhs) noexcept
{
    // Same code as divide_with_rem but skips the modulus step

    uint256_t quotient { { 0U, 0U }, { 0U, 0U }};

    boost::int128::uint128_t current = lhs.high.high;
    quotient.high.high = static_cast<std::uint64_t>(current / rhs);
    auto remainder = static_cast<std::uint64_t>(current % rhs);

    current = static_cast<boost::int128::uint128_t>(remainder) << 64U | lhs.high.low;
    quotient.high.low = static_cast<std::uint64_t>(current / rhs);
    remainder = static_cast<std::uint64_t>(current % rhs);

    current = static_cast<boost::int128::uint128_t>(remainder) << 64U | lhs.low.high;
    quotient.low.high = static_cast<std::uint64_t>(current / rhs);
    remainder = static_cast<std::uint64_t>(current % rhs);

    current = static_cast<boost::int128::uint128_t>(remainder) << 64U | lhs.low.low;
    quotient.low.low = static_cast<std::uint64_t>(current / rhs);

    return quotient;
}

constexpr uint256_t& uint256_t::operator/=(std::uint64_t rhs) noexcept
{
    *this = *this / rhs;
    return *this;
}

constexpr uint256_t& uint256_t::operator/=(const uint256_t& rhs) noexcept
{
    *this = *this / rhs;
    return *this;
}

constexpr uint256_t operator%(const uint256_t& lhs, const uint256_t& rhs) noexcept
{
    const auto res {divide(lhs, rhs)};
    return std::get<1>(res);
}

constexpr uint256_t operator%(uint256_t lhs, std::uint64_t rhs) noexcept
{
    const auto res {divide(lhs, uint256_t(rhs))};
    return std::get<1>(res);
}

// Get the 256-bit result of multiplication of two 128-bit unsigned integers
constexpr uint256_t umul256_impl(std::uint64_t a_high, std::uint64_t a_low, std::uint64_t b_high, std::uint64_t b_low) noexcept
{
    #ifdef BOOST_DECIMAL_HAS_INT128
    using unsigned_int128_type = boost::decimal::detail::boost::int128::uint128_t_t;
    #else
    using unsigned_int128_type = boost::decimal::detail::boost::int128::uint128_t;
    #endif

    const auto low_product {static_cast<unsigned_int128_type>(a_low) * b_low};
    const auto mid_product1 {static_cast<unsigned_int128_type>(a_low) * b_high};
    const auto mid_product2 {static_cast<unsigned_int128_type>(a_high) * b_low};
    const auto high_product {static_cast<unsigned_int128_type>(a_high) * b_high};

    const auto mid_sum {mid_product1 + mid_product2};
    bool carry {mid_sum < mid_product1};

    const auto low_sum {low_product + (mid_sum << 64)};
    carry |= (low_sum < low_product);

    uint256_t result {};
    result.low = low_sum;
    result.high = high_product + (mid_sum >> 64) + carry;

    return result;
}

template<typename T>
constexpr uint256_t umul256(const T &x, const boost::int128::uint128_t &y) noexcept
{
    static_assert(sizeof(T) == 16 && (!std::numeric_limits<T>::is_signed
            #ifdef BOOST_DECIMAL_HAS_INT128
            // May not have numeric_limits specialization without gnu mode
                                      || std::is_same<T, boost::int128::uint128_t_t>::value
            #endif
    ), "This function is only for 128-bit unsigned types");

    const auto a = static_cast<std::uint64_t>(x >> 64);
    const auto b = static_cast<std::uint64_t>(x);

    return umul256_impl(a, b, y.high, y.low);
}

constexpr uint256_t umul256(const boost::int128::uint128_t &x, const boost::int128::uint128_t &y) noexcept
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
    static constexpr boost::decimal::detail::uint256_t (min)() { return {0, 0}; }
    static constexpr boost::decimal::detail::uint256_t lowest() { return {0, 0}; }
    static constexpr boost::decimal::detail::uint256_t (max)() { return {{UINT64_MAX, UINT64_MAX}, {UINT64_MAX, UINT64_MAX}}; }
    static constexpr boost::decimal::detail::uint256_t epsilon() { return {0, 0}; }
    static constexpr boost::decimal::detail::uint256_t round_error() { return {0, 0}; }
    static constexpr boost::decimal::detail::uint256_t infinity() { return {0, 0}; }
    static constexpr boost::decimal::detail::uint256_t quiet_NaN() { return {0, 0}; }
    static constexpr boost::decimal::detail::uint256_t signaling_NaN() { return {0, 0}; }
    static constexpr boost::decimal::detail::uint256_t denorm_min() { return {0, 0}; }
};

} // Namespace std

#endif // BOOST_DECIMAL_DETAIL_EMULATED256_HPP
