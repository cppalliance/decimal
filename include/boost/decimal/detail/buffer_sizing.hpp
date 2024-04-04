// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_BUFFER_SIZING_HPP
#define BOOST_DECIMAL_DETAIL_BUFFER_SIZING_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <boost/decimal/detail/concepts.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <type_traits>
#endif

namespace boost {
namespace decimal {
namespace detail {

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4127) // Conditional expression for BOOST_IF_CONSTEXPR will be constant in not C++17
#endif

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE Dec>
constexpr int get_real_precision(int precision = -1) noexcept
{
    // If the user did not specify a precision than we use the maximum representable amount
    // and remove trailing zeros at the end
    return precision == -1 ? std::numeric_limits<Dec>::max_digits10 : precision;
}

template <BOOST_DECIMAL_INTEGRAL Int>
constexpr int total_buffer_length(int real_precision, Int exp, bool signed_value)
{
    // Sign + integer part + '.' + precision of fraction part + e+/e- or p+/p- + exponent digits
    return static_cast<int>(signed_value) + 1 + real_precision + 2 + num_digits(exp);
}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} //namespace detail
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_BUFFER_SIZING_HPP
