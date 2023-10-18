// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_TO_DECIMAL_HPP
#define BOOST_DECIMAL_DETAIL_TO_DECIMAL_HPP

#include <cerrno>
#include <limits>
#include <type_traits>
#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/attributes.hpp>

namespace boost {
namespace decimal {

template <typename TargetType, typename Decimal>
constexpr auto to_decimal(Decimal val) noexcept -> TargetType
{
    if (isinf(val))
    {
        return val.isneg() ? -std::numeric_limits<TargetType>::infinity() :
                              std::numeric_limits<TargetType>::infinity();
    }
    else if (issignaling(val))
    {
        return val.isneg() ? -std::numeric_limits<TargetType>::signaling_NaN() :
                              std::numeric_limits<TargetType>::signaling_NaN();
    }
    else if (isnan(val))
    {
        return val.isneg() ? -std::numeric_limits<TargetType>::quiet_NaN() :
                              std::numeric_limits<TargetType>::quiet_NaN();
    }

    return TargetType{val.full_significand(), val.biased_exponent(), val.isneg()};
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_TO_DECIMAL_HPP
