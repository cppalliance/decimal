// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <iostream>

#if defined(BOOST_DECIMAL_HAS_FMTLIB_SUPPORT) && defined(BOOST_DECIMAL_TEST_FMT)

#include <boost/decimal/fmt_format.hpp>

int main()
{
    constexpr boost::decimal::decimal64_t val1 {314, -2};
    constexpr boost::decimal::decimal32_t val2 {3141, -3};

    std::cout << fmt::format("{:10.3e}", val1) << '\n';
    std::cout << fmt::format("{:10.3e}", val2) << std::endl;

    return 0;
}

#else

int main()
{
    std::cout << "<fmt/format> is unsupported" << std::endl;
    return 0;
}

#endif
