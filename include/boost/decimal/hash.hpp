// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_HASH_HPP
#define BOOST_DECIMAL_HASH_HPP

#include <boost/decimal/decimal32.hpp>
#include <boost/decimal/decimal64.hpp>
#include <boost/decimal/decimal128.hpp>
#include <boost/decimal/detail/config.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <functional>
#include <cstring>
#endif

namespace std {

BOOST_DECIMAL_EXPORT template <>
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

BOOST_DECIMAL_EXPORT template <>
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

#if defined(__GNUC__) && __GNUC__ >= 8
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif

BOOST_DECIMAL_EXPORT template <>
struct hash<boost::decimal::decimal128>
{
    // Take the xor of the two words and hash that
    auto operator()(const boost::decimal::decimal128& v) const noexcept -> std::size_t
    {
        boost::decimal::detail::uint128 bits;
        std::memcpy(&bits, &v, sizeof(boost::decimal::detail::uint128));

        return std::hash<std::uint64_t>{}(bits.high ^ bits.low);
    }
};

#if defined(__GNUC__) && __GNUC__ >= 8
#  pragma GCC diagnostic pop
#endif

BOOST_DECIMAL_EXPORT template <>
struct hash<boost::decimal::decimal32_fast>
{
    // Need to convert into decimal32 then apply our memcpy
    auto operator()(const boost::decimal::decimal32_fast& v) const noexcept -> std::size_t
    {
        boost::decimal::decimal32 v_32 {v};
        std::uint32_t bits;
        std::memcpy(&bits, &v_32, sizeof(std::uint32_t));

        return std::hash<std::uint32_t>{}(bits);
    }
};

BOOST_DECIMAL_EXPORT template <>
struct hash<boost::decimal::decimal64_fast>
{
    // Since the underlying type is a std::uint64_t we will rely on its hash function from the STL
    // First we convert to a decimal64 so they will have the same hash value
    auto operator()(const boost::decimal::decimal64_fast& v) const noexcept -> std::size_t
    {
        boost::decimal::decimal64 v_64 {v};
        std::uint64_t bits;
        std::memcpy(&bits, &v_64, sizeof(std::uint64_t));

        return std::hash<std::uint64_t>{}(bits);
    }
};

#if defined(__GNUC__) && __GNUC__ >= 8
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif

BOOST_DECIMAL_EXPORT template <>
struct hash<boost::decimal::decimal128_fast>
{
    // Take the xor of the two words and hash that
    auto operator()(const boost::decimal::decimal128_fast& v) const noexcept -> std::size_t
    {
        boost::decimal::decimal128 v_128 {v};
        boost::decimal::detail::uint128 bits;
        std::memcpy(&bits, &v_128, sizeof(boost::decimal::detail::uint128));

        return std::hash<std::uint64_t>{}(bits.high ^ bits.low);
    }
};

#if defined(__GNUC__) && __GNUC__ >= 8
#  pragma GCC diagnostic pop
#endif

} // namespace std

#endif //BOOST_DECIMAL_HASH_HPP
