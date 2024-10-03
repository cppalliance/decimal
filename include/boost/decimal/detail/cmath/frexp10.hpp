// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_FREXP10_HPP
#define BOOST_DECIMAL_DETAIL_FREXP10_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/normalize.hpp>
#include <boost/decimal/detail/emulated128.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/config.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <limits>
#include <type_traits>
#endif

namespace boost {
namespace decimal {

// Returns the normalized significand and exponent to be cohort agnostic
// Returns num in the range
//   [1e06, 1e06 - 1] for decimal32
//   [1e15, 1e15 - 1] for decimal64
// If the conversion can not be performed returns UINT32_MAX and exp = 0
BOOST_DECIMAL_EXPORT template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto frexp10(T num, int* expptr) noexcept -> typename T::significand_type
{
    using ReturnType = typename T::significand_type;

    constexpr T zero {0, 0};

    if (num == zero)
    {
        *expptr = 0;
        return 0;
    }
    #ifndef BOOST_DECIMAL_FAST_MATH
    else if (isinf(num) || isnan(num))
    {
        *expptr = 0;
        return (std::numeric_limits<ReturnType>::max)();
    }
    #endif

    auto num_exp {num.biased_exponent()};
    auto num_sig {num.full_significand()};
    detail::normalize<T>(num_sig, num_exp);

    *expptr = static_cast<int>(num_exp);

    return num_sig;
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_FREXP10_HPP
