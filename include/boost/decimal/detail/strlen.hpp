// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_STRLEN_HPP
#define BOOST_DECIMAL_DETAIL_STRLEN_HPP

#include <boost/decimal/detail/config.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE

#include <cstddef>

#endif

namespace boost {
namespace decimal {
namespace detail {

template <typename T>
constexpr std::ptrdiff_t generic_strlen(T* ptr)
{
    std::ptrdiff_t dist {};
    while (*ptr != static_cast<T>(0))
    {
        ++dist;
        ++ptr;
    }

    return dist;
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_STRLEN_HPP
