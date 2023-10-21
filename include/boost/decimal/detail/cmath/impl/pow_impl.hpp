// Copyright 2023 Matt Borland
// Copyright 2023 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_POW_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_POW_IMPL_HPP

#include <cstdint>
#include <type_traits>

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>

namespace boost { namespace decimal { namespace detail {

template<typename T, typename UnsignedIntegralType>
constexpr auto pow_impl(T b, UnsignedIntegralType p) noexcept -> std::enable_if_t<(detail::is_decimal_floating_point_v<T> && std::is_integral<UnsignedIntegralType>::value && std::is_unsigned<UnsignedIntegralType>::value), T> // NOLINT(misc-no-recursion)
{
    // Calculate (b ^ p).

    using local_unsigned_integral_type = UnsignedIntegralType;

    constexpr T one { 1, 0 };

    T result { };

    if     (p == static_cast<local_unsigned_integral_type>(UINT8_C(0))) { result = one; }
    else if(p == static_cast<local_unsigned_integral_type>(UINT8_C(1))) { result = b; }
    else if(p == static_cast<local_unsigned_integral_type>(UINT8_C(2))) { result = b; result *= b; }
    else if(p == static_cast<local_unsigned_integral_type>(UINT8_C(3))) { result = b; result *= b; result *= b; }
    else if(p == static_cast<local_unsigned_integral_type>(UINT8_C(4))) { result = b; result *= b; result *= result; }
    else
    {
        result = one;

        T y(b);

        auto p_local = static_cast<local_unsigned_integral_type>(p);

        // Use the so-called ladder method for the power calculation.
        for(;;)
        {
            const auto do_power_multiply =
              (static_cast<std::uint_fast8_t>(p_local & static_cast<unsigned>(UINT8_C(1))) != static_cast<std::uint_fast8_t>(UINT8_C(0)));

            if(do_power_multiply)
            {
              result *= y;
            }

            p_local >>= static_cast<unsigned>(UINT8_C(1));

            if(p_local == static_cast<local_unsigned_integral_type>(UINT8_C(0)))
            {
                break;
            }

            y *= y;
        }
    }

    return result;
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif
