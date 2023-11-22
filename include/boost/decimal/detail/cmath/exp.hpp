// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_EXP_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_EXP_HPP

#include <array>
#include <type_traits>

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/cmath/impl/pow_impl.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/numbers.hpp>

namespace boost {
namespace decimal {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto exp(T x) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T> // NOLINT(misc-no-recursion)
{
    const auto fpc = fpclassify(x);

    constexpr T zero { 0, 0 };
    constexpr T one  { 1, 0 };

    auto result = zero;

    if (fpc == FP_ZERO)
    {
        result = one;
    }
    else if (fpc != FP_NORMAL)
    {
        if (fpc == FP_INFINITE)
        {
            if (signbit(x))
            {
                result = zero;
            }
            else
            {
                result = x;
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
            result = one / exp(-x);
        }
        else
        {
            // Scale the argument to 0 < x < log(2).

            int nf2 { };

            if (x > numbers::ln2_v<T>)
            {
                nf2 = int(x / numbers::ln2_v<T>);

                x -= (numbers::ln2_v<T> * nf2);
            }

            // PadeApproximant[Exp[x] - 1, {x, 0, {6, 6}}]
            // FullSimplify[%]
            //   (84 x (7920 + 240 x^2 + x^4))
            // / (665280 + x (-332640 + x (75600 + x (-10080 + x (840 + (-42 + x) x)))))

            const auto x2 = x * x;

            // Use the small-argument Pade approximation having coefficients shown above.
            const T top = T { UINT8_C(84), 0 } * x * ( T { UINT16_C(7920), 0 } + ( T { UINT8_C(240), 0 } + x2) * x2);
            const T bot = T { UINT32_C(665280), 0 } + x * (T { INT32_C(-332640), 0 } + x * (T { UINT32_C(75600), 0 } + x * (T { INT16_C(-10080), 0 } + x * (T { UINT16_C(840), 0 } + (T { INT8_C(-42), 0 } + x) * x))));

            result = one + (top / bot);

            if (nf2 > 0)
            {
                if (nf2 < 64)
                {
                    result *= T { static_cast<std::uint64_t>(UINT64_C(1) << static_cast<unsigned>(nf2)), 0 };
                }
                else
                {
                    result *= detail::pow_2_impl<T>(nf2);
                }
            }
        }
    }

    return result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_EXP_HPP
