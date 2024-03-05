// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <complex>

#if __cplusplus > 202002L

int main()
{
    using namespace boost::decimal;

    constexpr decimal64 half {5, -1};
    std::complex<decimal64> test_val {half, half};
    const auto res = std::acos(test_val);
    static_cast<void>(res);

    return 0;
}

#else

int main()
{
    using namespace boost::decimal;

    const decimal64 test_val = 1.5707963267948966192313216916397514L;
    BOOST_TEST_EQ(test_val, decimal64{1.5707963267948966192313216916397514L});

    return boost::report_errors();
}

#endif
