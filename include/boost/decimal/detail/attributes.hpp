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
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto storage_width_v = std::is_same<Dec, decimal32>::value || std::is_same<Dec, decimal32_fast>::value ? 32 : 64;

template <>
BOOST_DECIMAL_CONSTEXPR_VARIABLE_SPECIALIZATION auto storage_width_v<decimal128> = 128;

template <>
BOOST_DECIMAL_CONSTEXPR_VARIABLE_SPECIALIZATION auto storage_width_v<decimal128_fast> = 128;

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto precision_v = std::is_same<Dec, decimal32>::value || std::is_same<Dec, decimal32_fast>::value ? 7 : 16;

template <>
BOOST_DECIMAL_CONSTEXPR_VARIABLE_SPECIALIZATION auto precision_v<decimal128> = 34;

template <>
BOOST_DECIMAL_CONSTEXPR_VARIABLE_SPECIALIZATION auto precision_v<decimal128_fast> = 34;

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto bias_v = std::is_same<Dec, decimal32>::value || std::is_same<Dec, decimal32_fast>::value ? 101 : 398;

template <>
BOOST_DECIMAL_CONSTEXPR_VARIABLE_SPECIALIZATION auto bias_v<decimal128> = 6176;

template <>
BOOST_DECIMAL_CONSTEXPR_VARIABLE_SPECIALIZATION auto bias_v<decimal128_fast> = 6176;

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto max_biased_exp_v = std::is_same<Dec, decimal32>::value || std::is_same<Dec, decimal32_fast>::value ? 191 : 767;

template <>
BOOST_DECIMAL_CONSTEXPR_VARIABLE_SPECIALIZATION auto max_biased_exp_v<decimal128> = 12287;

template <>
BOOST_DECIMAL_CONSTEXPR_VARIABLE_SPECIALIZATION auto max_biased_exp_v<decimal128_fast> = 12287;

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto emax_v = std::is_same<Dec, decimal32>::value || std::is_same<Dec, decimal32_fast>::value ? 96 : 384;

template <>
BOOST_DECIMAL_CONSTEXPR_VARIABLE_SPECIALIZATION auto emax_v<decimal128> = 6144;

template <>
BOOST_DECIMAL_CONSTEXPR_VARIABLE_SPECIALIZATION auto emax_v<decimal128_fast> = 6144;

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto emin_v = std::is_same<Dec, decimal32>::value || std::is_same<Dec, decimal32_fast>::value ? -95 : -383;

template <>
BOOST_DECIMAL_CONSTEXPR_VARIABLE_SPECIALIZATION auto emin_v<decimal128> = -6143;

template <>
BOOST_DECIMAL_CONSTEXPR_VARIABLE_SPECIALIZATION auto emin_v<decimal128_fast> = -6143;

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto etiny_v = -bias_v<Dec>;

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto combination_field_width_v = std::is_same<Dec, decimal32>::value || std::is_same<Dec, decimal32_fast>::value ? 11 : 13;

template <>
BOOST_DECIMAL_CONSTEXPR_VARIABLE_SPECIALIZATION auto combination_field_width_v<decimal128> = 17;

template <>
BOOST_DECIMAL_CONSTEXPR_VARIABLE_SPECIALIZATION auto combination_field_width_v<decimal128_fast> = 17;

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto trailing_significand_field_width_v = std::is_same<Dec, decimal32>::value || std::is_same<Dec, decimal32_fast>::value ? 20 : 50;

template <>
BOOST_DECIMAL_CONSTEXPR_VARIABLE_SPECIALIZATION auto trailing_significand_field_width_v<decimal128> = 110;

template <>
BOOST_DECIMAL_CONSTEXPR_VARIABLE_SPECIALIZATION auto trailing_significand_field_width_v<decimal128_fast> = 110;

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto max_significand_v = std::is_same<Dec, decimal32>::value || std::is_same<Dec, decimal32_fast>::value ? 9'999'999 : 9'999'999'999'999'999;

template <>
BOOST_DECIMAL_CONSTEXPR_VARIABLE_SPECIALIZATION auto max_significand_v<decimal128> =
        uint128{UINT64_C(0b1111111111'1111111111'1111111111'1111111111'111111), UINT64_MAX};

template <>
BOOST_DECIMAL_CONSTEXPR_VARIABLE_SPECIALIZATION auto max_significand_v<decimal128_fast> =
        uint128{UINT64_C(542101086242752), UINT64_C(4003012203950112767)};

// sign + decimal digits + '.' + 'e' + '+/-' + max digits of exponent + null term
template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto max_string_length_v = std::is_same<Dec, decimal32>::value || std::is_same<Dec, decimal32_fast>::value ? 15 : 25;

template <>
BOOST_DECIMAL_CONSTEXPR_VARIABLE_SPECIALIZATION auto max_string_length_v<decimal128> = 41;

template <>
BOOST_DECIMAL_CONSTEXPR_VARIABLE_SPECIALIZATION auto max_string_length_v<decimal128_fast> = 41;

BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto storage_width {storage_width_v<decimal32>};
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto precision {precision_v<decimal32>};
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto bias {bias_v<decimal32>};
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto max_biased_exp {max_biased_exp_v<decimal32>};
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto emax {emax_v<decimal32>};
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto emin {emin_v<decimal32>};
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto etiny {etiny_v<decimal32>};
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto combination_field_width {combination_field_width_v<decimal32>};
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto max_significand {max_significand_v<decimal32>};
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto max_string_length {max_string_length_v<decimal32>};

} //namespace detail
} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DETAIL_MASKS_HPP
