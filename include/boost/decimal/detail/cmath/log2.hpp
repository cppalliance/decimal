// Copyright 2024 Matt Borland
// Copyright 2024 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_LOG2_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_LOG2_HPP

#include <cmath>
#include <type_traits>

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/numbers.hpp>
#include <boost/decimal/detail/cmath/log.hpp>

namespace boost {
namespace decimal {

template <typename T>
constexpr auto log2(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    return log(x) / numbers::ln2_v<T>;
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_LOG2_HPP
