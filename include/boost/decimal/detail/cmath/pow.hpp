// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_POW_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_POW_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <type_traits>
#include <cmath>

namespace boost { namespace decimal {

template<typename T, std::enable_if_t<detail::is_decimal_floating_point_v<T>, bool>>
constexpr auto pow(T b, int p) noexcept -> T
{
    // Calculate (b ^ p).

    using local_numeric_type = T;

    local_numeric_type result;

    if     (p <  INT64_C(0)) { result = local_numeric_type(1) / pow(b, -p); }
    else if(p == INT64_C(0)) { result = local_numeric_type(static_cast<unsigned>(UINT8_C(1))); }
    else if(p == INT64_C(1)) { result = b; }
    else if(p == INT64_C(2)) { result = b; result *= b; }
    else if(p == INT64_C(3)) { result = b; result *= b; result *= b; }
    else if(p == INT64_C(4)) { result = b; result *= b; result *= result; }
    else
    {
        result = local_numeric_type(static_cast<unsigned>(UINT8_C(1)));

        local_numeric_type y(b);

        auto p_local = static_cast<std::uint64_t>(p);

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

            if(p_local == static_cast<std::uint64_t>(UINT8_C(0)))
            {
                break;
            }

            y *= y;
        }
    }

    return result;
}

}} // Namespaces

#endif // BOOST_DECIMAL_DETAIL_CMATH_POW_HPP
