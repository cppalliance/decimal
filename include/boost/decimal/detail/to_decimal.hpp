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
    TargetType return_val {};
    BOOST_DECIMAL_IF_CONSTEXPR (std::is_same<Decimal, TargetType>::value)
    {
        return_val = val;
    }
    else
    {
        return_val = TargetType{val.full_significand(), val.biased_exponent(), val.isneg()};
    }

    return return_val;
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_TO_DECIMAL_HPP
