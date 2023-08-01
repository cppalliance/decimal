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

public:
    // 3.2.2.1 construct/copy/destroy:
    constexpr decimal32() noexcept : bits {} {}

    friend constexpr bool signbit(decimal32 v) noexcept;
};

}} // Namespace boost::decimal

#endif // BOOST_DECIMAL_DECIMAL32_HPP
