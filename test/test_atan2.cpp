// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

// Propagates up from boost.math
#define _SILENCE_CXX23_DENORM_DEPRECATION_WARNING

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/math/special_functions/next.hpp>
#include <iostream>
#include <random>
#include <cmath>

#if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
static constexpr auto N = static_cast<std::size_t>(128U); // Number of trials
#else
static constexpr auto N = static_cast<std::size_t>(128U >> 4U); // Number of trials
#endif

static std::mt19937_64 rng(42);

using namespace boost::decimal;

template <typename Dec>
void test()
{
    constexpr auto max_iter {std::is_same<Dec, decimal128>::value ? N / 4 : N};

    for (std::size_t n {}; n < max_iter; ++n)
    {
        std::uniform_real_distribution<float> vals(-3.14F, 3.14F);
        const auto val1 {vals(rng)};
        const auto val2 {vals(rng)};
        Dec d1 {val1};
        Dec d2 {val2};

        auto ret_val {std::atan2(val1, val2)};
        auto ret_dec {static_cast<float>(atan2(d1, d2))};

        const auto distance {std::fabs(boost::math::float_distance(ret_val, ret_dec))};
        if (!BOOST_TEST(distance < 100))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nVal 2: " << val2
                      << "\nfabs(y/x): " << std::fabs(val2 / val1)
                      << "\nDec 1: " << d1
                      << "\nDec 2: " << d2
                      << "\nfabs(y/x): " << fabs(d2 / d1)
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << distance << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    // Edge cases
    std::uniform_int_distribution<int> one(1,1);
}

int main()
{
    test<decimal32>();
    test<decimal64>();

    return boost::report_errors();
}
