// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <iostream>
#include <cassert>

int main()
{
    using namespace boost::decimal;

    decimal64 val {0.25}; // Construction from a double (not recommended but explicit construction is allowed)

    char buffer[256];
    auto r_to = to_chars(buffer, buffer + sizeof(buffer) - 1, val);
    assert(r_to); // checks std::errc()
    *r_to.ptr = '\0';

    decimal64 return_value;
    auto r_from = from_chars(buffer, buffer + std::strlen(buffer), return_value);
    assert(r_from);

    assert(val == return_value);

    std::cout << " Initial Value: " << val << '\n'
              << "Returned Value: " << return_value << std::endl;

    return 0;
}

