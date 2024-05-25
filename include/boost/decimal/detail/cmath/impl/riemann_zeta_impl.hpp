// Copyright 2024 Matt Borland
// Copyright 2024 Christopher Kormanyos
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_RIEMANN_ZETA_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_RIEMANN_ZETA_IMPL_HPP

#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/concepts.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <array>
#include <cstddef>
#include <cstdint>
#endif

namespace boost {
namespace decimal {
namespace detail {

namespace riemann_zeta_detail {

template <bool b, typename T>
struct prime_table_imp
{
    using decimal_type = T;

    using prime_table_t = std::array<decimal_type, 36>;

    // Table[Prime[n], {n, 1, 36, 1}]
    static constexpr prime_table_t primes =
    {{
        // Table[Prime[n], {n, 1, 26, 1}]
        decimal_type {   2 }, decimal_type {   3 }, decimal_type {   5 }, decimal_type {   7 },
        decimal_type {  11 }, decimal_type {  13 }, decimal_type {  17 }, decimal_type {  19 },
        decimal_type {  23 }, decimal_type {  29 }, decimal_type {  31 }, decimal_type {  37 },
        decimal_type {  41 }, decimal_type {  43 }, decimal_type {  47 }, decimal_type {  53 },
        decimal_type {  59 }, decimal_type {  61 }, decimal_type {  67 }, decimal_type {  71 },
        decimal_type {  73 }, decimal_type {  79 }, decimal_type {  83 }, decimal_type {  89 },
        decimal_type {  97 }, decimal_type { 101 }, decimal_type { 103 }, decimal_type { 107 },
        decimal_type { 109 }, decimal_type { 113 }, decimal_type { 127 }, decimal_type { 131 },
        decimal_type { 137 }, decimal_type { 139 }, decimal_type { 149 }, decimal_type { 151 }
    }};
};

template <bool b>
struct riemann_zeta_table_imp
{
private:
    using d32_coeffs_t      = std::array<decimal32,       5>;
    using d32_fast_coeffs_t = std::array<decimal32_fast,  5>;
    using d64_coeffs_t      = std::array<decimal64,       7>;
    using d128_coeffs_t     = std::array<decimal128,     12>;

public:
    static constexpr d32_coeffs_t d32_coeffs =
    {{
        // N[Series[Zeta[x], {x, 1, 4}], 19]

        +::boost::decimal::decimal32 { UINT64_C(5772156649015328606), - 19 - 0 }, // EulerGamma
        +::boost::decimal::decimal32 { UINT64_C(7281584548367672486), - 19 - 1 }, // * (x - 1)
        -::boost::decimal::decimal32 { UINT64_C(4845181596436159242), - 19 - 2 }, // * (x - 1)^2
        -::boost::decimal::decimal32 { UINT64_C(3423057367172243110), - 19 - 3 }, // * (x - 1)^3
        +::boost::decimal::decimal32 { UINT64_C(9689041939447083573), - 19 - 4 }, // * (x - 1)^4
    }};

    static constexpr d32_fast_coeffs_t d32_fast_coeffs =
    {{
        // N[Series[Zeta[x], {x, 1, 4}], 19]

        +::boost::decimal::decimal32_fast { UINT64_C(5772156649015328606), - 19 - 0 }, // EulerGamma
        +::boost::decimal::decimal32_fast { UINT64_C(7281584548367672486), - 19 - 1 }, // * (x - 1)
        -::boost::decimal::decimal32_fast { UINT64_C(4845181596436159242), - 19 - 2 }, // * (x - 1)^2
        -::boost::decimal::decimal32_fast { UINT64_C(3423057367172243110), - 19 - 3 }, // * (x - 1)^3
        +::boost::decimal::decimal32_fast { UINT64_C(9689041939447083573), - 19 - 4 }, // * (x - 1)^4
     }};

    static constexpr d64_coeffs_t d64_coeffs =
    {{
        // N[Series[Zeta[x], {x, 1, 6}], 19]

        +::boost::decimal::decimal64 { UINT64_C(5772156649015328606), - 19 - 0 }, // EulerGamma
        +::boost::decimal::decimal64 { UINT64_C(7281584548367672486), - 19 - 1 }, // * (x - 1)
        -::boost::decimal::decimal64 { UINT64_C(4845181596436159242), - 19 - 2 }, // * (x - 1)^2
        -::boost::decimal::decimal64 { UINT64_C(3423057367172243110), - 19 - 3 }, // * (x - 1)^3
        +::boost::decimal::decimal64 { UINT64_C(9689041939447083573), - 19 - 4 }, // * (x - 1)^4
        -::boost::decimal::decimal64 { UINT64_C(6611031810842189181), - 19 - 5 }, // * (x - 1)^5
        -::boost::decimal::decimal64 { UINT64_C(3316240908752772359), - 19 - 6 }, // * (x - 1)^6
     }};

    static constexpr d128_coeffs_t d128_coeffs =
    {{
        +::boost::decimal::decimal128 { UINT64_C(5772156649015328606), - 19 - 0 }, // EulerGamma
        +::boost::decimal::decimal128 { UINT64_C(7281584548367672486), - 19 - 1 }, // * (x - 1)
        -::boost::decimal::decimal128 { UINT64_C(4845181596436159242), - 19 - 2 }, // * (x - 1)^2
        -::boost::decimal::decimal128 { UINT64_C(3423057367172243110), - 19 - 3 }, // * (x - 1)^3
        +::boost::decimal::decimal128 { UINT64_C(9689041939447083573), - 19 - 4 }, // * (x - 1)^4
        -::boost::decimal::decimal128 { UINT64_C(6611031810842189181), - 19 - 5 }, // * (x - 1)^5
        -::boost::decimal::decimal128 { UINT64_C(3316240908752772359), - 19 - 6 }, // * (x - 1)^6
    }};
};

#if !(defined(__cpp_inline_variables) && __cpp_inline_variables >= 201606L) && (!defined(_MSC_VER) || _MSC_VER != 1900)

template <bool b>
constexpr typename riemann_zeta_table_imp<b>::d32_coeffs_t riemann_zeta_table_imp<b>::d32_coeffs;

template <bool b>
constexpr typename riemann_zeta_table_imp<b>::d64_coeffs_t riemann_zeta_table_imp<b>::d64_coeffs;

template <bool b>
constexpr typename riemann_zeta_table_imp<b>::d128_coeffs_t riemann_zeta_table_imp<b>::d128_coeffs;

template <bool b>
constexpr typename riemann_zeta_table_imp<b>::d32_fast_coeffs_t riemann_zeta_table_imp<b>::d32_fast_coeffs;

template <bool b, typename T>
constexpr typename prime_table_imp<b, T>::prime_table_t prime_table_imp<b, T>::primes;

#endif

} //namespace lgamma_detail

using riemann_zeta_table = riemann_zeta_detail::riemann_zeta_table_imp<true>;

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto riemann_zeta_series_expansion(T x) noexcept;

template <>
constexpr auto riemann_zeta_series_expansion<decimal32>(decimal32 x) noexcept
{
    return taylor_series_result(x, riemann_zeta_table::d32_coeffs);
}

template <>
constexpr auto riemann_zeta_series_expansion<decimal32_fast>(decimal32_fast x) noexcept
{
    return taylor_series_result(x, riemann_zeta_table::d32_fast_coeffs);
}

template <>
constexpr auto riemann_zeta_series_expansion<decimal64>(decimal64 x) noexcept
{
    return taylor_series_result(x, riemann_zeta_table::d64_coeffs);
}

template <>
constexpr auto riemann_zeta_series_expansion<decimal128>(decimal128 x) noexcept
{
    return taylor_series_result(x, riemann_zeta_table::d128_coeffs);
}

template<typename T>
using prime_table_t = typename riemann_zeta_detail::prime_table_imp<true, T>::prime_table_t;

template<typename T>
using prime_table = riemann_zeta_detail::prime_table_imp<true, T>;

template <typename T>
constexpr auto riemann_zeta_decimal_order(T x) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    int n { };

    const T fr10 = frexp10(x, &n);

    constexpr int order_bias
    {
          std::numeric_limits<T>::digits10 < 10 ?  6
        : std::numeric_limits<T>::digits10 < 20 ? 15
        :                                         33
    };

    return n + order_bias;
}

template <typename T>
constexpr auto riemann_zeta_factorial(int nf) noexcept
    BOOST_DECIMAL_REQUIRES(detail::is_decimal_floating_point_v, T)
{
    return { nf <= 1 ? T { 1 } : riemann_zeta_factorial<T>(nf - 1) * nf };
}

} //namespace detail
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_IMPL_RIEMANN_ZETA_IMPL_HPP
