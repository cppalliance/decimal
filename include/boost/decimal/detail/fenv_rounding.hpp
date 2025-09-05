// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_FENV_ROUNDING_HPP
#define BOOST_DECIMAL_DETAIL_FENV_ROUNDING_HPP

#include <boost/decimal/cfenv.hpp>
#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/power_tables.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>

namespace boost {
namespace decimal {
namespace detail {

namespace impl {

template <typename T, typename U>
constexpr auto fenv_round_constexpr_impl(T& val, const U max_sig, const bool, const bool sticky) noexcept
{
    const auto trailing_num {val % 10U};
    val /= 10U;
    int exp_delta {1};

    if (trailing_num > 5U || (trailing_num == 5U && sticky) || (trailing_num == 5U && !sticky && val % 2U == 1U))
    {
        ++val;
    }

    // If the significand was e.g. 99'999'999 rounding up
    // would put it out of range again

    if (static_cast<U>(val) > max_sig)
    {
        val /= 10U;
        ++exp_delta;
    }

    return exp_delta;
}

}

#ifdef BOOST_DECIMAL_NO_CONSTEVAL_DETECTION

// Rounds the value provided and returns an offset of exponent values as required
template <typename TargetType = decimal32_t, typename T, std::enable_if_t<is_integral_v<T>, bool> = true>
constexpr auto fenv_round(T& val, bool is_neg = false, bool sticky = false) noexcept -> int
{
    return impl::fenv_round_constexpr_impl(val, max_significand_v<TargetType>, is_neg, sticky);
}

#else

template <typename TargetType = decimal32_t, typename T, std::enable_if_t<is_integral_v<T>, bool> = true>
constexpr auto fenv_round(T& val, bool is_neg = false, bool sticky = false) noexcept -> int // NOLINT(readability-function-cognitive-complexity)
{
    using significand_type = std::conditional_t<decimal_val_v<TargetType> >= 128, int128::uint128_t, std::int64_t>;

    if (BOOST_DECIMAL_IS_CONSTANT_EVALUATED(coeff))
    {
        return impl::fenv_round_constexpr_impl(val, max_significand_v<TargetType>, is_neg, sticky);
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
                if (is_neg && (trailing_num != 0U || sticky))
                {
                    ++val;
                }
                break;
            case rounding_mode::fe_dec_to_nearest:
                // Round to even or nearest
                if (trailing_num > 5U || (trailing_num == 5U && sticky) || (trailing_num == 5U && !sticky && (static_cast<std::uint64_t>(val) & 1U) == 1U))
                {
                    ++val;
                }
                break;
            case rounding_mode::fe_dec_toward_zero:
                // Do nothing
                break;
            case rounding_mode::fe_dec_upward:
                if (!is_neg && (trailing_num != 0U || sticky))
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

template <typename TargetDecimalType, typename T1, typename T2, typename T3>
BOOST_DECIMAL_FORCE_INLINE constexpr auto coefficient_rounding(T1& coeff, T2& exp, T3& biased_exp, const bool sign) noexcept
{
    auto coeff_digits {detail::num_digits(coeff)};

    // How many digits need to be shifted?
    const auto shift_for_small_exp {(-biased_exp) - 1};
    const auto shift_for_large_coeff {(coeff_digits - detail::precision_v<TargetDecimalType>) - 1};
    const auto shift {std::max(shift_for_small_exp, shift_for_large_coeff)};

    if (BOOST_DECIMAL_UNLIKELY(shift > std::numeric_limits<T1>::digits10))
    {
        // Bounds check for our tables in pow10
        coeff = 0;
        return 1;
    }

    // Do shifting
    const auto shift_pow_ten {detail::pow10(static_cast<T1>(shift))};
    const auto shifted_coeff {coeff / shift_pow_ten};
    const auto trailing_digits {coeff % shift_pow_ten};

    coeff = shifted_coeff;
    const auto sticky {trailing_digits != 0u};
    exp += shift;
    biased_exp += shift;
    coeff_digits -= shift;

    // Do rounding
    const auto removed_digits {detail::fenv_round(coeff, sign, sticky)};
    exp += removed_digits;
    biased_exp += removed_digits;
    coeff_digits -= removed_digits;

    return coeff_digits;
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif //BOOST_DECIMAL_DETAIL_FENV_ROUNDING_HPP
