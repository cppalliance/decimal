// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include "mini_to_chars.hpp"
#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <iostream>
#include <sstream>
#include <random>

using namespace boost::decimal;

static constexpr auto N {1024U};

template <typename T>
void roundtrip_strtod()
{
    std::mt19937_64 rng(42);

    std::uniform_int_distribution<std::int64_t> sig(1'000'000, 9'999'999);
    std::uniform_int_distribution<std::int32_t> exp(std::numeric_limits<T>::min_exponent10 + 19,
                                                    std::numeric_limits<T>::max_exponent10 - 19);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val {sig(rng), exp(rng)};
        std::stringstream ss;
        ss << val;
        char* endptr;

        const T return_val {boost::decimal::strtod(ss.str().c_str(), &endptr)};
        const auto len {std::strlen(ss.str().c_str())};
        const auto dist {endptr - ss.str().c_str()};

        if (!BOOST_TEST_EQ(val, return_val) && BOOST_TEST_EQ(len, dist))
        {
            std::cerr << "Val 1: " << val
                      << "\nVal 2: " << return_val
                      << "\nStrlen: " << len
                      << "\n  Dist: " << dist << std::endl;
        }
    }
}

int main()
{
    roundtrip_strtod<decimal32>();

    return boost::report_errors();
}
