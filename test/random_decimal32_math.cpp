// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal/decimal32.hpp>
#include <boost/core/lightweight_test.hpp>
#include <random>
#include <limits>
#include <cmath>
#include <cerrno>

using namespace boost::decimal;

static constexpr auto N {1024U};

template <typename T>
void random_addition()
{
    std::mt19937_64 rng(42);
    std::uniform_int_distribution<T> dist(0, 5'000'000);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal32 dec1 {val1};
        const decimal32 dec2 {val2};

        const decimal32 res = dec1 + dec2;
        const auto res_int = static_cast<T>(res);

        if (!BOOST_TEST_EQ(res_int, val1 + val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << res_int << std::endl;
        }
    }
}

int main()
{
    random_addition<int>();
    random_addition<long>();
    random_addition<long long>();

    return boost::report_errors();
}
