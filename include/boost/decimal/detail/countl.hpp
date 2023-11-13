// Copyright 2022 Peter Dimov
// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <limits>

#ifndef BOOST_DECIMAL_DETAIL_COUNTL_HPP
#define BOOST_DECIMAL_DETAIL_COUNTL_HPP

namespace boost {
namespace decimal {
namespace detail {
namespace impl {

constexpr int countl_impl(unsigned char x) noexcept
{
    return x ? __builtin_clz(x) -
               (std::numeric_limits<unsigned int>::digits - std::numeric_limits<unsigned char>::digits)
             : std::numeric_limits<unsigned char>::digits;
}

constexpr int countl_impl(unsigned short x) noexcept
{
    return x ? __builtin_clz(x) -
               (std::numeric_limits<unsigned int>::digits - std::numeric_limits<unsigned short>::digits)
             : std::numeric_limits<unsigned short>::digits;
}

constexpr int countl_impl(unsigned int x) noexcept
{
    return x ? __builtin_clz(x) : std::numeric_limits<unsigned int>::digits;
}

constexpr int countl_impl(unsigned long x) noexcept
{
    return x ? __builtin_clzl(x) : std::numeric_limits<unsigned long>::digits;
}

constexpr int countl_impl(boost::ulong_long_type x) noexcept
{
    return x ? __builtin_clzll(x) : std::numeric_limits<boost::ulong_long_type>::digits;
}

} //namespace impl

template <typename T>
constexpr int countl_zero(T x) noexcept
{
    static_assert(std::numeric_limits<T>::is_integer && !std::numeric_limits<T>::is_signed,
                  "Can only count with unsigned integers");

    return impl::countl_impl(x);
}

} //namespace detail
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_COUNTL_HPP
