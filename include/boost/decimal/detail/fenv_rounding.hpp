// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_FENV_ROUNDING_HPP
#define BOOST_DECIMAL_DETAIL_FENV_ROUNDING_HPP

#include <boost/decimal/fenv.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/config.hpp>

namespace boost { namespace decimal { namespace detail {

#ifdef BOOST_DECIMAL_NO_CONSTEVAL_DETECTION

// Rounds the value provided and returns an offset of exponent values as required
template <typename T, std::enable_if_t<is_integral_v<T>, bool> = true>
constexpr auto fenv_round(T& val, bool = false) noexcept -> int
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

#else

template <typename T, std::enable_if_t<is_integral_v<T>, bool> = true>
constexpr auto fenv_round(T& val, bool is_neg = false) noexcept -> int // NOLINT(readability-function-cognitive-complexity)
{
    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(coeff))
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
    else
    {
        const auto round {fegetround()};
        int exp {};

        const auto trailing_num {val % 10};
        val /= 10;
        ++exp;

        // Default rounding mode
        if (round == rounding_mode::fe_dec_to_nearest_from_zero)
        {
            if (trailing_num >= 5)
            {
                ++val;
            }
        }
        else if (round == rounding_mode::fe_dec_downward)
        {
            if (trailing_num >= 5 && is_neg)
            {
                ++val;
            }
        }
        else if (round == rounding_mode::fe_dec_to_nearest)
        {
            // Round to even
            if (trailing_num == 5)
            {
                if (val % 2 == 1)
                {
                    ++val;
                }
            }
            // ... or nearest
            else if (trailing_num > 5)
            {
                ++val;
            }
        }
        else if (round == rounding_mode::fe_dec_toward_zero)
        {
            // Do nothing
        }
        else // rounding_mode::fe_dec_upward
        {
            if (!is_neg && trailing_num != 0)
            {
                ++val;
            }
        }


        // If the significand was e.g. 99'999'999 rounding up
        // would put it out of range again
        constexpr auto max_sig_val = 9'999'999; // TODO(mborland): dynamic based on type

        if (val > max_sig_val)
        {
            val /= 10;
            ++exp;
        }

        return exp;
    }
}

#endif

} // namespace detail
} // namespace decimal
} // namespace boost

#endif //BOOST_DECIMAL_DETAIL_FENV_ROUNDING_HPP
