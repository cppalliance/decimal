// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_ILOGB_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_ILOGB_HPP

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/config.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <cmath>
#include <type_traits>
#endif

namespace boost {
namespace decimal {

BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto ilogb(T d) noexcept
    BOOST_DECIMAL_REQUIRES_RETURN(detail::is_decimal_floating_point_v, T, int)
{
    const auto fpc_d = fpclassify(d);

    if (fpc_d == FP_ZERO)
    {
        return FP_ILOGB0;
    }
    else if (fpc_d == FP_INFINITE)
    {
        return INT_MAX;
    }
    else if (fpc_d == FP_NAN)
    {
        return FP_ILOGBNAN;
    }

    const auto offset = detail::num_digits(d.full_significand()) - 1;

    const auto expval = static_cast<int>(static_cast<int>(d.unbiased_exponent()) + offset);

    return expval;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_ILOGB_HPP
