// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include "mini_to_chars.hpp"

#include <boost/decimal.hpp>
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
void test_nonfinite_samequantum()
{
    constexpr Dec one(1);
    constexpr Dec two(2);
    BOOST_TEST(samequantum(std::numeric_limits<Dec>::infinity(), std::numeric_limits<Dec>::infinity()));
    BOOST_TEST(samequantum(std::numeric_limits<Dec>::quiet_NaN(), std::numeric_limits<Dec>::quiet_NaN()));
    BOOST_TEST(samequantum(std::numeric_limits<Dec>::signaling_NaN(), std::numeric_limits<Dec>::signaling_NaN()));
    BOOST_TEST(!samequantum(std::numeric_limits<Dec>::infinity(), std::numeric_limits<Dec>::quiet_NaN()));
    BOOST_TEST(!samequantum(one, std::numeric_limits<Dec>::infinity()));
    BOOST_TEST(!samequantum(one, std::numeric_limits<Dec>::quiet_NaN()));
    BOOST_TEST(!samequantum(one, std::numeric_limits<Dec>::signaling_NaN()));
    BOOST_TEST(!samequantum(std::numeric_limits<Dec>::infinity(), one));
    BOOST_TEST(!samequantum(std::numeric_limits<Dec>::quiet_NaN(), one));
    BOOST_TEST(!samequantum(std::numeric_limits<Dec>::signaling_NaN(), one));
    BOOST_TEST(samequantum(one, two));
}

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
            if (!BOOST_TEST(samequantum(val1, val2)))
            {
                std::cerr << "Val 1: " << val1
                          << "\nVal 2: " << val2 << std::endl;
            }
        }
        else
        {
            if (!BOOST_TEST(!samequantum(val1, val2)))
            {
                std::cerr << "Val 1: " << val1
                          << "\nVal 2: " << val2 << std::endl;
            }
        }
    }
}

template <typename Dec>
void test_quantexp()
{
    // Loop through every possible exponent
    for (auto i {std::numeric_limits<Dec>::min_exponent10}; i < std::numeric_limits<Dec>::max_exponent10; ++i)
    {
        const Dec val1 {1, i};

        if (static_cast<std::uint32_t>(i) + detail::bias > detail::d32_max_biased_exp)
        {
            if (!BOOST_TEST_EQ(quantexp(val1), detail::d32_max_biased_exp))
            {
                std::cerr << "Val: " << val1 << std::endl;
            }
        }
        else
        {
            if (!BOOST_TEST_EQ(quantexp(val1), i + detail::bias))
            {
                std::cerr << "Val: " << val1
                          << "\nExp 1: " << i << std::endl;
            }
        }
    }
}

template <typename Dec>
void test_nonfinite_quantexp()
{
    BOOST_TEST_EQ(quantexp(std::numeric_limits<Dec>::infinity()), INT_MIN);
    BOOST_TEST_EQ(quantexp(-std::numeric_limits<Dec>::infinity()), INT_MIN);
    BOOST_TEST_EQ(quantexp(std::numeric_limits<Dec>::quiet_NaN()), INT_MIN);
    BOOST_TEST_EQ(quantexp(-std::numeric_limits<Dec>::quiet_NaN()), INT_MIN);
    BOOST_TEST_EQ(quantexp(std::numeric_limits<Dec>::signaling_NaN()), INT_MIN);
    BOOST_TEST_EQ(quantexp(-std::numeric_limits<Dec>::signaling_NaN()), INT_MIN);
}

template <typename Dec>
void test_quantize()
{
    std::uniform_int_distribution<std::int64_t> sig(1'000'000, 9'999'999);
    std::uniform_int_distribution<std::int32_t> exp(std::numeric_limits<Dec>::min_exponent10 + 19,
                                                    std::numeric_limits<Dec>::max_exponent10 - 19);

    for (std::size_t i {}; i < N; ++i)
    {
        const auto sig1 {sig(rng)};
        const auto sig2 {sig(rng)};
        const auto exp1 {exp(rng)};
        const auto exp2 {exp(rng)};

        const Dec val1 {sig1, exp1};
        const Dec val2 {sig2, exp2};

        const Dec quantized_val {sig1, exp2};

        if (!BOOST_TEST_EQ(quantize(val1, val2), quantized_val))
        {
            std::cerr << "Val 1: " << val1
                      << "\nVal 2: " << val2
                      << "\nQuant: " << quantized_val << std::endl;
        }
    }
}

template <typename Dec>
void test_nonfinite_quantize()
{
    const Dec one{1};

    BOOST_TEST(isnan(quantize(std::numeric_limits<Dec>::quiet_NaN(), one)));
    BOOST_TEST(isnan(quantize(one, std::numeric_limits<Dec>::quiet_NaN())));
    BOOST_TEST(isnan(quantize(std::numeric_limits<Dec>::signaling_NaN(), one)));
    BOOST_TEST(isnan(quantize(one, std::numeric_limits<Dec>::signaling_NaN())));
    BOOST_TEST(isnan(quantize(std::numeric_limits<Dec>::infinity(), one)));
    BOOST_TEST(isnan(quantize(one, std::numeric_limits<Dec>::infinity())));
    BOOST_TEST(isinf(quantize(std::numeric_limits<Dec>::infinity(), std::numeric_limits<Dec>::infinity())));
}

int main()
{
    test_same_quantum<decimal32>();
    test_nonfinite_samequantum<decimal32>();
    test_quantexp<decimal32>();
    test_nonfinite_quantexp<decimal32>();
    test_quantize<decimal32>();
    test_nonfinite_quantize<decimal32>();

    return boost::report_errors();
}
