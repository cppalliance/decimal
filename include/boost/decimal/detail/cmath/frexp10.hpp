// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_FREXP10_HPP
#define BOOST_DECIMAL_DETAIL_FREXP10_HPP

namespace boost {
namespace decimal {

template <typename T>
constexpr auto frexp10(T num, int* expptr) noexcept;

template <>
constexpr auto frexp10<decimal32>(decimal32 num, int* expptr) noexcept
{
    return frexp10d32(num, expptr);
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_FREXP10_HPP
