// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

// Propagates up from boost.math
#define _SILENCE_CXX23_DENORM_DEPRECATION_WARNING

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/math/special_functions/next.hpp>

#include <cmath>
#include <iostream>
#include <random>
#include <sstream>

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
        const auto tol {std::fabs(val1 / val2) < 24.0F ? 1000 : 1e5};
        if (!BOOST_TEST(distance < tol))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nVal 2: " << val2
                      << "\nfabs(y/x): " << std::fabs(val1 / val2)
                      << "\nDec 1: " << d1
                      << "\nDec 2: " << d2
                      << "\nfabs(y/x): " << fabs(d1 / d2)
                      << "\nRet val: " << ret_val
                      << "\nRet dec: " << ret_dec
                      << "\nEps: " << distance << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    // Edge cases
    std::uniform_int_distribution<int> one(1,1);

    BOOST_TEST(isnan(atan2(Dec{one(rng)}, std::numeric_limits<Dec>::quiet_NaN())));
    BOOST_TEST(isnan(atan2(std::numeric_limits<Dec>::quiet_NaN(), Dec{one(rng)})));
    BOOST_TEST_EQ(atan2(Dec{0 * one(rng)}, -Dec(1)), numbers::pi_v<Dec>);
    BOOST_TEST_EQ(atan2(Dec{0 * -one(rng)}, -Dec(1)), numbers::pi_v<Dec>);
    BOOST_TEST_EQ(atan2(Dec{0 * one(rng)}, Dec(1)), Dec{0 * one(rng)});
    BOOST_TEST_EQ(atan2(std::numeric_limits<Dec>::infinity(), Dec{one(rng)}), numbers::pi_v<Dec> / 2);
    BOOST_TEST_EQ(atan2(-std::numeric_limits<Dec>::infinity(), Dec{one(rng)}), -numbers::pi_v<Dec> / 2);
    BOOST_TEST_EQ(atan2(std::numeric_limits<Dec>::infinity(), -std::numeric_limits<Dec>::infinity()), 3 * one(rng) * numbers::pi_v<Dec> / 4);
    BOOST_TEST_EQ(atan2(-std::numeric_limits<Dec>::infinity(), -std::numeric_limits<Dec>::infinity()), -3 * one(rng) * numbers::pi_v<Dec> / 4);

    const auto one_rng_pos   = one(rng);
    const auto atan2_inf_pos = atan2(std::numeric_limits<Dec>::infinity(), std::numeric_limits<Dec>::infinity());
    const auto ctrl_inf_pos  = one_rng_pos * numbers::pi_v<Dec> / 4;
    BOOST_TEST_EQ(atan2_inf_pos, ctrl_inf_pos);

    const auto one_rng_neg = -one(rng);
    const auto atan2_inf_neg = atan2(-std::numeric_limits<Dec>::infinity(), std::numeric_limits<Dec>::infinity());
    const auto ctrl_inf_neg  = one_rng_neg * numbers::pi_v<Dec> / 4;
    BOOST_TEST_EQ(atan2_inf_neg, ctrl_inf_neg);

    if(sizeof(Dec) >= 8U)
    {
        {
            std::stringstream strm;

            // Chris hex-dump:
            strm << "Chris hex-dump (atan2_inf_pos, ctrl_inf_pos): ("
                 << std::uppercase
                 << std::noshowbase
                 << std::hex
                 << std::setw(16)
                 << std::setfill('0')
                 << *reinterpret_cast<const std::uint64_t*>(&atan2_inf_pos)
                 << ", "
                 << *reinterpret_cast<const std::uint64_t*>(&ctrl_inf_pos)
                 << ")";

            std::cout << strm.str() << std::endl;
        }

        {
            std::stringstream strm;

            // Chris hex-dump:
            strm << "Chris hex-dump (atan2_inf_neg, ctrl_inf_neg): ("
                 << std::uppercase
                 << std::noshowbase
                 << std::hex
                 << std::setw(16)
                 << std::setfill('0')
                 << *reinterpret_cast<const std::uint64_t*>(&atan2_inf_neg)
                 << ", "
                 << *reinterpret_cast<const std::uint64_t*>(&ctrl_inf_neg)
                 << ")";

            std::cout << strm.str() << std::endl;
        }
    }

    BOOST_TEST_EQ(atan2(-Dec(1), Dec{0 * one(rng)}), -numbers::pi_v<Dec> / 2);
    BOOST_TEST_EQ(atan2(Dec(1), Dec{0 * one(rng)}), numbers::pi_v<Dec> / 2);
    BOOST_TEST_EQ(atan2(-Dec{one(rng)}, -std::numeric_limits<Dec>::infinity()), -numbers::pi_v<Dec>);
    BOOST_TEST_EQ(atan2(Dec{one(rng)}, -std::numeric_limits<Dec>::infinity()), numbers::pi_v<Dec>);
    BOOST_TEST_EQ(atan2(-Dec{one(rng)}, std::numeric_limits<Dec>::infinity()), -Dec{0 * one(rng)});
    BOOST_TEST_EQ(atan2(Dec{one(rng)}, std::numeric_limits<Dec>::infinity()), Dec{0 * one(rng)});
    BOOST_TEST_EQ(atan2(Dec(2), Dec(1)), atan(Dec(2)));
}

template <typename Dec = decimal32>
void spot_test(float val1, float val2)
{
    const auto ret_val {std::atan2(val1, val2)};

    Dec d_val1 {val1};
    Dec d_val2 {val2};
    const auto ret_dec {static_cast<float>(atan2(d_val1, d_val2))};

    const auto distance {std::fabs(boost::math::float_distance(ret_val, ret_dec))};
    if (!BOOST_TEST(distance < 1000))
    {
        // LCOV_EXCL_START
        std::cerr << "Val 1: " << val1
                  << "\nVal 2: " << val2
                  << "\nDec 1: " << Dec{val1}
                  << "\nDec 2: " << Dec {val2}
                  << "\nRet val: " << ret_val
                  << "\nRet dec: " << ret_dec
                  << "\nEps: " << distance << std::endl;
        // LCOV_EXCL_STOP
    }
}

int main()
{
    test<decimal32>();
    test<decimal64>();

    spot_test(2.36174F, 0.427896F);

    return boost::report_errors();
}
