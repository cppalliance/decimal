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
template <typename TargetDecimalType = decimal32, typename T1, typename T2>
constexpr auto normalize(T1& significand, T2& exp) noexcept -> void
{
    BOOST_DECIMAL_IF_CONSTEXPR (!std::is_same<TargetDecimalType, decimal128>::value)
    {
        auto digits {num_digits(significand)};

        if (digits < detail::precision_v<TargetDecimalType>)
        {
            while (digits < detail::precision_v<TargetDecimalType>)
            {
                significand *= 10;
                --exp;
                ++digits;
            }
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
    else
    {
        auto digits {num_digits(significand)};
        std::cerr << "Digits: " << digits << std::endl;

        if (digits < std::numeric_limits<std::uint64_t>::digits10)
        {
            significand.high = significand.low;
            significand.low = UINT64_C(0);
        }

        if (digits < detail::precision_v<decimal128>)
        {
            while (digits < detail::precision_v<decimal128>)
            {
                significand *= UINT64_C(10);
                --exp;
                ++digits;
            }
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
}

} //namespace detail
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_NORMALIZE_HPP
