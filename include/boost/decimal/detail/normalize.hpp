// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_NORMALIZE_HPP
#define BOOST_DECIMAL_DETAIL_NORMALIZE_HPP

#if ((defined(__GNUC__) && (__GNUC__ > 12)) && !defined(__clang__))
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Waggressive-loop-optimizations"
#endif

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
            exp = static_cast<T2>(static_cast<std::make_unsigned_t<T2>>(exp) + static_cast<std::make_unsigned_t<T2>>(UINT8_C(1)));
            --digits;
        }

        exp += detail::fenv_round<TargetDecimalType>(significand, significand < 0);
    }
}

} //namespace detail
} //namespace decimal
} //namespace boost

#if ((defined(__GNUC__) && (__GNUC__ > 12)) && !defined(__clang__))
#  pragma GCC diagnostic pop
#endif

#endif //BOOST_DECIMAL_DETAIL_NORMALIZE_HPP
