// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_TYPE_TRAITS
#define BOOST_DECIMAL_DETAIL_TYPE_TRAITS

// Extends the current type traits to include our types and __int128s

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/emulated128.hpp>
#include <type_traits>

namespace boost { namespace decimal { namespace detail {

template <typename T>
struct make_unsigned { using type = std::make_unsigned_t<T>; };

template <>
struct make_unsigned<uint128> { using type = uint128; };

#ifdef BOOST_DECIMAL_HAS_INT128

template <>
struct make_unsigned<int128_t> { using type = uint128_t; };

template <>
struct make_unsigned<uint128_t> { using type = uint128_t; };

#endif

template <typename T>
using make_unsigned_t = typename make_unsigned<T>::type;

template <typename T>
struct is_integral { static constexpr bool value = std::is_integral<T>::value;};

template <>
struct is_integral<uint128> { static constexpr bool value = true; };

#ifdef BOOST_DECIMAL_HAS_INT128

template <>
struct is_integral<int128_t> { static constexpr bool value = true; };

template <>
struct is_integral<uint128_t> { static constexpr bool value = true; };

#endif

template <typename T>
constexpr bool is_integral<T>::value;

template <typename T>
struct is_floating_point { static constexpr bool value = std::is_floating_point<T>::value; };

#ifdef BOOST_DECIMAL_HAS_FLOAT128
template <>
struct is_floating_point<__float128> { static constexpr bool value = true; };
#endif

template <typename T>
constexpr bool is_floating_point<T>::value;

}}} // Namespaces

#endif
