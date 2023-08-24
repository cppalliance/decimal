// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CONFIG_HPP
#define BOOST_DECIMAL_DETAIL_CONFIG_HPP

#include <boost/config.hpp>

// Determine endianness
#if defined(_WIN32)

#define BOOST_DECIMAL_ENDIAN_BIG_BYTE 0
#define BOOST_DECIMAL_ENDIAN_LITTLE_BYTE 1

#elif defined(__BYTE_ORDER__)

#define BOOST_DECIMAL_ENDIAN_BIG_BYTE (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define BOOST_DECIMAL_ENDIAN_LITTLE_BYTE (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

#else

#error Could not determine endian type. Please file an issue at https://github.com/cppalliance/decimal with your architecture

#endif // Determine endianness

#if __has_include(<bit>)
#  if __cplusplus >= 201806L || _MSVC_LANG >= 201806L
#    include <bit>
#    define BOOST_DECIMAL_HAS_CONSTEXPR_BITCAST
#  endif
#endif

#ifdef BOOST_DECIMAL_HAS_CONSTEXPR_BITCAST
#  define BOOST_DECIMAL_CXX20_CONSTEXPR constexpr
#endif

#ifndef BOOST_DECIMAL_CXX20_CONSTEXPR
#  define BOOST_DECIMAL_CXX20_CONSTEXPR
#endif

#endif // BOOST_DECIMAL_DETAIL_CONFIG_HPP
