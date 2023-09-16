// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_NUMBERS_HPP
#define BOOST_DECIMAL_NUMBERS_HPP

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <cstdint>

// TODO(mborland): Change the defaults from decimal32 to 64 once it is available
// TODO(mborland): These values only have sufficient precision for 32 and 64. Need different ones if 128 is supported

namespace boost { namespace decimal { namespace numbers {

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec e_v = Dec{UINT64_C(2718281828459045235), -18};

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec log2e_v = Dec{UINT64_C(1442695040888963407), -18};

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec log10e_v = Dec{UINT64_C(4342944819032518277), -19};

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec pi_v = Dec{UINT64_C(3141592653589793238), -18};

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec inv_pi_v = Dec{UINT64_C(3183098861837906715), -19};

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec inv_sqrtpi_v = Dec{UINT64_C(5641895835477562869), -19};

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec ln2_v = Dec{UINT64_C(6931471805599453094), -19};

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec ln10_v = Dec{UINT64_C(2302585092994045684), -18};

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec sqrt2_v = Dec{UINT64_C(1414213562373095049), -18};

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec sqrt3_v = Dec{UINT64_C(1732050807568877294), -18};

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec inv_sqrt3_v = Dec{UINT64_C(5773502691896257645), -19};

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec egamma_v = Dec{UINT64_C(5772156649015328606), -19};

template <typename Dec, std::enable_if_t<detail::is_decimal_floating_point_v<Dec>, bool> = true>
static constexpr Dec phi_v = Dec{UINT64_C(1618033988749894848), -18};

static constexpr auto e {e_v<decimal32>};
static constexpr auto log2e {log2e_v<decimal32>};
static constexpr auto log10e {log10e_v<decimal32>};
static constexpr auto pi {pi_v<decimal32>};
static constexpr auto inv_pi {inv_pi_v<decimal32>};
static constexpr auto inv_sqrtpi {inv_sqrtpi_v<decimal32>};
static constexpr auto ln2 {ln2_v<decimal32>};
static constexpr auto ln10 {ln10_v<decimal32>};
static constexpr auto sqrt2 {sqrt2_v<decimal32>};
static constexpr auto sqrt3 {sqrt3_v<decimal32>};
static constexpr auto inv_sqrt3 {inv_sqrt3_v<decimal32>};
static constexpr auto egamma {egamma_v<decimal32>};
static constexpr auto phi {phi_v<decimal32>};

} // namespace numbers
} // namespace decimal
} // namespace boost

#endif //BOOST_NUMBERS_HPP
