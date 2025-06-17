// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_TYPE_TRAITS
#define BOOST_DECIMAL_DETAIL_TYPE_TRAITS

// Extends the current type traits to include our types and __int128s
#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/emulated128.hpp>
#include <boost/int128/int128.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <type_traits>
#endif

namespace boost {
namespace decimal {
namespace detail {

template <typename T>
struct is_signed { static constexpr bool value = std::is_signed<T>::value; };

template <>
struct is_signed<uint128> { static constexpr bool value = false; };

template <>
struct is_signed<int128> { static constexpr  bool value = true;};

template <>
struct is_signed<boost::int128::int128_t> { static constexpr bool value = true; };

template <>
struct is_signed<boost::int128::uint128_t> { static constexpr bool value = false; };

#ifdef BOOST_DECIMAL_HAS_INT128

template <>
struct is_signed<int128_t> { static constexpr bool value = true; };

template <>
struct is_signed<uint128_t> { static constexpr bool value = false;};

#endif

template <typename T>
constexpr bool is_signed<T>::value;

template <typename T>
constexpr bool is_signed_v = is_signed<T>::value;

template <typename T>
constexpr bool is_unsigned_v = !is_signed_v<T>;

template <typename T>
struct make_unsigned { using type = std::make_unsigned_t<T>; };

template <>
struct make_unsigned<bool> { using type = std::uint8_t; };

template <>
struct make_unsigned<uint128> { using type = uint128; };

template <>
struct make_unsigned<int128> { using type = uint128; };

template <>
struct make_unsigned<boost::int128::uint128_t> { using type = boost::int128::uint128_t; };

template <>
struct make_unsigned<boost::int128::int128_t> { using type = boost::int128::uint128_t; };

#ifdef BOOST_DECIMAL_HAS_INT128

template <>
struct make_unsigned<int128_t> { using type = uint128_t; };

template <>
struct make_unsigned<uint128_t> { using type = uint128_t; };

#endif

template <typename T>
using make_unsigned_t = typename make_unsigned<T>::type;

template <typename T>
struct make_signed { using type = std::make_signed_t<T>; };

template <>
struct make_signed<uint128> { using type = int128; };

template <>
struct make_signed<int128> { using type = int128; };

template <>
struct make_signed<boost::int128::int128_t> { using type = boost::int128::int128_t; };

template <>
struct make_signed<boost::int128::uint128_t> { using type = boost::int128::int128_t; };

#ifdef BOOST_DECIMAL_HAS_INT128

template <>
struct make_signed<int128_t> { using type = int128_t; };

template <>
struct make_signed<uint128_t> { using type = int128_t; };

#endif

template <typename T>
using make_signed_t = typename make_signed<T>::type;

template <typename T>
struct is_integral { static constexpr bool value = std::is_integral<T>::value;};

template <>
struct is_integral<uint128> { static constexpr bool value = true; };

template <>
struct is_integral<int128> { static constexpr bool value = true; };

template <>
struct is_integral<boost::int128::int128_t> { static constexpr bool value = true; };

template <>
struct is_integral<boost::int128::uint128_t> { static constexpr bool value = true; };

#ifdef BOOST_DECIMAL_HAS_INT128

template <>
struct is_integral<int128_t> { static constexpr bool value = true; };

template <>
struct is_integral<uint128_t> { static constexpr bool value = true; };

#endif

template <typename T>
constexpr bool is_integral<T>::value;

template <typename T>
constexpr bool is_integral_v = is_integral<T>::value;

template <typename T>
struct is_floating_point { static constexpr bool value = std::is_floating_point<T>::value; };

#ifdef BOOST_DECIMAL_HAS_FLOAT128
template <>
struct is_floating_point<__float128> { static constexpr bool value = true; };
#endif

template <typename T>
constexpr bool is_floating_point<T>::value;

template <typename T>
constexpr bool is_floating_point_v = is_floating_point<T>::value;

template <typename T>
struct is_decimal_floating_point { static constexpr bool value = false; };

template <>
struct is_decimal_floating_point<decimal32> { static constexpr bool value = true; };

template <>
struct is_decimal_floating_point<decimal64> { static constexpr bool value = true; };

template <>
struct is_decimal_floating_point<decimal128> { static constexpr bool value = true; };

template <>
struct is_decimal_floating_point<decimal32_fast> { static constexpr bool value = true; };

template <>
struct is_decimal_floating_point<decimal64_fast> { static constexpr bool value = true; };

template <>
struct is_decimal_floating_point<decimal128_fast> { static constexpr bool value = true; };

template <typename T>
constexpr bool is_decimal_floating_point<T>::value;

template <typename T>
constexpr bool is_decimal_floating_point_v = is_decimal_floating_point<T>::value;

template <typename T>
struct is_ieee_type { static constexpr bool value = false; };

template <>
struct is_ieee_type<decimal32> { static constexpr bool value = true; };

template <>
struct is_ieee_type<decimal64> { static constexpr bool value = true; };

template <>
struct is_ieee_type<decimal128> { static constexpr bool value = true; };

template <>
struct is_ieee_type<decimal32_fast> { static constexpr bool value = false; };

template <>
struct is_ieee_type<decimal64_fast> { static constexpr bool value = false; };

template <>
struct is_ieee_type<decimal128_fast> { static constexpr bool value = false; };

template <typename T>
constexpr bool is_ieee_type_v = is_ieee_type<T>::value;

template <typename T>
constexpr bool is_fast_type_v = !is_ieee_type_v<T>;

template <typename...>
struct conjunction : std::true_type {};

template <typename B1>
struct conjunction<B1> : B1 {};

template <typename B1, typename... Bn>
struct conjunction<B1, Bn...>
        : std::conditional_t<static_cast<bool>(B1::value), conjunction<Bn...>, B1> {};

template <typename... B>
BOOST_DECIMAL_CONSTEXPR_VARIABLE bool conjunction_v = conjunction<B...>::value;

} // namespace detail
} // namespace decimal
} // namespace boost

#endif
