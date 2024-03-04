//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_TYPE_TRAITS_HPP
#define BOOST_DECIMAL_TYPE_TRAITS_HPP

#if __has_include(<boost/type_traits.hpp>)

#define BOOST_DECIMAL_HAS_BOOST_TYPE_TRAITS
#include <boost/type_traits.hpp>
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

// Section 3.11.2 is_decimal_floating_point

template <typename T> struct is_decimal_floating_point : public false_type{};
template <typename T> struct is_decimal_floating_point<const T> : public is_decimal_floating_point<T>{};
template <typename T> struct is_decimal_floating_point<volatile const T> : public is_decimal_floating_point<T>{};
template <typename T> struct is_decimal_floating_point<volatile T> : public is_decimal_floating_point<T>{};
template <> struct is_decimal_floating_point<boost::decimal::decimal32> : public true_type{};
template <> struct is_decimal_floating_point<boost::decimal::decimal64> : public true_type{};
template <> struct is_decimal_floating_point<boost::decimal::decimal128> : public true_type{};

} // namespace boost

#endif //BOOST_DECIMAL_TYPE_TRAITS_HPP
