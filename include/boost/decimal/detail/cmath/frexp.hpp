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

template<typename T, std::enable_if_t<detail::is_decimal_floating_point_v<T>, bool> = true>
constexpr auto frexp(T v, int* expon) noexcept -> T
{
    // This implementation of frexp follows closely that of eval_frexp
    // in Boost.Multiprecision's cpp_dec_float template class.
    constexpr T zero {0};

    auto result_frexp = zero;

    const auto v_fp {fpclassify(v)};

    if (v_fp != FP_NORMAL)
    {
        if (expon != nullptr) { *expon = 0; }

        if (v_fp == FP_NAN)
        {
            result_frexp = boost::decimal::from_bits(boost::decimal::detail::nan_mask);
        }
        else if (v_fp == FP_INFINITE)
        {
            result_frexp = boost::decimal::from_bits(boost::decimal::detail::inf_mask);
        }
    }
    else
    {
        result_frexp = v;

        const auto sign_bit = v.isneg();

        result_frexp.edit_sign(false);

        // N[1000/301, 44]
        auto t =
            static_cast<int>
            (
                  static_cast<long double>(ilogb(result_frexp))
                * static_cast<long double>(3.3222591362126245847176079734219269102990033L)
            );

        constexpr T local_two {2};

        result_frexp *= pow(local_two, -t);

        // TODO(ckormanyos): Handle underflow/overflow if (or when) needed.

        constexpr T local_one {1};

        while (result_frexp >= local_one)
        {
          result_frexp /= local_two;

          ++t;
        }

        constexpr T local_half {5, -1};

        while (result_frexp < local_half)
        {
          result_frexp *= local_two;

          --t;
        }

        if (expon != nullptr) { *expon = t; }

        result_frexp.edit_sign(sign_bit);
    }

    return result_frexp;
}

}} // Namespaces

#endif // BOOST_DECIMAL_DETAIL_CMATH_FREXP_HPP
