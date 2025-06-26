// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_NORMALIZE_HPP
#define BOOST_DECIMAL_DETAIL_NORMALIZE_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <boost/decimal/detail/fenv_rounding.hpp>
#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/remove_trailing_zeros.hpp>

namespace boost {
namespace decimal {
namespace detail {

// Converts the significand to full precision to remove the effects of cohorts
template <typename TargetDecimalType = decimal32, typename T1, typename T2>
constexpr auto normalize(T1& significand, T2& exp, bool sign = false) noexcept -> void
{
    constexpr auto target_precision {detail::precision_v<TargetDecimalType>};
    const auto digits {num_digits(significand)};

    if (digits < target_precision)
    {
        const auto zeros_needed {target_precision - digits};
        significand *= pow10(static_cast<T1>(zeros_needed));
        exp -= zeros_needed;
    }
    else if (digits > target_precision)
    {
        const auto excess_digits {digits - (target_precision + 1)};
        significand /= pow10(static_cast<T1>(excess_digits));
        // Perform final rounding according to the fenv rounding mode
        exp += detail::fenv_round<TargetDecimalType>(significand, sign || significand < 0U) + excess_digits;
    }
}

} //namespace detail
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_NORMALIZE_HPP
