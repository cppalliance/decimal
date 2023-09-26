// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_LDEXP_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_LDEXP_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <type_traits>
#include <cmath>

namespace boost { namespace decimal {

template<typename T, std::enable_if_t<detail::is_decimal_floating_point_v<T>, bool> = true>
constexpr auto ldexp(T v, int e2) noexcept -> T
{
    constexpr T zero {0};

    auto ldexp_result = zero;

    const auto v_fp {fpclassify(v)};

    if (v_fp != FP_NORMAL)
    {
        if (v_fp == FP_NAN)
        {
            ldexp_result = boost::decimal::from_bits(boost::decimal::detail::d32_nan_mask);
        }
        else if (v_fp == FP_INFINITE)
        {
            ldexp_result = boost::decimal::from_bits(boost::decimal::detail::d32_inf_mask);
        }
    }
    else
    {
        ldexp_result = v;

        if(e2 > 0)
        {
            if(e2 < 32)
            {
                const T local_p2 { static_cast<std::uint32_t>(1ULL << e2) };

                ldexp_result *= local_p2;
            }
            else
            {
                constexpr T local_two {2};

                ldexp_result *= pow(local_two, e2);
            }
        }
        else if(e2 < 0)
        {
            if(e2 > -32)
            {
                const T local_p2 { static_cast<std::uint32_t>(1ULL << -e2) };

                ldexp_result /= local_p2;
            }
            else
            {
                constexpr T local_half {5, -1};

                ldexp_result *= pow(local_half, -e2);
            }
        }
    }

    return ldexp_result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_LDEXP_HPP
