// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_FREXP10_HPP
#define BOOST_DECIMAL_DETAIL_FREXP10_HPP

#include <limits>
#include <type_traits>
#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/normalize.hpp>

namespace boost {
namespace decimal {

// Returns the normalized significand and exponent to be cohort agnostic
// Returns num in the range [1'000'000, 9'999'999]
//
// If the conversion can not be performed returns UINT32_MAX and exp = 0
template <typename T>
constexpr auto frexp10(T num, int* expptr) noexcept
    -> std::enable_if_t<detail::is_decimal_floating_point_v<T>,
            std::conditional_t<std::is_same<T, decimal32>::value, std::uint32_t, std::uint64_t>>
{
    using ReturnType = std::conditional_t<std::is_same<T, decimal32>::value, std::uint32_t, std::uint64_t>;

    constexpr T zero {0, 0};

    if (num == zero)
    {
        *expptr = 0;
        return 0;
    }
    else if (isinf(num) || isnan(num))
    {
        *expptr = 0;
        return (std::numeric_limits<ReturnType>::max)();
    }

    auto num_exp {num.biased_exponent()};
    auto num_sig {num.full_significand()};
    detail::normalize(num_sig, num_exp);

    *expptr = num_exp;

    return num_sig;
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_FREXP10_HPP
