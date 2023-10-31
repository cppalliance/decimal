// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_TGAMMA_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_TGAMMA_HPP

#include <array>
#include <type_traits>

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/numbers.hpp>

namespace boost { namespace decimal {

template<typename T>
constexpr auto tgamma(T x) noexcept -> std::enable_if_t<detail::is_decimal_floating_point_v<T>, T> // NOLINT(misc-no-recursion)
{
    T result { };

    const auto nx = static_cast<int>(x);

    const auto is_pure_int = (nx == x);

    const auto fpc = fpclassify(x);

    if (fpc != FP_NORMAL)
    {
      if (fpc == FP_ZERO)
      {
          result = (signbit(x) ? -std::numeric_limits<T>::infinity() : std::numeric_limits<T>::infinity());
      }
      else if(fpc == FP_INFINITE)
      {
          result = (signbit(x) ? std::numeric_limits<T>::quiet_NaN() : std::numeric_limits<T>::infinity());
      }
      else
      {
          result = x;
      }
    }
    else if ((nx < 0) && is_pure_int && ((nx & 1) != 0))
    {
        // Pure negative integer argument.
        result = std::numeric_limits<T>::quiet_NaN();
    }
    else
    {
        if (signbit(x))
        {
            // Reflection for negative argument.
            result = -numbers::pi_v<T> / (x * tgamma(-x) * sin(numbers::pi_v<T> * x));
        }
        else
        {
            constexpr T one { 1, 0 };

            if (is_pure_int)
            {
                result = one;

                for(auto index = 2; index < nx; ++index)
                {
                    result *= index;
                }
            }
            else
            {
                using coefficient_array_type = std::array<T, static_cast<std::size_t>(UINT8_C(25))>;

                #if (defined(__clang__) && (__clang__ < 6))
                #  pragma clang diagnostic push
                #  pragma clang diagnostic ignored "-Wmissing-braces"
                #endif

                constexpr auto coefficient_table =
                    coefficient_array_type
                    {
                        // Use a Taylor series expansion for the reciprocal of the gamma function.
                        // N[Series[1/Gamma[z], {z, 0, 26}], 32]
                        //         (1),                                 // * x
                         T { UINT64_C(577215664901532861), -18 -  0 }, // * z^2
                        -T { UINT64_C(655878071520253881), -18 -  0 }, // * z^3
                        -T { UINT64_C(420026350340952355), -18 -  1 }, // * z^4
                        +T { UINT64_C(166538611382291490), -18 -  0 }, // * z^5
                        -T { UINT64_C(421977345555443367), -18 -  1 }, // * z^6
                        -T { UINT64_C(962197152787697356), -18 -  2 }, // * z^7
                        +T { UINT64_C(721894324666309954), -18 -  2 }, // * z^8
                        -T { UINT64_C(116516759185906511), -18 -  2 }, // * z^9
                        -T { UINT64_C(215241674114950973), -18 -  3 }, // * z^10
                        +T { UINT64_C(128050282388116186), -18 -  3 }, // * z^11
                        -T { UINT64_C(201348547807882387), -18 -  4 }, // * z^12
                        -T { UINT64_C(125049348214267066), -18 -  5 }, // * z^13
                        +T { UINT64_C(113302723198169588), -18 -  5 }, // * z^14
                        -T { UINT64_C(205633841697760710), -18 -  6 }, // * z^15
                        +T { UINT64_C(611609510448141582), -18 -  8 }, // * z^16
                        +T { UINT64_C(500200764446922293), -18 -  8 }, // * z^17
                        -T { UINT64_C(118127457048702014), -18 -  8 }, // * z^18
                        +T { UINT64_C(104342671169110051), -18 -  9 }, // * z^19
                        +T { UINT64_C(778226343990507125), -18 - 11 }, // * z^20
                        -T { UINT64_C(369680561864220571), -18 - 11 }, // * z^21
                        +T { UINT64_C(510037028745447598), -18 - 12 }, // * z^22
                        -T { UINT64_C(205832605356650678), -18 - 13 }, // * z^23
                        -T { UINT64_C(534812253942301798), -18 - 14 }, // * z^24
                        +T { UINT64_C(122677862823826079), -18 - 14 }, // * z^25
                        -T { UINT64_C(118125930169745877), -18 - 15 }, // * z^26
                    };

                #if (defined(__clang__) && (__clang__ < 6))
                #  pragma clang diagnostic pop
                #endif

                const auto x_is_gt_one = (x > one);

                auto r = one;

                auto z = x;

                if (x_is_gt_one)
                {
                    // Use a scaling for arguments greater than one.
                    for(auto k = 1; k <= nx; ++k)
                    {
                        r = r * (z - k);
                    }

                    z = z - nx;
                }

                // Perform the Taylor series expansion.
                auto rit =
                    coefficient_table.crbegin()
                  + static_cast<std::size_t>
                    (
                      (sizeof(T) == static_cast<std::size_t>(UINT8_C(4))) ? 12U : 0U
                    );

                result = *rit;

                while(rit != coefficient_table.crend())
                {
                    result = fma(result, z, *rit++);
                }

                result = one / (z * fma(result, z, one));

                if (x_is_gt_one)
                {
                    // Use a scaling for arguments greater than one.
                    result *= r;
                }
            }
        }
    }

    return result;
}

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_ASINH_HPP
