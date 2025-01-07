// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

// MSVC 14.3 has a conversion error in <algorithm> so we need to try and supress that everywhere
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4244)
#endif

#include <boost/decimal.hpp>
#include <iostream>

#ifdef BOOST_CRYPT_HAS_FORMAT_SUPPORT

#include <format>

int main()
{
    constexpr boost::decimal::decimal64 val1 {314, -2};
    constexpr boost::decimal::decimal32 val2 {3141, -3};

    std::cout << std::format("{:10.3e}", val1) << '\n';
    std::cout << std::format("{:10.3e}", val2) << std::endl;

    return 0;
}

#else

int main()
{
    std::cout << "<format> is unsupported" << std::endl;
    return 0;
}

#endif
