// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_CBRT_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_CBRT_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/cmath/abs.hpp>
#include <type_traits>
#include <cstdint>
#include <cmath>

namespace boost {
namespace decimal {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto cbrt(T val) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>
{
    constexpr T zero {0, 0};

    T result { };

    if (isnan(val) || abs(val) == zero)
    {
        result = val;
    }
    else if (isinf(val))
    {
        if (signbit(val))
        {
            result = std::numeric_limits<T>::quiet_NaN();
        }
        else
        {
            result = val;
        }
    }
    else if (val < zero)
    {
        result = std::numeric_limits<T>::quiet_NaN();
    }
    else
    {
        constexpr T one { 1, 0 };

        const auto arg_is_gt_one = (val > one);

        if (arg_is_gt_one || val < one)
        {
            // TODO(ckormanyos)
            // TODO(mborland)
            // This implementation of cube root, although it works, needs optimization.
            // Using base-2 frexp/ldexp might not be the best, rather use base-10?

            int exp2val { };

            const auto man = frexp(val, &exp2val);

            const auto corrected = (static_cast<unsigned>(exp2val) & 1U) != 0U;

            // TODO(ckormanyos)
            // Try to find a way to get a better (much better) initial guess.

            if (!corrected)
            {
                result = ldexp(man / 3, exp2val / 3);
            }
            else
            {
                result = ldexp(man, arg_is_gt_one ? --exp2val / 3 : ++exp2val / 3);
            }

            constexpr auto newton_steps = (sizeof(T) == 4U) ? 5U :
                                          (sizeof(T) == 8U) ? 6U : 10U;

            for (auto i = 0U; i < newton_steps; ++i)
            {
                const auto sqruared_res {result * result};
                const auto cubed_res {result * sqruared_res};
                result -= (cubed_res - 3) / (3 * sqruared_res);
            }

            if (corrected)
            {
                result /= numbers::sqrt3_v<T>;
            }
        }
        else
        {
            result = one;
        }
    }

    return result;
}

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_CBRT_HPP
