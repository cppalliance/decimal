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

// https://en.cppreference.com/w/cpp/numeric/bit_cast
template<class To, class From>
std::enable_if_t<
        sizeof(To) == sizeof(From) &&
        std::is_trivially_copyable<From>::value &&
        std::is_trivially_copyable<To>::value,
        To>
// constexpr support needs compiler magic
bit_cast(const From& src) noexcept
{
    static_assert(std::is_trivially_constructible<To>::value,
                  "This implementation additionally requires "
                  "destination type to be trivially constructible");

    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
}
#endif

}}} // Namespaces

#endif // BOOST_DECIMAL_DETAIL_BIT_CAST_HPP
