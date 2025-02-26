// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

// GCC versions >= 10 don't adhere to pragma diagnostic ignored -Wstringop-overflow
// So we exclude them this way
#if defined(__GNUC__) && __GNUC__ >= 10

int main()
{
    return 0;
}

#else

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>

#if defined(__cpp_consteval) && __cpp_consteval >= 201811L && !defined(BOOST_DECIMAL_NO_CONSTEVAL_DETECTION)

#include <string_view>

using namespace boost::decimal;

template <typename T>
consteval bool test_value(T value, std::string_view res)
{
    char buffer[256];
    const auto r = to_chars(buffer, buffer + sizeof(buffer), value, chars_format::scientific, 40);
    *r.ptr = '\0';
    std::string_view buffer_view {buffer};
    return res == buffer_view;
}

int main()
{
    // Max
    static_assert(test_value(std::numeric_limits<decimal32>::max(), "9.9999990000000000000000000000000000000000e+96"));
    static_assert(test_value(std::numeric_limits<decimal32_fast>::max(), "9.9999990000000000000000000000000000000000e+96"));
    static_assert(test_value(std::numeric_limits<decimal64>::max(), "9.9999999999999990000000000000000000000000e+384"));
    static_assert(test_value(std::numeric_limits<decimal64_fast>::max(), "9.9999999999999990000000000000000000000000e+384"));
    static_assert(test_value(std::numeric_limits<decimal128>::max(), "9.9999999999999999999999999999999990000000e+6144"));
    static_assert(test_value(std::numeric_limits<decimal128_fast>::max(), "9.9999999999999999999999999999999990000000e+6144"));

    // Epsilon
    static_assert(test_value(std::numeric_limits<decimal32>::epsilon(), "1.0000000000000000000000000000000000000000e-06"));
    static_assert(test_value(std::numeric_limits<decimal32_fast>::epsilon(), "1.0000000000000000000000000000000000000000e-06"));
    static_assert(test_value(std::numeric_limits<decimal64>::epsilon(), "1.0000000000000000000000000000000000000000e-15"));
    static_assert(test_value(std::numeric_limits<decimal64_fast>::epsilon(), "1.0000000000000000000000000000000000000000e-15"));
    static_assert(test_value(std::numeric_limits<decimal128>::epsilon(), "1.0000000000000000000000000000000000000000e-33"));
    static_assert(test_value(std::numeric_limits<decimal128_fast>::epsilon(), "1.0000000000000000000000000000000000000000e-33"));

    // Min
    static_assert(test_value(std::numeric_limits<decimal32>::min(), "1.0000000000000000000000000000000000000000e-95"));
    static_assert(test_value(std::numeric_limits<decimal32_fast>::min(), "1.0000000000000000000000000000000000000000e-95"));
    static_assert(test_value(std::numeric_limits<decimal64>::min(), "1.0000000000000000000000000000000000000000e-383"));
    static_assert(test_value(std::numeric_limits<decimal64_fast>::min(), "1.0000000000000000000000000000000000000000e-383"));
    static_assert(test_value(std::numeric_limits<decimal128>::min(), "1.0000000000000000000000000000000000000000e-6143"));
    static_assert(test_value(std::numeric_limits<decimal128_fast>::min(), "1.0000000000000000000000000000000000000000e-6143"));

    // Min subnormal
    // Fast types don't support sub-normals so they should return min
    static_assert(test_value(std::numeric_limits<decimal32>::denorm_min(), "1.0000000000000000000000000000000000000000e-101"));
    static_assert(test_value(std::numeric_limits<decimal32_fast>::denorm_min(), "1.0000000000000000000000000000000000000000e-95"));
    static_assert(test_value(std::numeric_limits<decimal64>::denorm_min(), "1.0000000000000000000000000000000000000000e-398"));
    static_assert(test_value(std::numeric_limits<decimal64_fast>::denorm_min(), "1.0000000000000000000000000000000000000000e-383"));
    static_assert(test_value(std::numeric_limits<decimal128>::denorm_min(), "1.0000000000000000000000000000000000000000e-6176"));
    static_assert(test_value(std::numeric_limits<decimal128_fast>::denorm_min(), "1.0000000000000000000000000000000000000000e-6143"));

    // Lowest + max should be 0
    static_assert(test_value(std::numeric_limits<decimal32>::max() + std::numeric_limits<decimal32>::lowest(), "0.0000000000000000000000000000000000000000e+00"));
    static_assert(test_value(std::numeric_limits<decimal32_fast>::max() + std::numeric_limits<decimal32_fast>::lowest(), "0.0000000000000000000000000000000000000000e+00"));
    static_assert(test_value(std::numeric_limits<decimal64>::max() + std::numeric_limits<decimal64>::lowest(), "0.0000000000000000000000000000000000000000e+00"));
    static_assert(test_value(std::numeric_limits<decimal64_fast>::max() + std::numeric_limits<decimal64_fast>::lowest(), "0.0000000000000000000000000000000000000000e+00"));
    static_assert(test_value(std::numeric_limits<decimal128>::max() + std::numeric_limits<decimal128>::lowest(), "0.0000000000000000000000000000000000000000e+00"));
    static_assert(test_value(std::numeric_limits<decimal128_fast>::max() + std::numeric_limits<decimal128_fast>::lowest(), "0.0000000000000000000000000000000000000000e+00"));

    return boost::report_errors();
}

#else

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
    test_value(std::numeric_limits<decimal128>::min(), "1.0000000000000000000000000000000000000000e-6143");
    test_value(std::numeric_limits<decimal128_fast>::min(), "1.0000000000000000000000000000000000000000e-6143");

    // Min subnormal
    // Fast types don't support sub-normals so they should return min
    test_value(std::numeric_limits<decimal32>::denorm_min(), "1.0000000000000000000000000000000000000000e-101");
    test_value(std::numeric_limits<decimal32_fast>::denorm_min(), "1.0000000000000000000000000000000000000000e-95");
    test_value(std::numeric_limits<decimal64>::denorm_min(), "1.0000000000000000000000000000000000000000e-398");
    test_value(std::numeric_limits<decimal64_fast>::denorm_min(), "1.0000000000000000000000000000000000000000e-383");
    test_value(std::numeric_limits<decimal128>::denorm_min(), "1.0000000000000000000000000000000000000000e-6176");
    test_value(std::numeric_limits<decimal128_fast>::denorm_min(), "1.0000000000000000000000000000000000000000e-6143");

    // Lowest + max
    test_value(std::numeric_limits<decimal32>::max() + std::numeric_limits<decimal32>::lowest(), "0.0000000000000000000000000000000000000000e+00");
    test_value(std::numeric_limits<decimal32_fast>::max() + std::numeric_limits<decimal32_fast>::lowest(), "0.0000000000000000000000000000000000000000e+00");
    test_value(std::numeric_limits<decimal64>::max() + std::numeric_limits<decimal64>::lowest(), "0.0000000000000000000000000000000000000000e+00");
    test_value(std::numeric_limits<decimal64_fast>::max() + std::numeric_limits<decimal64_fast>::lowest(), "0.0000000000000000000000000000000000000000e+00");
    test_value(std::numeric_limits<decimal128>::max() + std::numeric_limits<decimal128>::lowest(), "0.0000000000000000000000000000000000000000e+00");
    test_value(std::numeric_limits<decimal128_fast>::max() + std::numeric_limits<decimal128_fast>::lowest(), "0.0000000000000000000000000000000000000000e+00");

    return boost::report_errors();
}

#endif

#endif
