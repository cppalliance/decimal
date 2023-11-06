// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_TO_INTEGRAL_HPP
#define BOOST_DECIMAL_DETAIL_TO_INTEGRAL_HPP

#include <cerrno>
#include <limits>
#include <type_traits>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/power_tables.hpp>
#include <boost/decimal/detail/apply_sign.hpp>

namespace boost {
namespace decimal {

// MSVC 14.1 warns of unary minus being applied to unsigned type from numeric_limits::min
// 14.2 and on get it right
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4146)
#endif

// Attempts conversion to integral type:
// If this is nan sets errno to EINVAL and returns 0
// If this is not representable sets errno to ERANGE and returns 0
template <typename Decimal, typename TargetType>
constexpr auto to_integral(Decimal val) noexcept -> TargetType
{
    using Conversion_Type = std::conditional_t<std::numeric_limits<TargetType>::is_signed, std::int64_t, std::uint64_t>;

    constexpr Decimal max_target_type { (std::numeric_limits<TargetType>::max)() };
    constexpr Decimal min_target_type { (std::numeric_limits<TargetType>::min)() };

    if (isnan(val))
    {
        errno = EINVAL;
        return static_cast<TargetType>(0);
    }
    if (isinf(val) || val > max_target_type || val < min_target_type)
    {
        errno = ERANGE;
        return static_cast<TargetType>(0);
    }

    auto result {static_cast<Conversion_Type>(val.full_significand())};
    auto expval {val.biased_exponent()};
    if (expval > 0)
    {
        result *= detail::pow10<Conversion_Type>(expval);
    }
    else if (expval < 0)
    {
        result /= detail::pow10<Conversion_Type>(detail::make_positive_unsigned(expval));
    }

    BOOST_DECIMAL_IF_CONSTEXPR (std::is_signed<TargetType>::value)
    {
        result = static_cast<bool>(val.isneg()) ? detail::apply_sign(result) : result;
    }

    return static_cast<TargetType>(result);
}

template <typename Decimal, typename TargetType>
constexpr auto to_integral_128(Decimal val) noexcept -> TargetType
{
    constexpr Decimal max_target_type { (std::numeric_limits<TargetType>::max)() };
    constexpr Decimal min_target_type { (std::numeric_limits<TargetType>::min)() };

    if (isnan(val))
    {
        errno = EINVAL;
        return static_cast<TargetType>(0);
    }
    if (isinf(val) || val > max_target_type || val < min_target_type)
    {
        errno = ERANGE;
        return static_cast<TargetType>(0);
    }

    auto sig {val.full_significand()};
    auto expval {val.biased_exponent()};
    if (expval > 0)
    {
        sig *= detail::pow10<detail::uint128>(expval);
    }
    else if (expval < 0)
    {
        sig /= detail::pow10<detail::uint128>(detail::make_positive_unsigned(expval));
    }

    auto result {static_cast<TargetType>(sig)};

    BOOST_DECIMAL_IF_CONSTEXPR (std::is_signed<TargetType>::value)
    {
        result = static_cast<bool>(val.isneg()) ? detail::apply_sign(result) : result;
    }

    return static_cast<TargetType>(result);
}


#ifdef _MSC_VER
#  pragma warning(pop)
#endif

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_TO_INTEGRAL_HPP
