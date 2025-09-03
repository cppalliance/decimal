// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_FENV_ROUNDING_HPP
#define BOOST_DECIMAL_DETAIL_FENV_ROUNDING_HPP

#include <boost/decimal/cfenv.hpp>
#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/config.hpp>

namespace boost {
namespace decimal {
namespace detail {

#ifdef BOOST_DECIMAL_NO_CONSTEVAL_DETECTION

// Rounds the value provided and returns an offset of exponent values as required
template <typename TargetType = decimal32_t, typename T, std::enable_if_t<is_integral_v<T>, bool> = true>
constexpr auto fenv_round(T& val, bool = false) noexcept -> int
{
    using significand_type = std::conditional_t<decimal_val_v<TargetType> >= 128, int128::uint128_t, std::int64_t>;

    const auto trailing_num {val % 10U};
    val /= 10U;
    int exp_delta {1};

    if (trailing_num >= 5U)
    {
        ++val;
    }

    if (static_cast<significand_type>(val) > max_significand_v<TargetType>)
    {
        val /= 10U;
        ++exp_delta;
    }

    return exp_delta;
}

#else

template <typename TargetType = decimal32_t, typename T, std::enable_if_t<is_integral_v<T>, bool> = true>
constexpr auto fenv_round(T& val, bool is_neg = false) noexcept -> int // NOLINT(readability-function-cognitive-complexity)
{
    using significand_type = std::conditional_t<decimal_val_v<TargetType> >= 128, int128::uint128_t, std::int64_t>;

    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(coeff))
    {
        const auto trailing_num {val % 10U};
        val /= 10U;
        int exp_delta {1};

        if (trailing_num > 5U || (trailing_num == 5U && ((val & 1U) == 1U)))
        {
            ++val;
        }

        // If the significand was e.g. 99'999'999 rounding up
        // would put it out of range again

        if (static_cast<significand_type>(val) > static_cast<significand_type>(max_significand_v<TargetType>))
        {
            val /= 10U;
            ++exp_delta;
        }

        return exp_delta;
    }
    else
    {
        const auto round {fegetround()};
        int exp {1};

        const auto trailing_num {static_cast<std::uint32_t>(val % 10U)};
        val /= 10U;

        // Default rounding mode
        switch (round)
        {
            case rounding_mode::fe_dec_to_nearest_from_zero:
                if (trailing_num >= 5U)
                {
                    ++val;
                }
                break;
            case rounding_mode::fe_dec_downward:
                if (is_neg && trailing_num != 0U)
                {
                    ++val;
                }
                break;
            case rounding_mode::fe_dec_to_nearest:
                // Round to even or nearest
                if (trailing_num > 5U || (trailing_num == 5U && val % 2U == 0U))
                {
                    ++val;
                }
                break;
            case rounding_mode::fe_dec_toward_zero:
                // Do nothing
                break;
            case rounding_mode::fe_dec_upward:
                if (!is_neg && trailing_num != 0U)
                {
                    ++val;
                }
                break;
            // LCOV_EXCL_START
            default:
                BOOST_DECIMAL_UNREACHABLE;
            // LCOV_EXCL_STOP
        }


        // If the significand was e.g. 99'999'999 rounding up
        // would put it out of range again

        if (BOOST_DECIMAL_UNLIKELY(static_cast<significand_type>(val) > max_significand_v<TargetType>))
        {
            val /= 10U;
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
