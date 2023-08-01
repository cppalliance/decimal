// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DECIMAL32_HPP
#define BOOST_DECIMAL_DECIMAL32_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/config.hpp>
#include <cstdint>

namespace boost { namespace decimal {

// ISO/IEC DTR 24733
// 3.2.2 class decimal32
class decimal32 final
{
private:
    std::uint32_t bits;

    // Construct from bits
    constexpr explicit decimal32(std::uint32_t value) : bits {value} {}

public:
    // 3.2.2.1 construct/copy/destroy:
    constexpr decimal32() noexcept : bits {} {}

    friend constexpr bool signbit(decimal32 rhs) noexcept;
    friend constexpr bool isinf(decimal32 rhs) noexcept;
    friend constexpr bool isnan(decimal32 rhs) noexcept;
    friend constexpr bool issignaling(decimal32 rhs) noexcept;
    friend constexpr bool isfinite(decimal32 rhs) noexcept;

    // 3.2.7 unary arithmetic operators:
    friend constexpr decimal32 operator+(decimal32 rhs) noexcept;
    friend constexpr decimal32 operator-(decimal32 rhs) noexcept;

    // 3.2.9 comparison operators:
    friend constexpr bool operator==(decimal32 lhs, decimal32 rhs) noexcept;
    friend constexpr bool operator!=(decimal32 lhs, decimal32 rhs) noexcept;
};

}} // Namespace boost::decimal

#endif // BOOST_DECIMAL_DECIMAL32_HPP
