// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_COS_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_COS_IMPL_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <type_traits>
#include <cstdint>

namespace boost { namespace decimal { namespace detail {

template<typename T, std::enable_if_t<detail::is_decimal_floating_point_v<T>, bool> = true>
constexpr auto cos_impl(T x) noexcept -> T
{
    constexpr T zero {0, 0};
    if (x < zero)
    {
        return cos_impl(-x);
    }

    // Constants calculated for [0, pi/4]
    constexpr T a0 {UINT64_C(22805960529562646), -21};
    constexpr T a1 {UINT64_C(39171880037888081), -22};
    constexpr T a2 {UINT64_C(1392392773950284), -18, true};
    constexpr T a3 {UINT64_C(17339629614857501), -22};
    constexpr T a4 {UINT64_C(41666173896377827), -18};
    constexpr T a5 {UINT64_C(77764646000512304), -24};
    constexpr T a6 {UINT64_C(50000000610949535), -17, true};
    constexpr T a7 {UINT64_C(18421494272283811), -26};
    constexpr T a8 {UINT64_C(99999999999908662), -17};

    T u {a0};
    u = fma(u, x, a1);
    u = fma(u, x, a2);
    u = fma(u, x, a3);
    u = fma(u, x, a4);
    u = fma(u, x, a5);
    u = fma(u, x, a6);
    u = fma(u, x, a7);
    u = fma(u, x, a8);

    return u;
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_CMATH_IMPL_COS_IMPL_HPP
