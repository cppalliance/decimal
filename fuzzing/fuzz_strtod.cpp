// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <iostream>
#include <exception>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size)
{
    try
    {
        auto c_data = reinterpret_cast<const char*>(data);

        char** endptr;

        boost::decimal::decimal32 f_val = boost::decimal::strtod32(c_data, endptr);

        boost::decimal::decimal64 val = boost::decimal::strtod64(c_data, endptr);

        boost::decimal::decimal128 ld_val = boost::decimal::strtod128(c_data, endptr);
    }
    catch(...)
    {
        std::cerr << "Error with: " << data << std::endl;
        std::terminate();
    }

    return 0;
}
