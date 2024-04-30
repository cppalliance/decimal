// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_TANH_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_TANH_HPP

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/numbers.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <array>
#include <type_traits>
#endif

namespace boost {
namespace decimal {

namespace detail {

template <typename T>
constexpr auto tanh_impl(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    const auto fpc = fpclassify(x);

    constexpr T zero { 0, 0 };
    constexpr T one  { 1, 0 };

    auto result = zero;

    if (fpc == FP_ZERO)
    {
        result = x;
    }
    else if (fpc != FP_NORMAL)
    {
        if (fpc == FP_INFINITE)
        {
            if (signbit(x))
            {
                result = -one;
            }
            else
            {
                result = one;
            }
        }
        else if (fpc == FP_NAN)
        {
            result = x;
        }
    }
    else
    {
        if (signbit(x))
        {
            result = -tanh(-x);
        }
        else
        {
            constexpr T quarter { 25, -2 };

            if (x < quarter)
            {
                using coefficient_array_type = std::array<T, static_cast<std::size_t>(UINT8_C(11))>;

                #if (defined(__clang__) && (__clang__ < 6))
                #  pragma clang diagnostic push
                #  pragma clang diagnostic ignored "-Wmissing-braces"
                #endif

                constexpr auto coefficient_table =
                    coefficient_array_type
                    {
                        // Series[Tanh[x], {x, 0, 23}]
                        //         (1),                             // * x
                        -T { UINT64_C(333333333333333333), - 18 - 0 },  // * x^3
                         T { UINT64_C(133333333333333333), - 18 - 0 },  // * x^5
                        -T { UINT64_C(539682539682539683), - 18 - 1 },  // * x^7
                         T { UINT64_C(218694885361552028), - 18 - 1 },  // * x^9
                        -T { UINT64_C(886323552990219657), - 18 - 2 },  // * x^11
                         T { UINT64_C(359212803657248102), - 18 - 2 },  // * x^13
                        -T { UINT64_C(145583438705131827), - 18 - 2 },  // * x^15
                         T { UINT64_C(590027440945585981), - 18 - 3 },  // * x^17
                        -T { UINT64_C(239129114243552481), - 18 - 3 },  // * x^19
                         T { UINT64_C(969153795692945033), - 18 - 4 },  // * x^21
                        -T { UINT64_C(392783238833168341), - 18 - 4 },  // * x^23
                    };

                #if (defined(__clang__) && (__clang__ < 6))
                #  pragma clang diagnostic pop
                #endif

                auto rit =
                    coefficient_table.crbegin()
                  + static_cast<std::size_t>
                    (
                      (sizeof(T) == static_cast<std::size_t>(UINT8_C(4))) ? 6U : 0U
                    );

                result = *rit;

                const auto xsq = x * x;

                while(rit != coefficient_table.crend())
                {
                    result = fma(result, xsq, *rit++);
                }

                result = x * fma(result, xsq, one);
            }
            else
            {
                const auto exp_pos_val = exp(x);
                const auto exp_neg_val = one / exp_pos_val;

                result = (exp_pos_val - exp_neg_val) / (exp_pos_val + exp_neg_val);
            }
        }
    }

    return result;
}

} // namespace detail

BOOST_DECIMAL_EXPORT template <typename T>
constexpr auto tanh(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    #if BOOST_DECIMAL_DEC_EVAL_METHOD == 0

    using evaluation_type = T;

    #elif BOOST_DECIMAL_DEC_EVAL_METHOD == 1

    using evaluation_type = detail::promote_args_t<T, decimal64>;

    #else // BOOST_DECIMAL_DEC_EVAL_METHOD == 2

    using evaluation_type = detail::promote_args_t<T, decimal128>;

    #endif

    return static_cast<T>(detail::tanh_impl(static_cast<evaluation_type>(x)));
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_TANH_HPP
