// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_POWER_TABLES_HPP
#define BOOST_DECIMAL_DETAIL_POWER_TABLES_HPP

#include <array>
#include <cstdint>
#include <cassert>

namespace boost { namespace decimal { namespace detail {

static constexpr std::array<std::uint64_t, 20> powers_of_10 =
{{
         UINT64_C(1), UINT64_C(10), UINT64_C(100), UINT64_C(1000), UINT64_C(10000), UINT64_C(100000), UINT64_C(1000000),
         UINT64_C(10000000), UINT64_C(100000000), UINT64_C(1000000000), UINT64_C(10000000000), UINT64_C(100000000000),
         UINT64_C(1000000000000), UINT64_C(10000000000000), UINT64_C(100000000000000), UINT64_C(1000000000000000),
         UINT64_C(10000000000000000), UINT64_C(100000000000000000), UINT64_C(1000000000000000000), UINT64_C(10000000000000000000)
 }};

template <typename T>
constexpr T pow10(int n) noexcept
{
    assert(n >= 0);
    return static_cast<T>(powers_of_10[n]);
}

}}} // Namespaces

#endif // BOOST_DECIMAL_DETAIL_POWER_TABLES_HPP
