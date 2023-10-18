// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_POW_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_POW_HPP

#include <algorithm>
#include <cmath>
#include <type_traits>

#include <boost/decimal/fwd.hpp> // NOLINT(llvm-include-order)
#include <boost/decimal/detail/type_traits.hpp>

namespace boost { namespace decimal {

template<typename T, std::enable_if_t<detail::is_decimal_floating_point_v<T>, bool> = true>
constexpr auto pow(T x, T a) noexcept -> T
{
    const auto fpc_x = fpclassify(x);
    const auto fpc_a = fpclassify(a);

    const auto na = static_cast<int>(a);

    const auto a_is_integer      = (na == a);
    const auto a_is_integer_even = (a_is_integer && ((na & 1) == 0));
    const auto a_is_integer_odd  = (a_is_integer && ((na & 1) != 0));

    constexpr T zero { 0, 0 };

    auto result = zero;

    // For non-normal arguments and special cases, see:
    //   https://en.cppreference.com/w/cpp/numeric/math/pow

    if (fpc_x == FP_ZERO)
    {
        if (fpc_a == FP_NORMAL)
        {
            const auto x_is_neg = signbit(x);

            if      ((!x_is_neg) && a_is_integer_odd && (na < 0))         { result =  std::numeric_limits<T>::infinity(); }
            else if (  x_is_neg  && a_is_integer_odd && (na < 0))         { result = -std::numeric_limits<T>::infinity(); }
            else if (  x_is_neg)                                          { result =  std::numeric_limits<T>::infinity(); }
            else if ((!x_is_neg) && a_is_integer_odd && (na > 0))         { result =  zero; }
            else if (  x_is_neg  && a_is_integer_odd && (na > 0))         { result = -zero; }
            else if ((!x_is_neg) && (a_is_integer_even || (!signbit(a)))) { result =  zero; }
        }
        else if (fpc_a == FP_INFINITE)
        {
            result = (signbit(a) ? std::numeric_limits<T>::infinity() : zero);
        }
    }
    else if (fpc_x == FP_INFINITE)
    {
        // TBD: Infinite |base| still needed.
        result = zero;
    }
    else if (fpc_x != FP_NORMAL)
    {
        result = x;
    }
    else
    {
        constexpr T one  { 1, 0 };

        using std::abs;

        const auto x_abs_is_one = (a_is_integer_odd && abs(na) == 1);

        if (x_abs_is_one)
        {
            if (((na < 0) && (fpc_a == FP_INFINITE)) || (na > 0)) { result = one; }
        }
        else if (fpc_a == FP_ZERO)
        {
            result = one;
        }
        else if (fpc_a == FP_INFINITE)
        {
            const auto abs_base = abs(x);

            if      (abs_base < one) { result = (signbit(a) ? std::numeric_limits<T>::infinity() : zero); }
            else if (abs_base > one) { result = (signbit(a) ? zero : std::numeric_limits<T>::infinity()); }
        }
        else if (fpc_a == FP_ZERO)
        {
            result = one;
        }
        else
        {
            const auto a_log_x = a * log(x);

            result = exp(a_log_x);
        }
    }

    return result;
}

template<typename T, std::enable_if_t<detail::is_decimal_floating_point_v<T>, bool> = true>
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

} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_POW_HPP
