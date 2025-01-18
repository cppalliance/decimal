// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>

using namespace boost::decimal;

template <typename T>
void test_value(T value, const char* res)
{
    char buffer[256];
    const auto r = to_chars(buffer, buffer + sizeof(buffer), value, chars_format::scientific, 40);
    *r.ptr = '\0';
    BOOST_TEST(r);
    BOOST_TEST_CSTR_EQ(buffer, res);
}

int main()
{
    // Max
    test_value(std::numeric_limits<decimal32>::max(), "9.9999990000000000000000000000000000000000e+96");
    test_value(std::numeric_limits<decimal32_fast>::max(), "9.9999990000000000000000000000000000000000e+96");
    test_value(std::numeric_limits<decimal64>::max(), "9.9999999999999990000000000000000000000000e+384");
    test_value(std::numeric_limits<decimal64_fast>::max(), "9.9999999999999990000000000000000000000000e+384");
    test_value(std::numeric_limits<decimal128>::max(), "9.9999999999999999999999999999999990000000e+6144");
    test_value(std::numeric_limits<decimal128_fast>::max(), "9.9999999999999999999999999999999990000000e+6144");

    // Epsilon
    test_value(std::numeric_limits<decimal32>::epsilon(), "1.0000000000000000000000000000000000000000e-06");
    test_value(std::numeric_limits<decimal32_fast>::epsilon(), "1.0000000000000000000000000000000000000000e-06");
    test_value(std::numeric_limits<decimal64>::epsilon(), "1.0000000000000000000000000000000000000000e-15");
    test_value(std::numeric_limits<decimal64_fast>::epsilon(), "1.0000000000000000000000000000000000000000e-15");
    test_value(std::numeric_limits<decimal128>::epsilon(), "1.0000000000000000000000000000000000000000e-33");
    test_value(std::numeric_limits<decimal128_fast>::epsilon(), "1.0000000000000000000000000000000000000000e-33");

    // Min
    test_value(std::numeric_limits<decimal32>::min(), "1.0000000000000000000000000000000000000000e-95");
    test_value(std::numeric_limits<decimal32_fast>::min(), "1.0000000000000000000000000000000000000000e-95");
    test_value(std::numeric_limits<decimal64>::min(), "1.0000000000000000000000000000000000000000e-383");
    test_value(std::numeric_limits<decimal64_fast>::min(), "1.0000000000000000000000000000000000000000e-383");
    test_value(std::numeric_limits<decimal128>::epsilon(), "1.0000000000000000000000000000000000000000e-33");
    test_value(std::numeric_limits<decimal128_fast>::epsilon(), "1.0000000000000000000000000000000000000000e-33");

    // Min subnormal
    // Fast types don't support sub-normals so they should return min
    test_value(std::numeric_limits<decimal32>::denorm_min(), "1.0000000000000000000000000000000000000000e-101");
    test_value(std::numeric_limits<decimal32_fast>::denorm_min(), "1.0000000000000000000000000000000000000000e-95");
    test_value(std::numeric_limits<decimal64>::denorm_min(), "1.0000000000000000000000000000000000000000e-398");
    test_value(std::numeric_limits<decimal64_fast>::denorm_min(), "1.0000000000000000000000000000000000000000e-383");
    test_value(std::numeric_limits<decimal128>::denorm_min(), "1.0000000000000000000000000000000000000000e-6176");
    test_value(std::numeric_limits<decimal128_fast>::denorm_min(), "1.0000000000000000000000000000000000000000e-6143");

    return boost::report_errors();
}
