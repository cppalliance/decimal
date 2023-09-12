// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_FREXP_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_FREXP_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <type_traits>
#include <cmath>

namespace boost { namespace decimal {

template<typename T, std::enable_if_t<detail::is_decimal_floating_point_v<T>, bool>>
constexpr auto frexp(T v, int* expon) noexcept -> T
{
    // This implementation of frexp follows closely that of eval_frexp
    // in Boost.Multiprecision's cpp_dec_float template class.
    constexpr T zero {0};

    auto result = zero;

    const auto v_fp {fpclassify(v)};

    if (v_fp != FP_NORMAL)
    {
        if (expon != nullptr) { *expon = 0; }

        if (v_fp == FP_NAN)
        {
            result = boost::decimal::from_bits(boost::decimal::detail::nan_mask);
        }
        else if (v_fp == FP_INFINITE)
        {
            result = boost::decimal::from_bits(boost::decimal::detail::inf_mask);
        }
    }
    else
    {
        result = v;

        const auto sign_bit = result.bits_.sign;

        result.edit_sign(false);

        using std::ilogb;

        // N[1000/301, 44]
        auto t =
            static_cast<int>
            (
                  static_cast<long double>(ilogb(result))
                * static_cast<long double>(3.3222591362126245847176079734219269102990033L)
            );

        constexpr T local_two {2};

        result *= pow(local_two, -t);

        // TBD: Handle underflow/overflow if (or when) needed.

        constexpr T local_one {1};

        while (result >= local_one)
        {
          result /= local_two;

          ++t;
        }

        constexpr T local_half {5, -1};

        while (result < local_half)
        {
          result *= local_two;

          --t;
        }

        if (expon != nullptr) { *expon = t; }

        result.bits_.sign = sign_bit;
    }

    return result;
}

}} // Namespaces

#endif // BOOST_DECIMAL_DETAIL_CMATH_FREXP_HPP
