// Copyright 2006 John Maddock
// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_LAGUERRE_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_LAGUERRE_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/promotion.hpp>
#include <type_traits>
#include <cstdint>

namespace boost {
namespace decimal {

namespace detail {

template <typename T1, typename T2, typename T3>
constexpr auto laguerre_next(unsigned n, T1 x, T2 Ln, T3 Lnm1)
{
    using promoted_type = promote_args_t<T1, T2, T3>;
    return ((2 * n + 1 - static_cast<promoted_type>(x)) * static_cast<promoted_type>(Ln) - n * static_cast<promoted_type>(Lnm1)) / (n + 1);
}

} //namespace detail

// Implement Laguerre polynomials via recurrence:
template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto laguerre(unsigned n, T x) -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T> // NOLINT(misc-no-recursion)
{
    T p0 {UINT64_C(1)};
    T p1 {UINT64_C(1) - x};

    if (n == 0)
    {
        return p0;
    }

    unsigned c = 1;

    while(c < n)
    {
        std::swap(p0, p1);
        p1 = detail::laguerre_next(c, x, p0, p1);
        ++c;
    }

    return p1;
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_LAGUERRE_HPP
