//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_TYPE_TRAITS_HPP
#define BOOST_DECIMAL_TYPE_TRAITS_HPP

#if __has_include(<boost/type_traits.hpp>)

#define BOOST_DECIMAL_HAS_BOOST_TYPE_TRAITS

// Warnings propagate up from boost.type_traits, so we will ignore them
#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

#if defined(__GNUC__) && !defined(__clang__)
#  pragma GCC diagnostic ignored "-Wduplicated-branches"
#endif

#include <boost/type_traits.hpp>

#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif

#include <boost/decimal/detail/type_traits.hpp>
#include <type_traits>

namespace boost {

template <> struct is_arithmetic<boost::decimal::decimal32> : public true_type {};
template <> struct is_arithmetic<boost::decimal::decimal64> : public true_type {};
template <> struct is_arithmetic<boost::decimal::decimal128> : public true_type {};

template <> struct is_fundamental<boost::decimal::decimal32> : public true_type {};
template <> struct is_fundamental<boost::decimal::decimal64> : public true_type {};
template <> struct is_fundamental<boost::decimal::decimal128> : public true_type {};

template <> struct is_scalar<boost::decimal::decimal32> : public true_type {};
template <> struct is_scalar<boost::decimal::decimal64> : public true_type {};
template <> struct is_scalar<boost::decimal::decimal128> : public true_type {};

template <> struct is_class<boost::decimal::decimal32> : public false_type {};
template <> struct is_class<boost::decimal::decimal64> : public false_type {};
template <> struct is_class<boost::decimal::decimal128> : public false_type {};

template <> struct is_pod<boost::decimal::decimal32> : public true_type {};
template <> struct is_pod<boost::decimal::decimal64> : public true_type {};
template <> struct is_pod<boost::decimal::decimal128> : public true_type {};


} // namespace boost

#endif //__has_include(<boost/type_traits.hpp>)

namespace boost {

namespace decimal {
namespace detail {

#ifdef BOOST_DECIMAL_HAS_BOOST_TYPE_TRAITS
using local_true_type = true_type;
using local_false_type = false_type;
#else
using local_true_type = std::true_type;
using local_false_type = std::false_type;
#endif

}
}

// Section 3.11.2 is_decimal_floating_point
template <typename T> struct is_decimal_floating_point : public decimal::detail::local_false_type{};
template <typename T> struct is_decimal_floating_point<const T> : public is_decimal_floating_point<T>{};
template <typename T> struct is_decimal_floating_point<volatile const T> : public is_decimal_floating_point<T>{};
template <typename T> struct is_decimal_floating_point<volatile T> : public is_decimal_floating_point<T>{};
template <> struct is_decimal_floating_point<boost::decimal::decimal32> : public decimal::detail::local_true_type{};
template <> struct is_decimal_floating_point<boost::decimal::decimal64> : public decimal::detail::local_true_type{};
template <> struct is_decimal_floating_point<boost::decimal::decimal128> : public decimal::detail::local_true_type{};

#if defined(__cpp_inline_variables) && __cpp_inline_variables >= 201606L
template <typename T>
inline constexpr bool is_decimal_floating_point_v = is_decimal_floating_point<T>::value;
#endif

} // namespace boost

#endif //BOOST_DECIMAL_TYPE_TRAITS_HPP
