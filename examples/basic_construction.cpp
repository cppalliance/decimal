// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <iostream>
#include <iomanip>

int main()
{
    using namespace boost::decimal;

    constexpr decimal32 val_1 {100};         // Construction from an integer
    constexpr decimal32 val_2 {10, 1};       // Construction from a signed integer and exponent
    constexpr decimal32 val_3 {1U, 2, false}; // Construction from an unsigned integer, exponent, and sign

    std::cout << "Val_1: " << val_1 << '\n'
              << "Val_2: " << val_2 << '\n'
              << "Val_3: " << val_3 << '\n';

    if (val_1 == val_2 && val_2 == val_3 && val_1 == val_3)
    {
        std::cout << "All equal values" << std::endl;
    }

    constexpr decimal64 val_4 {decimal64{2, -1} + decimal64{1, -1}};
    constexpr double float_val_4 {0.2 + 0.1};
    const decimal64 val_5 { float_val_4 };

    std::cout << std::setprecision(17) << "Val_4: " << val_4 << '\n'
              <<  "Float: " << float_val_4 << '\n'
              <<  "Val_5: " << val_5 << '\n';

    if (val_4 == val_5)
    {
        std::cout << "Floats are equal" << std::endl;
    }
    else
    {
        std::cout << "Floats are not equal" << std::endl;
    }

    return 0;
}

