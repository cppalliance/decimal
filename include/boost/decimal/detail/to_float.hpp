// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt


#ifndef BOOST_DECIMAL_DETAIL_TO_FLOAT_HPP
#define BOOST_DECIMAL_DETAIL_TO_FLOAT_HPP

#include <cerrno>
#include <limits>
#include <type_traits>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/fast_float/compute_float32.hpp>
#include <boost/decimal/detail/fast_float/compute_float64.hpp>

namespace boost {
namespace decimal {

template <typename Decimal, typename TargetType>
BOOST_DECIMAL_CXX20_CONSTEXPR auto to_float(Decimal val) noexcept -> TargetType
{
    bool success {};

    auto fp_class = fpclassify(val);

    switch (fp_class)
    {
        case FP_NAN:
            if (issignaling(val))
            {
                return std::numeric_limits<TargetType>::signaling_NaN();
            }
            return std::numeric_limits<TargetType>::quiet_NaN();
        case FP_INFINITE:
            return std::numeric_limits<TargetType>::infinity();
        case FP_ZERO:
            return 0;
        default:
            static_cast<void>(success);
    }

    // The casts to result are redundant but in pre C++17 modes MSVC warns about implicit conversions
    TargetType result {};
    BOOST_DECIMAL_IF_CONSTEXPR (std::is_same<TargetType, float>::value)
    {
        result = static_cast<TargetType>(detail::fast_float::compute_float32(val.biased_exponent(),
                                                                             val.full_significand(),
                                                                             val.isneg(), success));
    }
    else BOOST_DECIMAL_IF_CONSTEXPR (std::is_same<TargetType, double>::value)
    {
        result = static_cast<TargetType>(detail::fast_float::compute_float64(val.biased_exponent(),
                                                                             val.full_significand(),
                                                                             val.isneg(), success));
    }
    // TODO(mborland): Implement conversion to long double

    if (BOOST_DECIMAL_UNLIKELY(!success))
    {
        errno = EINVAL;
        return 0;
    }

    return result;
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_TO_FLOAT_HPP
