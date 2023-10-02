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
