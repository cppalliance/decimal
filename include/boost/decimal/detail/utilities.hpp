// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_UTILITIES_HPP
#define BOOST_DECIMAL_DETAIL_UTILITIES_HPP

namespace boost { namespace decimal { namespace detail {

template <typename T>
constexpr void swap(T& x, T& y) noexcept
{
    const T temp = x;
    x = y;
    y = temp;
}

}}} // Namespaces

#endif //BOOST_DECIMAL_DETAIL_UTILITIES_HPP
