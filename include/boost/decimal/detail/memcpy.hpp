// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_MEMCPY_HPP
#define BOOST_DECIMAL_DETAIL_MEMCPY_HPP

#include <boost/decimal/detail/config.hpp>
#include <cstring>
#include <cstdint>

// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=89689
// GCC 10 added checks for length of memcpy which yields the following warning (converted to error with -Werror)
// /usr/include/x86_64-linux-gnu/bits/string_fortified.h:34:33: error: 
// ‘void* __builtin___memcpy_chk(void*, const void*, long unsigned int, long unsigned int)’ specified size between 
// 18446744071562067968 and 18446744073709551615 exceeds maximum object size 9223372036854775807 [-Werror=stringop-overflow=]
//
// memcpy is defined as taking a size_t for the count and the largest count this will recieve is the number of digits
// in a 128-bit int (39) so we can safely ignore
#if defined(__GNUC__) && __GNUC__ >= 10
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wstringop-overflow"
#  define BOOST_DECIMAL_STRINGOP_OVERFLOW_DISABLED
#endif

namespace boost {
namespace decimal {
namespace detail {

#if !defined(BOOST_DECIMAL_NO_CONSTEVAL_DETECTION)

#define BOOST_DECIMAL_CONSTEXPR constexpr

BOOST_DECIMAL_CONSTEXPR char* memcpy(char* dest, const char* src, std::size_t count)
{
    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(count))
    {
        for (std::size_t i = 0; i < count; ++i)
        {
            *(dest + i) = *(src + i);
        }

        return dest;
    }
    else
    {
        // Workaround for GCC-11 because it does not honor GCC diagnostic ignored
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53431
        // Hopefully the optimizer turns this into memcpy
        #if defined(__GNUC__) && __GNUC__ == 11
            for (std::size_t i = 0; i < count; ++i)
            {
                *(dest + i) = *(src + i);
            }

            return dest;
        #else
            return static_cast<char*>(std::memcpy(dest, src, count));
        #endif
    }
}

BOOST_DECIMAL_CONSTEXPR char* memset(char* dest, int ch, std::size_t count)
{
    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(count))
    {
        for (std::size_t i = 0; i < count; ++i)
        {
            *(dest + i) = static_cast<char>(ch);
        }

        return dest;
    }
    else
    {
        // Workaround for GCC-11 because it does not honor GCC diagnostic ignored
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53431
        // Hopefully the optimizer turns this into memset
        #if defined(__GNUC__) && __GNUC__ == 11
            for (std::size_t i = 0; i < count; ++i)
            {
                *(dest + i) = static_cast<char>(ch);
            }

            return dest;
        #else
            return static_cast<char*>(std::memset(dest, ch, count));
        #endif
    }
}

#else // No consteval detection

#define BOOST_DECIMAL_CONSTEXPR inline

BOOST_DECIMAL_CONSTEXPR void* memcpy(void* dest, const void* src, std::size_t count)
{
    return std::memcpy(dest, src, count);
}

BOOST_DECIMAL_CONSTEXPR void* memset(void* dest, int ch, std::size_t count)
{
    return std::memset(dest, ch, count);
}

#endif

} //namespace detail
} //namespace decimal
} //namespace boost

#ifdef BOOST_DECIMAL_STRINGOP_OVERFLOW_DISABLED
#  pragma GCC diagnostic pop
#endif

#endif // BOOST_DECIMAL_DETAIL_MEMCPY_HPP
