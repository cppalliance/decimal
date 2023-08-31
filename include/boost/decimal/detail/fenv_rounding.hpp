// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_FENV_ROUNDING_HPP
#define BOOST_DECIMAL_DETAIL_FENV_ROUNDING_HPP

#include <boost/decimal/fenv.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/config.hpp>

namespace boost { namespace decimal { namespace detail {

// Rounds the value provided and returns an offset of exponent values as required
template <typename T, std::enable_if_t<is_integral_v<T>, bool> = true>
constexpr int fenv_round(T& val) noexcept
{
    const auto trailing_num {val % 10};
    int exp_delta {};
    val /= 10;
    ++exp_delta;

    if (trailing_num >= 5)
    {
        ++val;
    }

    // If the significand was e.g. 99'999'999 rounding up
    // would put it out of range again
    constexpr auto max_sig_val = 9'999'999; // TODO(mborland): dynamic based on type

    if (val > max_sig_val)
    {
        val /= 10;
        ++exp_delta;
    }

    return exp_delta;
}

}}} // Namespaces

#endif //BOOST_DECIMAL_DETAIL_FENV_ROUNDING_HPP
