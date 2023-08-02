// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DECIMAL32_HPP
#define BOOST_DECIMAL_DECIMAL32_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/config.hpp>
#include <limits>
#include <cstdint>

namespace boost { namespace decimal {

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

    data_layout_ bits_;

public:
    // 3.2.2.1 construct/copy/destroy:
    decimal32() noexcept : bits_ {} {}

    // 3.2.5 initialization from coefficient and exponent:
    decimal32(long long coeff, int exp) noexcept;

    friend bool signbit(decimal32 rhs) noexcept;
    friend bool isinf(decimal32 rhs) noexcept;
    friend bool isnan(decimal32 rhs) noexcept;
    friend bool issignaling(decimal32 rhs) noexcept;
    friend bool isfinite(decimal32 rhs) noexcept;

    // 3.2.7 unary arithmetic operators:
    friend decimal32 operator+(decimal32 rhs) noexcept;
    friend decimal32 operator-(decimal32 rhs) noexcept;

    // 3.2.9 comparison operators:
    friend bool operator==(decimal32 lhs, decimal32 rhs) noexcept;
    friend bool operator!=(decimal32 lhs, decimal32 rhs) noexcept;
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
