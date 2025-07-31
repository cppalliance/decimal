// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <cassert>

using namespace boost::decimal::literals;

template <typename T>
bool float_equal(T lhs, T rhs)
{
    using std::fabs;
    return fabs(lhs - rhs) < std::numeric_limits<T>::epsilon(); // numeric_limits is overloaded for all decimal types
}


int main()
{
    using namespace boost::decimal;

    BOOST_DECIMAL_ATTRIBUTE_UNUSED const auto pi_32 {"3.141592653589793238"_DF};
    BOOST_DECIMAL_ATTRIBUTE_UNUSED const auto pi_64 {"3.141592653589793238"_DD};

    assert(float_equal(pi_32, static_cast<decimal32_t>(pi_64))); // Explicit conversion between decimal types
    assert(float_equal(pi_32, boost::decimal::numbers::pi_v<decimal32_t>)); // Constants available in numbers namespace
    assert(float_equal(pi_64, numbers::pi)); // Default constant type is decimal64_t

    return 0;
}


