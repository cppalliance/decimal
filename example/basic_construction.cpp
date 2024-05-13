// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <iostream>

int main()
{
    using namespace boost::decimal;

    constexpr decimal32 val_1 {100};         // Construction from an integer
    constexpr decimal32 val_2 {10, 1};       // Construction from an integer and exponent
    constexpr decimal32 val_3 {1, 2, false}; // Construction from an integer, exponent, and sign

    std::cout << "Val_1: " << val_1 << '\n'
              << "Val_2: " << val_2 << '\n'
              << "Val_3: " << val_3 << '\n';

    if (val_1 == val_2 && val_2 == val_3 && val_1 == val_3)
    {
        std::cout << "All equal values" << std::endl;
    }

    return 0;
}
