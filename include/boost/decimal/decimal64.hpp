// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_DECIMAL64_HPP
#define BOOST_DECIMAL_DECIMAL64_HPP

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
#include <boost/decimal/detail/fast_float/compute_float32.hpp>
#include <boost/decimal/detail/fast_float/compute_float64.hpp>
#include <boost/decimal/detail/fenv_rounding.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <boost/decimal/detail/parser.hpp>
#include <boost/decimal/detail/power_tables.hpp>
#include <boost/decimal/detail/ryu/ryu_generic_128.hpp>
#include <boost/decimal/detail/type_traits.hpp>
#include <boost/decimal/detail/utilities.hpp>
#include <boost/decimal/detail/cmath/isfinite.hpp>
#include <boost/decimal/detail/cmath/fpclassify.hpp>
#include <boost/decimal/detail/cmath/abs.hpp>
#include <boost/decimal/detail/cmath/floor.hpp>
#include <boost/decimal/detail/cmath/ceil.hpp>

namespace boost {
namespace decimal {
namespace detail {

// See IEEE 754 dection 3.5.2
static constexpr auto d64_inf_mask = UINT64_C(0b0'11110'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
static constexpr auto d64_nan_mask = UINT64_C(0b0'11111'00000000'0000000000'0000000000'0000000000'0000000000'0000000000);
static constexpr auto d64_snan_mask = UINT64_C(0b0'11111'10000000'0000000000'0000000000'0000000000'0000000000'0000000000);
static constexpr auto d64_comb_inf_mask = UINT64_C(0b11110);
static constexpr auto d64_comb_nan_mask = UINT64_C(0b11111);
static constexpr auto d64_exp_snan_mask = UINT64_C(0b10000000);

// Masks to update the significand based on the combination field
// In these first three 00, 01, or 10 are the leading 2 bits of the exp
// and the trailing 3 bits are to be concatenated onto the significand (23 bits total)
//
//    Comb.  Exponent          Significand
// s 00 TTT (00)eeeeeeee (0TTT)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
// s 01 TTT (01)eeeeeeee (0TTT)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
// s 10 TTT (10)eeeeeeee (0TTT)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
static constexpr std::uint64_t d64_comb_01_mask = 0b01000;
static constexpr std::uint64_t d64_comb_10_mask = 0b10000;

// This mask is used to determine if we use the masks above or below since 11 TTT is invalid
static constexpr std::uint64_t d64_comb_11_mask = 0b11000;
static constexpr std::uint64_t d64_comb_11_exp_bits = 0b00110;
static constexpr std::uint64_t d64_comb_11_significand_bits = 0b00001;

// For these masks the first two bits of the combination field imply 100 T as the
// leading bits of the significand and then bits 3 and 4 are the exp
//
//    Comb.  Exponent          Significand
// s 1100 T (00)eeeeeeee (100T)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
// s 1101 T (01)eeeeeeee (100T)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
// s 1110 T (10)eeeeeeee (100T)[tttttttttt][tttttttttt][tttttttttt][tttttttttt][tttttttttt]
static constexpr std::uint64_t d64_comb_1101_mask = 0b11010;
static constexpr std::uint64_t d64_comb_1110_mask = 0b11100;

// Powers of 2 used to determine the size of the significand
static constexpr std::uint64_t d64_no_combination = 0b1111111111'1111111111'1111111111'1111111111'1111111111;
static constexpr std::uint64_t d64_big_combination = 0b111'1111111111'1111111111'1111111111'1111111111'1111111111;

// Exponent fields
static constexpr std::uint64_t d64_max_exp_no_combination = 0b11111111;
static constexpr std::uint64_t d64_exp_combination_field_mask = d64_max_exp_no_combination;
static constexpr std::uint64_t d64_exp_one_combination = 0b1'11111111;
static constexpr std::uint64_t d64_max_biased_exp = 0b10'11111111;
static constexpr std::uint64_t d64_small_combination_field_mask = 0b111'0000000000'0000000000'0000000000'0000000000'0000000000;
static constexpr std::uint64_t d64_big_combination_field_mask = 0b1'0000000000'0000000000'0000000000'0000000000'0000000000;

// Constexpr construction form an uint64_t without having to memcpy
static constexpr std::uint64_t d64_construct_sign_mask = 0b1'00000'00000000'0000000000'0000000000'0000000000'0000000000'0000000000;
static constexpr std::uint64_t d64_construct_combination_mask = 0b0'11111'00000000'0000000000'0000000000'0000000000'0000000000'0000000000;
static constexpr std::uint64_t d64_construct_exp_mask = 0b0'00000'11111111'0000000000'0000000000'0000000000'0000000000'0000000000;
static constexpr std::uint64_t d64_construct_significand_mask = d64_no_combination;

} //namespace detail

class decimal64 final
{
private:
    #pragma pack(push, 1)

    struct data_layout_
    {
        #ifdef BOOST_DECIMAL_ENDIAN_LITTLE_BYTE

        std::uint64_t significand : 50;
        std::uint64_t exponent : 8;
        std::uint64_t combination_field : 5;
        std::uint64_t sign : 1;

        #else

        std::uint64_t sign : 1;
        std::uint64_t combination_field : 5;
        std::uint64_t exponent : 8;
        std::uint64_t significand : 50;

        #endif
    };

    #pragma pack(pop)

    data_layout_ bits_ {};

public:
    // 3.2.3.1 construct/copy/destroy
    constexpr decimal64() noexcept = default;
};

} //namespace decimal
} //namespace boost

#endif //BOOST_DECIMAL_DECIMAL64_HPP
