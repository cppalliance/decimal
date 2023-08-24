// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_BIT_CAST_HPP
#define BOOST_DECIMAL_DETAIL_BIT_CAST_HPP

#include <boost/decimal/detail/config.hpp>
#include <type_traits>
#include <cstring>

namespace boost { namespace decimal { namespace detail {

#ifdef BOOST_DECIMAL_HAS_CONSTEXPR_BITCAST

using std::bit_cast;

#else

#if defined(__GNUC__) && !defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Warray-bounds"
#endif

template<class To, class From>
To bit_cast(const From& src) noexcept
{
    static_assert(sizeof(To) >= sizeof(From), "To and From must be the same size");
    To dst;
    std::memcpy(&dst, &src, sizeof(From));
    return dst;
}

#if defined(__GNUC__) && !defined(__clang__)
#  pragma GCC diagnostic pop
#endif

#endif

}}} // Namespaces

#endif // BOOST_DECIMAL_DETAIL_BIT_CAST_HPP
