// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_UTILITIES_HPP
#define BOOST_DECIMAL_DETAIL_UTILITIES_HPP

#include <cstddef>

namespace boost { namespace decimal { namespace detail {

template <typename T>
constexpr void swap(T& x, T& y) noexcept
{
    const T temp {x};
    x = y;
    y = temp;
}

template <typename T>
constexpr std::size_t strlen(const T* str) noexcept
{
    std::size_t i {};
    while (*str != '\0')
    {
        ++str;
        ++i;
    }

    return i;
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif //BOOST_DECIMAL_DETAIL_UTILITIES_HPP
