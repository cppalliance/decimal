// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_MASKS_HPP
#define BOOST_DECIMAL_DETAIL_MASKS_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/config.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <type_traits>
#include <cstdint>
#endif

namespace boost {
namespace decimal {
namespace detail {

// Values from IEEE 754-2019 table 3.6

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto storage_width_v = std::is_same<Dec, decimal32>::value ? 32 : 64;

template <>
BOOST_DECIMAL_CLANG_STATIC constexpr auto storage_width_v<decimal128> = 128;

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto precision_v = std::is_same<Dec, decimal32>::value ? 7 : 16;

template <>
BOOST_DECIMAL_CLANG_STATIC constexpr auto precision_v<decimal128> = 34;

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto bias_v = std::is_same<Dec, decimal32>::value ? 101 : 398;

template <>
BOOST_DECIMAL_CLANG_STATIC constexpr auto bias_v<decimal128> = 6176;

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto max_biased_exp_v = std::is_same<Dec, decimal32>::value ? 191 : 767;

template <>
BOOST_DECIMAL_CLANG_STATIC constexpr auto max_biased_exp_v<decimal128> = 12287;

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto emax_v = std::is_same<Dec, decimal32>::value ? 96 : 384;

template <>
BOOST_DECIMAL_CLANG_STATIC constexpr auto emax_v<decimal128> = 6144;

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto emin_v = std::is_same<Dec, decimal32>::value ? -95 : -383;

template <>
BOOST_DECIMAL_CLANG_STATIC constexpr auto emin_v<decimal128> = -6143;

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto etiny_v = -bias_v<Dec>;

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto combination_field_width_v = std::is_same<Dec, decimal32>::value ? 11 : 13;

template <>
BOOST_DECIMAL_CLANG_STATIC constexpr auto combination_field_width_v<decimal128> = 17;

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto trailing_significand_field_width_v = std::is_same<Dec, decimal32>::value ? 20 : 50;

template <>
BOOST_DECIMAL_CLANG_STATIC constexpr auto trailing_significand_field_width_v<decimal128> = 110;

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto max_significand_v = std::is_same<Dec, decimal32>::value ? 9'999'999 : 9'999'999'999'999'999;

template <>
BOOST_DECIMAL_CLANG_STATIC constexpr auto max_significand_v<decimal128> =
        uint128{UINT64_C(0b1111111111'1111111111'1111111111'1111111111'111111), UINT64_MAX};

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto max_string_length_v = std::is_same<Dec, decimal32>::value ? 15 : 25;

template <>
BOOST_DECIMAL_CLANG_STATIC constexpr auto max_string_length_v<decimal128> = 41;

BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto storage_width {storage_width_v<decimal32>};
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto precision {precision_v<decimal32>};
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto bias {bias_v<decimal32>};
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto max_biased_exp {max_biased_exp_v<decimal32>};
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto emax {emax_v<decimal32>};
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto emin {emin_v<decimal32>};
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto etiny {etiny_v<decimal32>};
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto combination_field_width {combination_field_width_v<decimal32>};
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto max_significand {max_significand_v<decimal32>};
BOOST_DECIMAL_ATTRIBUTE_UNUSED static constexpr auto max_string_length {max_string_length_v<decimal32>};

} //namespace detail
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_MASKS_HPP
