// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_LOG_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_LOG_HPP

#include <cmath>
#include <type_traits>

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>

namespace boost { namespace decimal {

template<typename T>
constexpr auto log(T x) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T>
{
    constexpr auto zero = T { 0 };
    constexpr auto one  = T { 1 };

    auto result = T { };

    if (x < one)
    {
        // Handle reflection.
        result = ((x > zero) ? -one / log(-x) : std::numeric_limits<T>::infinity());
    }
    else if(x > one)
    {
        constexpr auto two  = T { 2 };

        // This algorithm for logarithm is based on Chapter 5 of Cody and Waite,
        // Software Manual for the Elementary Functions, Prentice Hall, 1980.

        auto exp2val = int { };

        // TODO(ckormanyos) There is probably something more efficient than calling frexp here.
        auto g = (x > two) ? frexp(x, &exp2val) : x;

        BOOST_DECIMAL_CXX20_CONSTEXPR auto value_sqrt2 = static_cast<T>(1.41421356237309504880L);

        auto is_sqrt2_scaled = bool { };

        if (g > value_sqrt2)
        {
            g /= value_sqrt2;

            is_sqrt2_scaled = true;
        }

        using coef_list_array_type = std::array<T, static_cast<std::size_t>(UINT8_C(12))>;

        constexpr auto coefs =
            coef_list_array_type
            {
                // 1, 12, 80, 448, 2304, 11264, 53248, 245760, 1114112, 4980736, 22020096, 96468992, ...
                // See also Sloane's A058962 at: https://oeis.org/A058962
                one, one / 12U, one / 80U, one / 448U, one / 11264U, one / 53248U, one / UINT32_C(245760),  one / UINT32_C(1114112),  one / UINT32_C(4980736),  one / UINT32_C(22020096),  one / UINT32_C(96468992)
            };

        const auto s = (g - one) / (g + one);
        const auto z = s + s;

              auto zn   = z;
        const auto z2   = z * z;
              auto term = z;

        // TODO(ckormanyos) Optimize?
        // Using a loop expansion is scalable from decimal32 up to decimal64.
        // It might, however, be somewhat slower than using fma statically
        // expanded on the static, constexpr coefficient list.

        for(auto   index = static_cast<std::size_t>(UINT8_C(1));
                 ((index < std::tuple_size<coef_list_array_type>::value) && (abs(term) >= std::numeric_limits<T>::epsilon()));
                 ++index)

        {
          result += term;

          zn *= z2;

          term = zn * coefs[index];
        }

        {
            BOOST_DECIMAL_CXX20_CONSTEXPR auto value_ln2 = static_cast<T>(0.693147180559945309417L);

            if (exp2val > 0)
            {
                result += static_cast<T>(exp2val * value_ln2);
            }

            if(is_sqrt2_scaled)
            {
                result += value_ln2 / 2U;
            }
        }
    }
    else
    {
        result = zero;
    }

    return result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_LOG_HPP
