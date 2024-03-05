// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CONFIG_HPP
#define BOOST_DECIMAL_DETAIL_CONFIG_HPP

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
#  if __cplusplus >= 201806L || (defined(_MSVC_LANG) && (_MSVC_LANG >= 201806L))
#    include <bit>
#    define BOOST_DECIMAL_HAS_STDBIT
#    if defined(__cpp_lib_bit_cast) && (__cpp_lib_bit_cast >= 201806L)
#      define BOOST_DECIMAL_HAS_CONSTEXPR_BITCAST
#    endif
#  endif
#endif

// Constexpr bit cast is broken on clang-10 and 32-bit platforms
#if defined(BOOST_DECIMAL_HAS_CONSTEXPR_BITCAST) && ((defined(__clang__) && __clang_major__ == 10) || defined(__i386__))
#  undef BOOST_DECIMAL_HAS_CONSTEXPR_BITCAST
#endif

#ifdef BOOST_DECIMAL_HAS_CONSTEXPR_BITCAST
#  define BOOST_DECIMAL_CXX20_CONSTEXPR constexpr
#endif

#ifndef BOOST_DECIMAL_CXX20_CONSTEXPR
#  define BOOST_DECIMAL_CXX20_CONSTEXPR inline
#endif

// Include intrinsics if available
// This section allows us to disable any of the following independently.
//   Use #define BOOST_DECIMAL_DISABLE_CASSERT to disable uses of assert.
//   Use #define BOOST_DECIMAL_DISABLE_IOSTREAM to disable uses of I/O streaming.
//   Use #define BOOST_DECIMAL_DISABLE_CLIB to disable uses of both assert as well as I/O streaming (and all oother heavyweight C-LIB artifacts).

#if (!defined(BOOST_DECIMAL_DISABLE_CASSERT) && !defined(BOOST_DECIMAL_DISABLE_CLIB))
#include <cassert>
#endif

#ifndef BOOST_DECIMAL_DISABLE_CASSERT
#  define BOOST_DECIMAL_ASSERT(x) assert(x)
#  define BOOST_DECIMAL_ASSERT_MSG(expr, msg) assert((expr)&&(msg))
#else
#  define BOOST_DECIMAL_ASSERT(x)
#  define BOOST_DECIMAL_ASSERT_MSG(expr, msg)
#endif

#ifdef BOOST_DECIMAL_DISABLE_CLIB
#  ifndef BOOST_DECIMAL_DISABLE_IOSTREAM
#    define BOOST_DECIMAL_DISABLE_IOSTREAM
#  endif
#  ifndef BOOST_DECIMAL_DISABLE_CASSERT
#    undef BOOST_DECIMAL_ASSERT
#    define BOOST_DECIMAL_ASSERT(x)
#  endif
#endif

// Include intrinsics if available
#if defined(_MSC_VER)
#  include <intrin.h>
#  if defined(_WIN64)
#    define BOOST_DECIMAL_HAS_MSVC_64BIT_INTRINSICS
#  else
#    define BOOST_DECIMAL_HAS_MSVC_32BIT_INTRINSICS
#  endif
#elif defined(__x86_64__)
#  include <x86intrin.h>
#  define BOOST_DECIMAL_HAS_X86_INTRINSICS
#elif defined(__ARM_NEON__)
#  include <arm_neon.h>
#  define BOOST_DECIMAL_HAS_ARM_INTRINSICS
#else
#  define BOOST_DECIMAL_HAS_NO_INTRINSICS
#endif

// Use 128-bit integers and suppress warnings for using extensions
#if defined(BOOST_HAS_INT128) || (defined(__SIZEOF_INT128__) && !defined(_MSC_VER))

namespace boost { namespace decimal { namespace detail {

#  ifdef __GNUC__
__extension__ typedef __int128 int128_t;
__extension__ typedef unsigned __int128 uint128_t;
#  else
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;
#  endif

} // namespace detail
} // namespace decimal
} // namespace boost

#  define BOOST_DECIMAL_HAS_INT128
#  define BOOST_DECIMAL_INT128_MAX  (boost::decimal::detail::int128_t)(((boost::decimal::detail::uint128_t) 1 << 127) - 1)
#  define BOOST_DECIMAL_INT128_MIN  (-BOOST_DECIMAL_INT128_MAX - 1)
#  define BOOST_DECIMAL_UINT128_MAX ((2 * (boost::decimal::detail::uint128_t) BOOST_DECIMAL_INT128_MAX) + 1)
#endif

// 128-bit floats
#if defined(BOOST_HAS_FLOAT128) || defined(__SIZEOF_FLOAT128__)
#  define BOOST_DECIMAL_HAS_FLOAT128
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
#    if __cplusplus > 202002L || _MSVC_LANG > 202002L
#    include <stdfloat>
#    endif
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

#define BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION

#if defined(___GNUC__) || defined(__clang__)
#    define BOOST_DECIMAL_ATTRIBUTE_UNUSED __attribute__((__unused__))
#else
#    define BOOST_DECIMAL_ATTRIBUTE_UNUSED
#endif

#if !defined(__cpp_if_constexpr) || (__cpp_if_constexpr < 201606L)
#  define BOOST_DECIMAL_NO_CXX17_IF_CONSTEXPR
#endif

#ifndef BOOST_DECIMAL_NO_CXX17_IF_CONSTEXPR
#  define BOOST_DECIMAL_IF_CONSTEXPR if constexpr
#else
#  define BOOST_DECIMAL_IF_CONSTEXPR if
#endif

#if BOOST_DECIMAL_HAS_BUILTIN(__builtin_expect)
#  define BOOST_DECIMAL_LIKELY(x) __builtin_expect(x, 1)
#  define BOOST_DECIMAL_UNLIKELY(x) __builtin_expect(x, 0)
#else
#  define BOOST_DECIMAL_LIKELY(x) x
#  define BOOST_DECIMAL_UNLIKELY(x) x
#endif

#if defined(__cpp_lib_three_way_comparison) && __cpp_lib_three_way_comparison >= 201907L
#  include <compare>
#  define BOOST_DECIMAL_HAS_SPACESHIP_OPERATOR
#endif

// Is constant evaluated detection
#ifdef __cpp_lib_is_constant_evaluated
#  define BOOST_DECIMAL_HAS_IS_CONSTANT_EVALUATED
#endif

#ifdef __has_builtin
#  if __has_builtin(__builtin_is_constant_evaluated)
#    define BOOST_DECIMAL_HAS_BUILTIN_IS_CONSTANT_EVALUATED
#  endif
#endif

//
// MSVC also supports __builtin_is_constant_evaluated if it's recent enough:
//
#if defined(_MSC_FULL_VER) && (_MSC_FULL_VER >= 192528326)
#  define BOOST_DECIMAL_HAS_BUILTIN_IS_CONSTANT_EVALUATED
#endif

//
// As does GCC-9:
//
#if (__GNUC__ >= 9) && !defined(BOOST_DECIMAL_HAS_BUILTIN_IS_CONSTANT_EVALUATED)
#  define BOOST_DECIMAL_HAS_BUILTIN_IS_CONSTANT_EVALUATED
#endif

#if defined(BOOST_DECIMAL_HAS_IS_CONSTANT_EVALUATED)
#  define BOOST_DECIMAL_IS_CONSTANT_EVALUATED(x) std::is_constant_evaluated()
#elif defined(BOOST_DECIMAL_HAS_BUILTIN_IS_CONSTANT_EVALUATED)
#  define BOOST_DECIMAL_IS_CONSTANT_EVALUATED(x) __builtin_is_constant_evaluated()
#else
#  define BOOST_DECIMAL_IS_CONSTANT_EVALUATED(x) false
#  define BOOST_DECIMAL_NO_CONSTEVAL_DETECTION
#endif

// https://github.com/llvm/llvm-project/issues/55638
#if defined(__clang__) && __cplusplus > 202002L
#  undef BOOST_DECIMAL_IS_CONSTANT_EVALUATED
#  define BOOST_DECIMAL_IS_CONSTANT_EVALUATED(x) false
#  define BOOST_DECIMAL_NO_CONSTEVAL_DETECTION
#endif

#if defined(__clang__)
#  if defined __has_feature
#    if __has_feature(thread_sanitizer) || __has_feature(address_sanitizer) || __has_feature(thread_sanitizer)
#      define BOOST_DECIMAL_REDUCE_TEST_DEPTH
#    endif
#  endif
#elif defined(__GNUC__)
#  if defined(__SANITIZE_THREAD__) || defined(__SANITIZE_ADDRESS__) || defined(__SANITIZE_THREAD__)
#    define BOOST_DECIMAL_REDUCE_TEST_DEPTH
#  endif
#elif defined(_MSC_VER)
#  if defined(_DEBUG) || defined(__SANITIZE_ADDRESS__)
#    define BOOST_DECIMAL_REDUCE_TEST_DEPTH
#  endif
#endif

#if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH) && ((defined(UBSAN) && (UBSAN == 1)))
#  define BOOST_DECIMAL_REDUCE_TEST_DEPTH
#endif

#ifdef __clang__
#  define BOOST_DECIMAL_CLANG_STATIC static
#else
#  define BOOST_DECIMAL_CLANG_STATIC
#endif

#endif // BOOST_DECIMAL_DETAIL_CONFIG_HPP
