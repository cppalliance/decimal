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
    return 0;
}

#endif
