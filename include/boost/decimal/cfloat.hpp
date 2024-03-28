// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_DECIMAL_CFLOAT_HPP
#define BOOST_DECIMAL_CFLOAT_HPP

#include <cfloat>
#include <boost/decimal/decimal32.hpp>
#include <boost/decimal/decimal64.hpp>

// number of digits in the coefficient:
#define BOOST_DECIMAL_DEC32_MANT_DIG std::numeric_limits<boost::decimal::decimal32>::digits
#define BOOST_DECIMAL_DEC64_MANT_DIG std::numeric_limits<boost::decimal::decimal64>::digits
#define BOOST_DECIMAL_DEC128_MANT_DIG std::numeric_limits<boost::decimal::decimal128>::digits

// minimum exponent:
#define BOOST_DECIMAL_DEC32_MIN_EXP std::numeric_limits<boost::decimal::decimal32>::min_exponent
#define BOOST_DECIMAL_DEC64_MIN_EXP std::numeric_limits<boost::decimal::decimal64>::min_exponent
#define BOOST_DECIMAL_DEC128_MIN_EXP std::numeric_limits<boost::decimal::decimal128>::min_exponent

// maximum exponent:
#define BOOST_DECIMAL_DEC32_MAX_EXP std::numeric_limits<boost::decimal::decimal32>::max_exponent
#define BOOST_DECIMAL_DEC64_MAX_EXP std::numeric_limits<boost::decimal::decimal64>::max_exponent
#define BOOST_DECIMAL_DEC128_MAX_EXP std::numeric_limits<boost::decimal::decimal128>::max_exponent

// 3.4.3 maximum finite value:
#define BOOST_DECIMAL_DEC32_MAX (std::numeric_limits<boost::decimal::decimal32>::max)()
#define BOOST_DECIMAL_DEC64_MAX (std::numeric_limits<boost::decimal::decimal64>::max)()
#define BOOST_DECIMAL_DEC128_MAX (std::numeric_limits<boost::decimal::decimal128>::max)()

// 3.4.4 epsilon:
#define BOOST_DECIMAL_DEC32_EPSILON std::numeric_limits<boost::decimal::decimal32>::epsilon()
#define BOOST_DECIMAL_DEC64_EPSILON std::numeric_limits<boost::decimal::decimal64>::epsilon()
#define BOOST_DECIMAL_DEC128_EPSILON std::numeric_limits<boost::decimal::decimal128>::epsilon()

// 3.4.5 minimum positive normal value:
#define BOOST_DECIMAL_DEC32_MIN (std::numeric_limits<boost::decimal::decimal32>::min)()
#define BOOST_DECIMAL_DEC64_MIN (std::numeric_limits<boost::decimal::decimal64>::min)()
#define BOOST_DECIMAL_DEC128_MIN (std::numeric_limits<boost::decimal::decimal128>::max)()

// 3.4.6 minimum positive subnormal value:
#define BOOST_DECIMAL_DEC32_SUBNORMAL std::numeric_limits<boost::decimal::decimal32>::denorm_min()
#define BOOST_DECIMAL_DEC64_SUBNORMAL std::numeric_limits<boost::decimal::decimal64>::denorm_min()
#define BOOST_DECIMAL_DEC128_SUBNORMAL std::numeric_limits<boost::decimal::decimal128>::denorm_min()

#endif //BOOST_DECIMAL_CFLOAT_HPP
