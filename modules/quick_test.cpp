// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <iostream>

import boost2.decimal;

int main()
{
    boost::decimal::decimal32 a {2, 0};
    std::cout << a << std::endl;

    return 0;
}
