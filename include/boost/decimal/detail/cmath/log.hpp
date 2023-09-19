// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_LOG_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_LOG_HPP

#include <cmath>
#include <type_traits>

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/numbers.hpp>

namespace boost { namespace decimal {

template<typename T>
constexpr auto log(T x) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T> // NOLINT(misc-no-recursion)
{
    // TODO(ckormanyos) Handle arguments infinity and NaN.

    constexpr T zero { 0, 0 };
    constexpr T one  { 1, 0 };

    T result;

    if (isinf(x) || isnan(x))
    {
        result = x;
    }
    else if (x < one)
    {
        // Handle reflection.
        result = ((x > zero) ? -one / log(-x) : -std::numeric_limits<T>::infinity());
    }
    else if(x > one)
    {
        constexpr T two { 2, 0 };

        // The algorithm for logarithm is based on Chapter 5, pages 35-36
        // of Cody and Waite, Software Manual for the Elementary Functions,
        // Prentice Hall, 1980.

        int exp2val { };

        // TODO(ckormanyos) There is probably something more efficient than calling frexp here.
        auto g = (x > two) ? frexp(x, &exp2val) : x;

        bool is_scaled_by_sqrt { };

        if (g > numbers::sqrt2_v<T>)
        {
            g /= numbers::sqrt2_v<T>;

            is_scaled_by_sqrt = true;
        }

        using coef_list_array_type = std::array<T, static_cast<std::size_t>(UINT8_C(12))>;

        constexpr auto coefs =
            coef_list_array_type
            {
                // 1, 12, 80, 448, 2304, 11264, 53248, 245760, 1114112, 4980736, 22020096, 96468992, ...
                // See also Sloane's A058962 at: https://oeis.org/A058962
                one,
                one / UINT8_C(12),
                one / UINT8_C(80),
                one / UINT16_C(448),
                one / UINT16_C(11264),
                one / UINT16_C(53248),
                one / UINT32_C(245760),
                one / UINT32_C(1114112),
                one / UINT32_C(4980736),
                one / UINT32_C(22020096),
                one / UINT32_C(96468992)
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

        if (exp2val > 0)
        {
            result += static_cast<T>(exp2val * numbers::ln2_v<T>);
        }

        if(is_scaled_by_sqrt)
        {
            result += numbers::ln2_v<T> / 2U;
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
