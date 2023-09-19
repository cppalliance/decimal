// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_LOG10_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_LOG10_HPP

#include <cmath>
#include <type_traits>

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/type_traits.hpp>

namespace boost { namespace decimal {

template<typename T>
constexpr auto log10(T x) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>
{
    // TODO(ckormanyos) Handle non-nermal varguments.

    // TODO(ckormanyos) Put in a bisic check for pure powers of 10, resulting
    // in an exact result.

    BOOST_DECIMAL_CXX20_CONSTEXPR auto value_lg10 = static_cast<T>(2.30258509299404568402L);

    return log(x) / value_lg10;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_LOG10_HPP
