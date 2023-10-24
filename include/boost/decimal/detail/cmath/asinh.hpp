// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_ASINH_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_ASINH_HPP

#include <array>
#include <type_traits>

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/numbers.hpp>

namespace boost { namespace decimal {

template<typename T>
constexpr auto asinh(T x) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>
{
    const auto fpc = fpclassify(x);

    static_cast<void>(fpc);
    static_cast<void>(x);

    return T { 0, 0 };
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_ASINH_HPP
