// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/type_traits.hpp>

int main()
{
    using namespace boost::decimal;

    #ifdef BOOST_DECIMAL_HAS_BOOST_TYPE_TRAITS
    static_assert(boost::is_arithmetic<decimal32>::value, "Incorrect definition");
    static_assert(boost::is_arithmetic<decimal64>::value, "Incorrect definition");
    static_assert(boost::is_arithmetic<decimal128>::value, "Incorrect definition");

    static_assert(boost::is_fundamental<decimal32>::value, "Incorrect definition");
    static_assert(boost::is_fundamental<decimal64>::value, "Incorrect definition");
    static_assert(boost::is_fundamental<decimal128>::value, "Incorrect definition");

    static_assert(boost::is_scalar<decimal32>::value, "Incorrect definition");
    static_assert(boost::is_scalar<decimal64>::value, "Incorrect definition");
    static_assert(boost::is_scalar<decimal128>::value, "Incorrect definition");

    static_assert(!boost::is_class<decimal32>::value, "Incorrect definition");
    static_assert(!boost::is_class<decimal64>::value, "Incorrect definition");
    static_assert(!boost::is_class<decimal128>::value, "Incorrect definition");

    static_assert(boost::is_pod<decimal32>::value, "Incorrect definition");
    static_assert(boost::is_pod<decimal64>::value, "Incorrect definition");
    static_assert(boost::is_pod<decimal128>::value, "Incorrect definition");
    #endif

    static_assert(boost::is_decimal_floating_point<decimal32>::value, "Incorrect definition");
    static_assert(boost::is_decimal_floating_point<decimal64>::value, "Incorrect definition");
    static_assert(boost::is_decimal_floating_point<decimal128>::value, "Incorrect definition");

    static_assert(!boost::is_decimal_floating_point<float>::value, "Incorrect definition");
    static_assert(!boost::is_decimal_floating_point<double>::value, "Incorrect definition");
    static_assert(!boost::is_decimal_floating_point<long double>::value, "Incorrect definition");

    return 0;
}
