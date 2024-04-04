// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_POWER_TABLES_HPP
#define BOOST_DECIMAL_DETAIL_POWER_TABLES_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/type_traits.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <array>
#include <cstdint>
#endif

namespace boost {
namespace decimal {
namespace detail {

BOOST_DECIMAL_CONSTEXPR_VARIABLE std::array<std::uint64_t, 20> powers_of_10 =
{{
         UINT64_C(1), UINT64_C(10), UINT64_C(100), UINT64_C(1000), UINT64_C(10000), UINT64_C(100000), UINT64_C(1000000),
         UINT64_C(10000000), UINT64_C(100000000), UINT64_C(1000000000), UINT64_C(10000000000), UINT64_C(100000000000),
         UINT64_C(1000000000000), UINT64_C(10000000000000), UINT64_C(100000000000000), UINT64_C(1000000000000000),
         UINT64_C(10000000000000000), UINT64_C(100000000000000000), UINT64_C(1000000000000000000), UINT64_C(10000000000000000000)
 }};

template <typename T>
constexpr auto pow10(T n) noexcept -> T
{
    BOOST_DECIMAL_IF_CONSTEXPR (detail::is_signed_v<T>)
    {
        BOOST_DECIMAL_ASSERT(n >= 0);
    }
    BOOST_DECIMAL_ASSERT(n <= 19);

    return static_cast<T>(powers_of_10[static_cast<std::size_t>(n)]);
}

template <>
constexpr auto pow10(detail::uint128 n) noexcept -> detail::uint128
{
    detail::uint128 res {1};
    if (n <= 19)
    {
        res = powers_of_10[static_cast<std::size_t>(n)];
    }
    else
    {
        res = powers_of_10[static_cast<std::size_t>(19)];
        res *= powers_of_10[static_cast<std::size_t>(n - 19)];
    }

    return res;
}

} // namespace detail
} // namespace decimal
} // namespace boost

#endif // BOOST_DECIMAL_DETAIL_POWER_TABLES_HPP
