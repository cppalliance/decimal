// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_LOGB_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_LOGB_HPP

#include <cmath>
#include <type_traits>

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>

namespace boost {
namespace decimal {

namespace detail {

template <typename T>
constexpr auto logb_impl(T num) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    const auto fpc {fpclassify(num)};

    if (fpc == FP_ZERO)
    {
        return -std::numeric_limits<T>::infinity();
    }
    else if (fpc == FP_INFINITE)
    {
        return std::numeric_limits<T>::infinity();
    }
    else if (fpc == FP_NAN)
    {
        return num;
    }

    const auto offset = detail::num_digits(num.full_significand()) - 1;
    const auto expval = static_cast<int>(static_cast<int>(num.unbiased_exponent()) + offset);

    return static_cast<T>(expval);
}

} // namespace detail

template <typename T>
constexpr auto logb(T num) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    #if BOOST_DECIMAL_DEC_EVAL_METHOD == 0

    using evaluation_type = T;

    #elif BOOST_DECIMAL_DEC_EVAL_METHOD == 1

    using evaluation_type = detail::promote_args_t<T, decimal64>;

    #else // BOOST_DECIMAL_DEC_EVAL_METHOD == 2

    using evaluation_type = detail::promote_args_t<T, decimal128>;

    #endif

    return static_cast<T>(detail::logb_impl(static_cast<evaluation_type>(num)));
}

}
}

#endif //BOOST_DECIMAL_DETAIL_CMATH_LOGB_HPP
