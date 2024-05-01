// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_NORMALIZE_HPP
#define BOOST_DECIMAL_DETAIL_NORMALIZE_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <boost/decimal/detail/fenv_rounding.hpp>
#include <boost/decimal/detail/attributes.hpp>

namespace boost {
namespace decimal {
namespace detail {

// Converts the significand to full precision to remove the effects of cohorts
template <typename TargetDecimalType = decimal32, typename T1, typename T2,
          std::enable_if_t<!std::is_same<TargetDecimalType, decimal128>::value, bool> = true>
constexpr auto normalize(T1& significand, T2& exp) noexcept -> void
{
    auto digits {num_digits(significand)};

    if (digits < detail::precision_v<TargetDecimalType>)
    {
        const auto zeros_needed {detail::precision_v<TargetDecimalType> - digits};
        significand *= pow10(static_cast<T1>(zeros_needed));
        exp -= zeros_needed;
    }
    else if (digits > detail::precision_v<TargetDecimalType>)
    {
        while (digits > detail::precision_v<TargetDecimalType> + 1)
        {
            significand /= 10;

            #if ((defined(__GNUC__) && (__GNUC__ > 12)) && !defined(__clang__))
            #  pragma GCC diagnostic push
            #  pragma GCC diagnostic ignored "-Waggressive-loop-optimizations"
            #endif

            ++exp;

            #if ((defined(__GNUC__) && (__GNUC__ > 12)) && !defined(__clang__))
            #  pragma GCC diagnostic pop
            #endif

            --digits;
        }

        exp += detail::fenv_round<TargetDecimalType>(significand, significand < 0);
    }
}

template <typename TargetDecimalType = decimal32, typename T1, typename T2,
          std::enable_if_t<std::is_same<TargetDecimalType, decimal128>::value, bool> = true>
constexpr auto normalize(T1& significand, T2& exp) noexcept
{
    auto digits {num_digits(significand)};

    if (digits < detail::precision_v<decimal128>)
    {
        const auto zeros_needed {detail::precision_v<TargetDecimalType> - digits};
        significand *= pow10(static_cast<T1>(zeros_needed));
        exp -= zeros_needed;
    }

    else if (digits > detail::precision_v<TargetDecimalType>)
    {
        while (digits > detail::precision_v<TargetDecimalType> + 1)
        {
            significand /= 10;

            #if ((defined(__GNUC__) && (__GNUC__ > 12)) && !defined(__clang__))
            #  pragma GCC diagnostic push
                #  pragma GCC diagnostic ignored "-Waggressive-loop-optimizations"
            #endif

            ++exp;

            #if ((defined(__GNUC__) && (__GNUC__ > 12)) && !defined(__clang__))
            #  pragma GCC diagnostic pop
            #endif

            --digits;
        }

        exp += detail::fenv_round<TargetDecimalType>(significand, significand < 0);
    }
}

} //namespace detail
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_NORMALIZE_HPP
