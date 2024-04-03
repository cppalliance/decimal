// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_FENV_HPP
#define BOOST_DECIMAL_FENV_HPP

#include <cfenv>

namespace boost { namespace decimal {

BOOST_DECIMAL_EXPORT enum class rounding_mode : unsigned
{
    fe_dec_downward = 1 << 0,
    fe_dec_to_nearest = 1 << 1,
    fe_dec_to_nearest_from_zero = 1 << 2,
    fe_dec_toward_zero = 1 << 3,
    fe_dec_upward = 1 << 4,
    fe_dec_default = fe_dec_to_nearest_from_zero
};

BOOST_DECIMAL_EXPORT static rounding_mode _boost_decimal_global_rounding_mode {rounding_mode::fe_dec_default};

BOOST_DECIMAL_EXPORT inline auto fegetround() noexcept -> rounding_mode
{
    return _boost_decimal_global_rounding_mode;
}

// If we can't support constexpr and non-constexpr code paths we won't honor the updated rounding-mode,
// since it will not be used anyway.
// Return the default rounding mode
BOOST_DECIMAL_EXPORT inline auto fesetround(rounding_mode round) noexcept -> rounding_mode
{
    _boost_decimal_global_rounding_mode = round;
    return round;
}

} // namespace decimal
} // namespace boost

#endif //BOOST_DECIMAL_FENV_HPP
