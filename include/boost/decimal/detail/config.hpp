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

// Include intrinsics if available
#if defined(BOOST_MSVC)
#  include <intrin.h>
#  if defined(_WIN64)
#    define BOOST_DECIMAL_HAS_MSVC_64BIT_INTRINSICS
#  else
#    define BOOST_DECIMAL_HAS_MSVC_32BIT_INTRINSICS
#  endif
#elif (defined(__x86_64__) || defined(__i386__))
#  include <x86intrin.h>
#  define BOOST_DECIMAL_HAS_X86_INTRINSICS
#elif defined(__ARM_NEON__)
#  include <arm_neon.h>
#  define BOOST_DECIMAL_HAS_ARM_INTRINSICS
#else
#  define BOOST_DECIMAL_HAS_NO_INTRINSICS
#endif

// Use 128-bit integers and suppress warnings for using extensions
#if defined(BOOST_HAS_INT128)
#  define BOOST_DECIMAL_HAS_INT128
#  define BOOST_DECIMAL_INT128_MAX  (boost::int128_type)(((boost::uint128_type) 1 << 127) - 1)
#  define BOOST_DECIMAL_INT128_MIN  (-BOOST_CHARCONV_INT128_MAX - 1)
#  define BOOST_DECIMAL_UINT128_MAX ((2 * (boost::uint128_type) BOOST_CHARCONV_INT128_MAX) + 1)
#endif

#if defined(__has_builtin)
#define BOOST_DECIMAL_HAS_BUILTIN(x) __has_builtin(x)
#else
#define BOOST_DECIMAL_HAS_BUILTIN(x) false
#endif

// Detection for C++23 fixed width floating point types
// All of these types are optional so check for each of them individually
#ifdef __has_include
#  if __has_include(<stdfloat>)
#    include <stdfloat>
#  endif
#endif
#ifdef __STDCPP_FLOAT16_T__
#  define BOOST_DECIMAL_HAS_FLOAT16
#endif
#ifdef __STDCPP_FLOAT32_T__
#  define BOOST_DECIMAL_HAS_FLOAT32
#endif
#ifdef __STDCPP_FLOAT64_T__
#  define BOOST_DECIMAL_HAS_FLOAT64
#endif
#ifdef __STDCPP_FLOAT128_T__
#  define BOOST_DECIMAL_HAS_STDFLOAT128
#endif
#ifdef __STDCPP_BFLOAT16_T__
#  define BOOST_DECIMAL_HAS_BRAINFLOAT16
#endif

#endif // BOOST_DECIMAL_DETAIL_CONFIG_HPP
