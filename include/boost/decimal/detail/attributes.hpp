// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DETAIL_MASKS_HPP
#define BOOST_DECIMAL_DETAIL_MASKS_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/concepts.hpp>
#include <boost/decimal/detail/promotion.hpp>

#ifndef BOOST_DECIMAL_BUILD_MODULE
#include <type_traits>
#include <cstdint>
#endif

namespace boost {
namespace decimal {
namespace detail {

// Values from IEEE 754-2019 table 3.6
namespace impl {

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
constexpr auto storage_width_v() noexcept -> int
{
    return decimal_val_v<DecimalType> < 64 ? 32 :
           decimal_val_v<DecimalType> < 128 ? 64 : 128;
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
constexpr auto precision_v() noexcept -> int
{
    return decimal_val_v<DecimalType> < 64 ? 7 :
           decimal_val_v<DecimalType> < 128 ? 16 : 34;
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
constexpr auto bias_v() noexcept -> int
{
    return decimal_val_v<DecimalType> < 64 ? 101 :
           decimal_val_v<DecimalType> < 128 ? 398 : 6176;
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
constexpr auto max_biased_exp_v() noexcept -> int
{
    return decimal_val_v<DecimalType> < 64 ? 191 :
           decimal_val_v<DecimalType> < 128 ? 767 : 12287;
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
constexpr auto emax_v() noexcept -> int
{
    return decimal_val_v<DecimalType> < 64 ? 96 :
           decimal_val_v<DecimalType> < 128 ? 384 : 6144;
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
constexpr auto emin_v() noexcept -> int
{
    return decimal_val_v<DecimalType> < 64 ? -95 :
           decimal_val_v<DecimalType> < 128 ? -383 : -6143;
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
constexpr auto combination_field_width_v() noexcept -> int
{
    return decimal_val_v<DecimalType> < 64 ? 11 :
           decimal_val_v<DecimalType> < 128 ? 13 : 17;
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
constexpr auto trailing_significand_field_width_v() noexcept -> int
{
    return decimal_val_v<DecimalType> < 64 ? 20 :
           decimal_val_v<DecimalType> < 128 ? 50 : 110;
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType, std::enable_if_t<decimal_val_v<DecimalType> < 128, bool> = true>
constexpr auto max_significand_v() noexcept
{
    return decimal_val_v<DecimalType> < 64 ? 9'999'999 : 9'999'999'999'999'999;
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType, std::enable_if_t<decimal_val_v<DecimalType> >= 128, bool> = true>
constexpr auto max_significand_v() noexcept
{
    return std::is_same<DecimalType, decimal128>::value ? uint128{UINT64_C(0b1111111111'1111111111'1111111111'1111111111'111111), UINT64_MAX} :
                                                          uint128{UINT64_C(542101086242752), UINT64_C(4003012203950112767)};
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
constexpr auto max_string_length_v() noexcept -> int
{
    return decimal_val_v<DecimalType> < 64 ? 15 :
           decimal_val_v<DecimalType> < 128 ? 25 : 41;
}

template <BOOST_DECIMAL_DECIMAL_FLOATING_TYPE DecimalType>
constexpr auto is_fast_type_v() noexcept -> bool
{
    // The fast types all assign 1 additional bit over the regular types
    return decimal_val_v<DecimalType> % 2 == 1;
}

} // namespace impl

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto storage_width_v = impl::storage_width_v<Dec>();

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto precision_v = impl::precision_v<Dec>();

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto bias_v = impl::bias_v<Dec>();

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto max_biased_exp_v = impl::max_biased_exp_v<Dec>();

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto emax_v = impl::emax_v<Dec>();

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto emin_v = impl::emin_v<Dec>();

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto etiny_v = -impl::bias_v<Dec>();

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto combination_field_width_v = impl::combination_field_width_v<Dec>();

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto trailing_significand_field_width_v = impl::trailing_significand_field_width_v<Dec>();

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto max_significand_v = impl::max_significand_v<Dec>();

// sign + decimal digits + '.' + 'e' + '+/-' + max digits of exponent + null term
template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto max_string_length_v = impl::max_string_length_v<Dec>();

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
BOOST_DECIMAL_ATTRIBUTE_UNUSED BOOST_DECIMAL_CONSTEXPR_VARIABLE auto is_fast_type_v = impl::is_fast_type_v<Dec>();

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
