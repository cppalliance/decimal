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

        const auto formats = {boost::decimal::chars_format::general, boost::decimal::chars_format::fixed,
                              boost::decimal::chars_format::scientific, boost::decimal::chars_format::hex};

        for (const auto format : formats)
        {
            boost::decimal::decimal32 f_val;
            boost::decimal::from_chars(c_data, c_data + size, f_val, format);

            boost::decimal::decimal64 val;
            boost::decimal::from_chars(c_data, c_data + size, val, format);

            boost::decimal::decimal128 ld_val;
            boost::decimal::from_chars(c_data, c_data + size, ld_val, format);
        }
    }
    catch(...)
    {
        std::cerr << "Error with: " << data << std::endl;
        std::terminate();
    }

    return 0;
}
