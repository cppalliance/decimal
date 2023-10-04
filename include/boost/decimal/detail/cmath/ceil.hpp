// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_CEIL_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_CEIL_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/power_tables.hpp>
#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/cmath/fpclassify.hpp>
#include <boost/decimal/detail/cmath/frexp10.hpp>
#include <type_traits>
#include <cmath>

namespace boost { namespace decimal {

template <typename T>
constexpr auto ceil BOOST_DECIMAL_PREVENT_MACRO_SUBSTITUTION (T val) noexcept
    -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>
{
    constexpr T zero {0, 0};
    constexpr T one {1, 0};
    const auto fp {fpclassify(val)};

    switch (fp)
    {
        case FP_ZERO:
        case FP_NAN:
        case FP_INFINITE:
            return val;
        default:
            static_cast<void>(val);
    }

    int exp_ptr {};
    auto new_sig {frexp10(val, &exp_ptr)};
    const auto abs_exp {detail::make_positive_unsigned(exp_ptr)};
    const bool is_neg {val < zero};

    const auto sig_dig {static_cast<unsigned>(detail::num_digits(new_sig))};
    auto decimal_digits {sig_dig};

    if (sig_dig > abs_exp)
    {
        decimal_digits = abs_exp;
    }
    else if (exp_ptr < 1 && abs_exp >= sig_dig)
    {
        return is_neg ? zero : one;
    }
    else
    {
        decimal_digits--;
    }

    new_sig /= detail::pow10<std::uint32_t>(decimal_digits);
    if (!is_neg)
    {
        ++new_sig;
    }
    new_sig *= 10;

    return {new_sig, exp_ptr + decimal_digits - 1, is_neg};
}

} // namespace decimal
} // namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_CEIL_HPP
