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
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/numbers.hpp>

namespace boost {
namespace decimal {

template <typename T>
constexpr auto log10(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    // TODO(ckormanyos) Actually this is eintirely preliminary. The implementation
    // of log10 will/should-be entirely re-worked with specific base-10-relevant details.

    // TODO(ckormanyos) Handle non-normal varguments.

    // TODO(ckormanyos) Put in a basic check for pure powers of 10, resulting
    // in an exact result.

    return log(x) / numbers::ln10_v<T>;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_LOG10_HPP
