// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_CMATH_IMPL_ASIN_IMPL_HPP
#define BOOST_DECIMAL_DETAIL_CMATH_IMPL_ASIN_IMPL_HPP

#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/emulated128.hpp>
#include <boost/decimal/detail/cmath/fma.hpp>
#include <cstdint>

namespace boost {
namespace decimal {
namespace detail {

namespace asin_detail {

// Use a struct of arrays so that we can have static constexpr arrays of coefficients.
// If we don't do this we will end up constructing the values in the arrays each time the function is called
// which leads to massive slowdowns.
//
// See https://github.com/boostorg/math/issues/923 for further information
template <bool b>
struct asin_table_imp
{
    // 10th degree remez polynomial calculated from 0, 0.5
    // Estimated max error: 7.3651618860008751e-11
    static constexpr std::array<decimal32, 11> d32_coeffs = {{
                                                                     decimal32 {UINT64_C(263887099755925), -15},
                                                                     decimal32 {UINT64_C(43491393212832818), -17, true},
                                                                     decimal32 {UINT64_C(38559884786102105), -17},
                                                                     decimal32 {UINT64_C(13977130653211101), -17, true},
                                                                     decimal32 {UINT64_C(54573213517731915), -18},
                                                                     decimal32 {UINT64_C(64851743877986187), -18},
                                                                     decimal32 {UINT64_C(11606701725692841), -19},
                                                                     decimal32 {UINT64_C(16658989049586517), -17},
                                                                     decimal32 {UINT64_C(25906093603686159), -22},
                                                                     decimal32 {UINT64_C(99999996600828589), -17},
                                                                     decimal32 {UINT64_C(73651618860008751), -27}
                                                             }};

    // 20th degree remez polynomial calculated from 0, 0.5
    // Estimated max error: 6.0872797932519911178133457751215133e-19
    static constexpr std::array<decimal64, 21> d64_coeffs = {{
                                                                     decimal64 {UINT64_C(2201841632531125594), -18},
                                                                     decimal64 {UINT64_C(9319383818485265142), -18,
                                                                                true},
                                                                     decimal64 {UINT64_C(1876826158920611297), -17},
                                                                     decimal64 {UINT64_C(2351630530022519158), -17,
                                                                                true},
                                                                     decimal64 {UINT64_C(2046603318375014621), -17},
                                                                     decimal64 {UINT64_C(1304427904865204196), -17,
                                                                                true},
                                                                     decimal64 {UINT64_C(6308794339076719731), -18},
                                                                     decimal64 {UINT64_C(2333806156857836980), -18,
                                                                                true},
                                                                     decimal64 {UINT64_C(6826985955727270693), -19},
                                                                     decimal64 {UINT64_C(1326415745606167277), -19,
                                                                                true},
                                                                     decimal64 {UINT64_C(2747750823768175476), -20},
                                                                     decimal64 {UINT64_C(2660509753516203115), -20},
                                                                     decimal64 {UINT64_C(3977122944636320545), -22},
                                                                     decimal64 {UINT64_C(4461135938842722307), -20},
                                                                     decimal64 {UINT64_C(1826730778134521645), -24},
                                                                     decimal64 {UINT64_C(7499992533825458566), -20},
                                                                     decimal64 {UINT64_C(2034140780525051207), -27},
                                                                     decimal64 {UINT64_C(1666666666327808185), -19},
                                                                     decimal64 {UINT64_C(2987315928933390856), -31},
                                                                     decimal64 {UINT64_C(9999999999999989542), -19},
                                                                     decimal64 {UINT64_C(6087279793251991118), -37}
                                                             }};
};

#if !(defined(__cpp_inline_variables) && __cpp_inline_variables >= 201606L) && (!defined(_MSC_VER) || _MSC_VER != 1900)

template <bool b>
constexpr std::array<decimal32, 11> asin_table_imp<b>::d32_coeffs;

template <bool b>
constexpr std::array<decimal64, 21> asin_table_imp<b>::d64_coeffs;

#endif

using asin_table = asin_table_imp<true>;

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T, typename Array>
constexpr auto asin_helper_impl(T x, const Array &coeffs)
{
    T result {};

    result = coeffs[0];
    for (std::size_t i {1}; i < coeffs.size(); ++i)
    {
        result = fma(result, x, coeffs[i]);
    }

    return result;
}

} //namespace asin_detail

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE T>
constexpr auto asin_impl(T x) noexcept;

template <>
constexpr auto asin_impl<decimal32>(decimal32 x) noexcept
{
    return asin_detail::asin_helper_impl(x, asin_detail::asin_table::d32_coeffs);
}

template <>
constexpr auto asin_impl<decimal64>(decimal64 x) noexcept
{
    return asin_detail::asin_helper_impl(x, asin_detail::asin_table::d64_coeffs);
}

} //namespace detail
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_CMATH_IMPL_ASIN_IMPL_HPP
