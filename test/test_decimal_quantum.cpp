// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include "mini_to_chars.hpp"

#include <boost/decimal/decimal32.hpp>
#include <boost/core/lightweight_test.hpp>
#include <bitset>
#include <limits>
#include <random>
#include <climits>
#include <cmath>
#include <cerrno>

using namespace boost::decimal;

static constexpr auto N {1024U};

// NOLINTNEXTLINE : Seed with a constant for repeatability
static std::mt19937_64 rng(42); // NOSONAR : Global rng is not const

template <typename Dec>
void test_same_quantum()
{
    std::uniform_int_distribution<std::int64_t> sig(1'000'000, 9'999'999);
    std::uniform_int_distribution<std::int32_t> exp(std::numeric_limits<Dec>::min_exponent10 + 19,
                                                    std::numeric_limits<Dec>::max_exponent10 - 19);

    for (std::size_t i {}; i < N; ++i)
    {
        auto exp1 {exp(rng)};
        auto exp2 {exp(rng)};

        const Dec val1 {sig(rng), exp1};
        const Dec val2 {sig(rng), exp2};


        if (exp1 == exp2)
        {
            if (!BOOST_TEST(samequantumd32(val1, val2)))
            {
                std::cerr << "Val 1: " << val1
                          << "\nVal 2: " << val2 << std::endl;
            }
        }
        else
        {
            if (!BOOST_TEST(!samequantumd32(val1, val2)))
            {
                std::cerr << "Val 1: " << val1
                          << "\nVal 2: " << val2 << std::endl;
            }
        }
    }
}

int main()
{
    test_same_quantum<decimal32>();

    return boost::report_errors();
}
