// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_FENV_HPP
#define BOOST_DECIMAL_FENV_HPP

#include <cfenv>

namespace boost { namespace decimal {

enum class rounding_mode : unsigned
{
    fe_dec_downward = 1 << 0,
    fe_dec_to_nearest = 1 << 1,
    fe_dec_to_nearest_from_zero = 1 << 2,
    fe_dec_toward_zero = 1 << 3,
    fe_dec_upward = 1 << 4
};

rounding_mode _boost_decimal_global_rounding_mode {rounding_mode::fe_dec_to_nearest_from_zero};

rounding_mode fegetround() noexcept
{
    return _boost_decimal_global_rounding_mode;
}

rounding_mode fesetround(rounding_mode round) noexcept
{
    _boost_decimal_global_rounding_mode = round;
    return round;
}

}} // Namespaces

#endif //BOOST_DECIMAL_FENV_HPP
