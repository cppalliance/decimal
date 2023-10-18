// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_HASH_HPP
#define BOOST_DECIMAL_HASH_HPP

#include <boost/decimal/decimal32.hpp>
#include <boost/decimal/decimal64.hpp>
#include <functional>
#include <cstring>

namespace std {

template <>
struct hash<boost::decimal::decimal32>
{
    // Since the underlying type is a std::uint32_t we will rely on its hash function from the STL
    auto operator()(const boost::decimal::decimal32& v) const noexcept -> std::size_t
    {
        std::uint32_t bits;
        std::memcpy(&bits, &v, sizeof(std::uint32_t));

        return std::hash<std::uint32_t>{}(bits);
    }
};

template <>
struct hash<boost::decimal::decimal64>
{
    // Since the underlying type is a std::uint64_t we will rely on its hash function from the STL
    auto operator()(const boost::decimal::decimal64& v) const noexcept -> std::size_t
    {
        std::uint64_t bits;
        std::memcpy(&bits, &v, sizeof(std::uint64_t));

        return std::hash<std::uint64_t>{}(bits);
    }
};

}

#endif //BOOST_DECIMAL_HASH_HPP
