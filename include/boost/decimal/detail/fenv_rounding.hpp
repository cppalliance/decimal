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

template <typename T1, typename T2>
BOOST_DECIMAL_FORCE_INLINE constexpr auto find_sticky_bit(T1& coeff, T2& exp, const int exp_bias) noexcept
{
    bool sticky {false};

    #ifndef BOOST_DECIMAL_NO_CONSTEVAL_DETECTION

    if (!BOOST_DECIMAL_IS_CONSTANT_EVALUATED(coeff))
    {
        // If we are in a rounding mode that does not depend on the sticky bit opt out early
        // Unlikely that the user sets the global rounding mode anyway
        if (BOOST_DECIMAL_UNLIKELY(_boost_decimal_global_rounding_mode == rounding_mode::fe_dec_to_nearest_from_zero ||
                                   _boost_decimal_global_rounding_mode == rounding_mode::fe_dec_toward_zero))
        {
            return sticky;
        }
    }

    #endif

    const auto biased_exp {exp + exp_bias};

    if (biased_exp < 0)
    {
        const auto shift {static_cast<unsigned>(-biased_exp)};
        const auto shift_p10 {detail::pow10<T1>(shift)};
        const auto shift_p10_min_1 {shift_p10 / 10U};

        const auto q {coeff / shift_p10};
        const auto rem {coeff % shift_p10};

        auto guard_digits {rem / shift_p10_min_1};
        sticky = (rem % shift_p10) != 0U;

        coeff = q;
        exp += static_cast<int>(shift);

        if (guard_digits > 5U || (guard_digits == 5U && (sticky || (coeff % 10U) & 1U)))
        {
            ++coeff;
        }
    }

    return sticky;
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif //BOOST_DECIMAL_DETAIL_FENV_ROUNDING_HPP
