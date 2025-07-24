// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

// Propogates up from boost.math
#define _SILENCE_CXX23_DENORM_DEPRECATION_WARNING

#include <boost/decimal.hpp>

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wold-style-cast"
#  pragma clang diagnostic ignored "-Wundef"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wsign-conversion"
#  pragma clang diagnostic ignored "-Wfloat-equal"
#  if __clang_major__ >= 20
#    pragma clang diagnostic ignored "-Wfortify-source"
#  endif
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wundef"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wsign-conversion"
#  pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

#include <boost/math/special_functions/next.hpp>
#include <boost/math/special_functions/laguerre.hpp>
#include <boost/core/lightweight_test.hpp>
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
    std::uniform_real_distribution<float> dist(-2, 2);

    constexpr auto max_iter {std::is_same<Dec, decimal128_t_t>::value ? N / 4 : N};
    for (std::size_t i {}; i < max_iter / 4; ++i)
    {
        for (unsigned n {}; n < 4; ++n)
        {
            for (unsigned m {}; m < 4; ++m)
            {
                const auto val1 {dist(rng)};
                Dec d1 {val1};

                auto ret_val {boost::math::laguerre(n, m, val1)};
                auto ret_dec {static_cast<float>(assoc_laguerre(n, m, d1))};

                if (!BOOST_TEST(std::fabs(ret_val - ret_dec) < 500*std::numeric_limits<float>::epsilon()))
                {
                    // LCOV_EXCL_START
                    std::cerr << "Val 1: " << val1
                              << "\nDec 1: " << d1
                              << "\nRet val: " << ret_val
                              << "\nRet dec: " << ret_dec
                              << "\nEps: " << std::fabs(ret_val - ret_dec) / std::numeric_limits<float>::epsilon() << std::endl;
                    // LCOV_EXCL_STOP
                }
            }
        }
    }
}

int main()
{
    test<decimal32_t_t>();
    test<decimal64_t_t>();
    test<decimal_fast32_t>();

    #if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
    test<decimal128_t_t>();
    #endif

    return boost::report_errors();
}
