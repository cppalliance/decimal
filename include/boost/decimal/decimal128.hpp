// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DECIMAL128_HPP
#define BOOST_DECIMAL_DECIMAL128_HPP

#include <cassert>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <iostream>
#include <limits>
#include <type_traits>
#include <sstream>

#include <boost/decimal/fwd.hpp>
#include <boost/decimal/detail/attributes.hpp>
#include <boost/decimal/detail/apply_sign.hpp>
#include <boost/decimal/detail/bit_cast.hpp>
#include <boost/decimal/detail/config.hpp>
#include <boost/decimal/detail/emulated128.hpp>
#include <boost/decimal/detail/emulated256.hpp>
#include <boost/decimal/detail/fenv_rounding.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <boost/decimal/detail/parser.hpp>
#include <boost/decimal/detail/power_tables.hpp>
#include <boost/decimal/detail/ryu/ryu_generic_128.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/utilities.hpp>
#include <boost/decimal/detail/normalize.hpp>
#include <boost/decimal/detail/comparison.hpp>
#include <boost/decimal/detail/mixed_decimal_arithmetic.hpp>
#include <boost/decimal/detail/to_integral.hpp>
#include <boost/decimal/detail/to_float.hpp>
#include <boost/decimal/detail/to_decimal.hpp>
#include <boost/decimal/detail/promotion.hpp>
#include <boost/decimal/detail/io.hpp>
#include <boost/decimal/detail/check_non_finite.hpp>
#include <boost/decimal/detail/shrink_significand.hpp>
#include <boost/decimal/detail/cmath/isfinite.hpp>
#include <boost/decimal/detail/cmath/fpclassify.hpp>
#include <boost/decimal/detail/cmath/abs.hpp>
#include <boost/decimal/detail/cmath/floor.hpp>
#include <boost/decimal/detail/cmath/ceil.hpp>

namespace boost {
namespace decimal {

namespace detail {

// See IEEE 754 section 3.5.2
static constexpr uint128 d128_inf_mask {d64_inf_mask, UINT64_C(0)};
static constexpr uint128 d128_nan_mask {d64_nan_mask, UINT64_C(0)};
static constexpr uint128 d128_snan_mask {d64_snan_mask, UINT64_C(0)};
static constexpr uint128 d128_comb_inf_mask {d64_comb_inf_mask, UINT64_C(0)};
static constexpr uint128 d128_comb_nan_mask {d64_comb_nan_mask, UINT64_C(0)};
static constexpr uint128 d128_exp_snan_mask {d64_exp_snan_mask, UINT64_C(0)};

// Masks to update the significand based on the combination field
// In these first three 00, 01, or 10 are the leading 2 bits of the exp
// and the trailing 3 bits are to be concatenated onto the significand
//
//    Comb.  Exponent          Significand
// s 00 TTT (00)eeeeeeeeeeee (0TTT) 110-bits
// s 01 TTT (01)eeeeeeeeeeee (0TTT) 110-bits
// s 10 TTT (10)eeeeeeeeeeee (0TTT) 110-bits
static constexpr std::uint64_t d128_significand_bits = UINT64_C(110);
static constexpr std::uint64_t d128_exponent_bits = UINT64_C(12);

static constexpr uint128 d128_sign_mask {d64_sign_mask, UINT64_C(0)};
static constexpr uint128 d128_combination_field_mask {d64_combination_field_mask, UINT64_C(0)};
static constexpr uint128 d128_exponent_mask {UINT64_C(0b0'00000'111111111111'0000000000'0000000000'0000000000'0000000000'000000),
                                             UINT64_C(0)};

static constexpr uint128 d128_significand_mask {UINT64_C(0b1111111111'1111111111'1111111111'1111111111'111111), UINT64_MAX};

static constexpr uint128 d128_comb_01_mask {d64_comb_01_mask, UINT64_C(0)};
static constexpr uint128 d128_comb_10_mask {d64_comb_10_mask, UINT64_C(0)};
static constexpr uint128 d128_comb_00_01_10_significand_bits {d64_comb_00_01_10_significand_bits, UINT64_C(0)};

// This mask is used to determine if we use the masks above or below since 11 TTT is invalid
static constexpr uint128 d128_comb_11_mask {d64_comb_11_mask, UINT64_C(0)};
static constexpr uint128 d128_comb_11_exp_bits {d64_comb_11_exp_bits, UINT64_C(0)};
static constexpr uint128 d128_comb_11_significand_bits {d64_comb_11_significand_bits, UINT64_C(0)};

// For these masks the first two bits of the combination field imply 100 T as the
// leading bits of the significand and then bits 3 and 4 are the exp
//
//    Comb.  Exponent          Significand
// s 1100 T (00)eeeeeeeeeeee (100T) 110-bits
// s 1101 T (01)eeeeeeeeeeee (100T) 110-bits
// s 1110 T (10)eeeeeeeeeeee (100T) 110-bits
static constexpr uint128 d128_comb_1101_mask {d64_comb_1101_mask, UINT64_C(0)};
static constexpr uint128 d128_comb_1110_mask {d64_comb_1110_mask, UINT64_C(0)};

// Powers of 2 used to determine the size of the significand
static constexpr uint128 d128_no_combination {d128_significand_mask};
static constexpr uint128 d128_big_combination {UINT64_C(0b111'1111111111'1111111111'1111111111'1111111111'111111),
                                               UINT64_MAX};

// Exponent Fields
static constexpr std::uint64_t d128_max_exp_no_combination {0b111111111111};
static constexpr std::uint64_t d128_exp_one_combination {0b1'111111111111};
static constexpr std::uint64_t d128_max_biased_exp {0b10'111111111111};
static constexpr uint128 d128_small_combination_field_mask {UINT64_C(0b111'0000000000'0000000000'0000000000'0000000000'000000),
                                                            UINT64_C(0)};
static constexpr uint128 d128_big_combination_field_mask {UINT64_C(0b1'0000000000'0000000000'0000000000'0000000000'000000),
                                                          UINT64_C(0)};

struct decimal128_components
{
    uint128 sig;
    std::int32_t exp;
    bool sign;
};

} //namespace detail

class decimal128 final
{

};

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DECIMAL128_HPP
