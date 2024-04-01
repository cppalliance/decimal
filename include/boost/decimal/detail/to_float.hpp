// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt


#ifndef BOOST_DECIMAL_DETAIL_TO_FLOAT_HPP
#define BOOST_DECIMAL_DETAIL_TO_FLOAT_HPP

#include <cerrno>
#include <limits>
#include <type_traits>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/shrink_significand.hpp>
#include <boost/decimal/detail/fast_float/compute_float32.hpp>
#include <boost/decimal/detail/fast_float/compute_float64.hpp>
#include <boost/decimal/detail/fast_float/compute_float80_128.hpp>
#include <boost/decimal/detail/concepts.hpp>

namespace boost {
namespace decimal {

// Duplicated branches when on a machine with 64 bit long doubles
#if defined(__GNUC__) && __GNUC__ >= 6
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wduplicated-branches"
#endif

template <typename Decimal, typename TargetType>
BOOST_DECIMAL_CXX20_CONSTEXPR auto to_float(Decimal val) noexcept
    BOOST_DECIMAL_REQUIRES_TWO_RETURN(detail::is_decimal_floating_point_v, Decimal, detail::is_floating_point_v, TargetType, TargetType)
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

    auto sig {val.full_significand()};
    auto exp {val.biased_exponent()};
    const auto new_sig {detail::shrink_significand<std::uint64_t>(sig, exp)};

    BOOST_DECIMAL_IF_CONSTEXPR (std::is_same<TargetType, float>::value)
    {
        result = static_cast<TargetType>(detail::fast_float::compute_float32(exp, new_sig, val.isneg(), success));
    }
    else BOOST_DECIMAL_IF_CONSTEXPR (std::is_same<TargetType, double>::value)
    {
        result = static_cast<TargetType>(detail::fast_float::compute_float64(exp, new_sig, val.isneg(), success));
    }
    else BOOST_DECIMAL_IF_CONSTEXPR (std::is_same<TargetType, long double>::value)
    {
        #if BOOST_DECIMAL_LDBL_BITS == 64
        result = static_cast<TargetType>(detail::fast_float::compute_float64(exp, new_sig, val.isneg(), success));
        #else
        result = static_cast<TargetType>(detail::fast_float::compute_float80(exp, new_sig, val.isneg(), success));
        #endif
    }
    // TODO(mborland): Add conversion for __float128 and 128 bit long doubles

    if (BOOST_DECIMAL_UNLIKELY(!success))
    {
        errno = EINVAL;
        return 0;
    }

    return result;
}

#if defined(__GNUC__) && __GNUC__ >= 6
#  pragma GCC diagnostic pop
#endif

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_TO_FLOAT_HPP
