// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <random>
#include <limits>
#include <climits>

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wfloat-equal"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

#include <boost/core/lightweight_test.hpp>

using namespace boost::decimal;

#if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
static constexpr auto N = static_cast<std::size_t>(128U); // Number of trials
#else
static constexpr auto N = static_cast<std::size_t>(8U); // Number of trials
#endif

static std::mt19937_64 rng(42);

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4146)
#endif

#if defined(__GNUC__) && __GNUC__ >= 8
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif


template <typename T>
void random_addition(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal128_t dec1 {val1};
        const decimal128_t dec2 {val2};

        const decimal128_t res = dec1 + dec2;
        const auto res_int = static_cast<T>(res);

        if (!BOOST_TEST_EQ(res_int, val1 + val2))
        {
            // LCOV_EXCL_START
            std::cerr << std::setprecision(std::numeric_limits<decimal128_t>::digits10)
                      << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 + val2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isinf(std::numeric_limits<decimal128_t>::infinity() + decimal128_t{0,0}));
    BOOST_TEST(isinf(decimal128_t{0,0} + std::numeric_limits<decimal128_t>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<decimal128_t>::quiet_NaN() + decimal128_t{0,0}));
    BOOST_TEST(isnan(decimal128_t{0,0} + std::numeric_limits<decimal128_t>::quiet_NaN()));

    // Cohorts
    #if !(defined(__GNUC__) && __GNUC__ == 6)
    BOOST_TEST_EQ(decimal128_t(4,0) + decimal128_t(40, -1), decimal128_t(8,0));
    BOOST_TEST_EQ(decimal128_t(4,0) + decimal128_t(400, -2), decimal128_t(8,0));
    BOOST_TEST_EQ(decimal128_t(4,0) + decimal128_t(4000, -3), decimal128_t(8,0));
    BOOST_TEST_EQ(decimal128_t(40000000000, -10) + decimal128_t(4, 0), decimal128_t(8,0));
    #endif
}

template <typename T>
void random_mixed_addition(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal128_t dec1 {val1};
        const T trunc_val_2 {static_cast<T>(decimal128_t(val2))};

        const decimal128_t res = dec1 + trunc_val_2;
        const auto res_int = static_cast<T>(res);

        if (!BOOST_TEST_EQ(res_int, val1 + val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << trunc_val_2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 + val2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isinf(std::numeric_limits<decimal128_t>::infinity() + dist(rng)));
    BOOST_TEST(isinf(dist(rng) + std::numeric_limits<decimal128_t>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<decimal128_t>::quiet_NaN() + dist(rng)));
    BOOST_TEST(isnan(dist(rng) + std::numeric_limits<decimal128_t>::quiet_NaN()));
}

template <typename T>
void random_subtraction(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal128_t dec1 {val1};
        const decimal128_t dec2 {val2};

        const decimal128_t res = dec1 - dec2;
        const auto res_int = static_cast<T>(res);

        if (!BOOST_TEST_EQ(res_int, val1 - val2))
        {
            // LCOV_EXCL_START
            std::cerr << std::setprecision(std::numeric_limits<decimal128_t>::digits10)
                      << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 - val2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isinf(std::numeric_limits<decimal128_t>::infinity() - decimal128_t{0,0}));
    BOOST_TEST(isinf(decimal128_t{0,0} - std::numeric_limits<decimal128_t>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<decimal128_t>::quiet_NaN() - decimal128_t{0,0}));
    BOOST_TEST(isnan(decimal128_t{0,0} - std::numeric_limits<decimal128_t>::quiet_NaN()));
}

template <typename T>
void random_mixed_subtraction(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal128_t dec1 {val1};
        const T trunc_val_2 {static_cast<T>(decimal128_t(val2))};

        const decimal128_t res = dec1 - trunc_val_2;
        const auto res_int = static_cast<T>(res);

        if (!BOOST_TEST_EQ(res_int, val1 - val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << trunc_val_2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 - val2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const T trunc_val_1 {static_cast<T>(decimal128_t(val1))};
        const decimal128_t dec2 {val2};

        const decimal128_t res = trunc_val_1 - dec2;
        const auto res_int = static_cast<T>(res);

        if (!BOOST_TEST_EQ(res_int, val1 - val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << trunc_val_1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 - val2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isinf(std::numeric_limits<decimal128_t>::infinity() - dist(rng)));
    BOOST_TEST(isinf(dist(rng) - std::numeric_limits<decimal128_t>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<decimal128_t>::quiet_NaN() - dist(rng)));
    BOOST_TEST(isnan(dist(rng) - std::numeric_limits<decimal128_t>::quiet_NaN()));
}

template <typename T>
void spot_check_sub(T lhs, T rhs)
{
    const decimal128_t dec1 {lhs};
    const decimal128_t dec2 {rhs};
    const decimal128_t res {dec1 - dec2};
    const auto res_int {static_cast<T>(res)};

    if (!BOOST_TEST_EQ(res_int, lhs - rhs))
    {
        // LCOV_EXCL_START
        std::cerr << "Val 1: " << lhs
                  << "\nDec 1: " << dec1
                  << "\nVal 2: " << rhs
                  << "\nDec 2: " << dec2
                  << "\nDec res: " << res
                  << "\nInt res: " << lhs - rhs << std::endl;
        // LCOV_EXCL_STOP
    }
}

template <typename T>
void random_multiplication(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal128_t dec1 {val1};
        const decimal128_t dec2 {val2};

        const decimal128_t res = dec1 * dec2;
        const auto res_int = static_cast<T>(res);

        if (val1 * val2 == 0)
        {
            // Integers don't have signed 0 but decimal does
            continue; // LCOV_EXCL_LINE
        }

        if (!BOOST_TEST_EQ(res_int, val1 * val2))
        {
            // LCOV_EXCL_START
            std::cerr << std::setprecision(std::numeric_limits<decimal128_t>::digits10)
                      << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nDec int: " << res_int
                      << "\nInt res: " << val1 * val2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isinf(std::numeric_limits<decimal128_t>::infinity() * decimal128_t(dist(rng))));
    BOOST_TEST(isinf(decimal128_t(dist(rng)) * std::numeric_limits<decimal128_t>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<decimal128_t>::quiet_NaN() * decimal128_t(dist(rng))));
    BOOST_TEST(isnan(decimal128_t(dist(rng)) * std::numeric_limits<decimal128_t>::quiet_NaN()));
}

template <typename T>
void random_mixed_multiplication(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal128_t dec1 {val1};
        const T dec2 {static_cast<T>(decimal128_t(val2))};

        const decimal128_t res {dec1 * dec2};
        const T res_int {static_cast<T>(res)};

        if (val1 * val2 == 0)
        {
            // Integers don't have signed 0 but decimal does
            continue;
        }

        if (!BOOST_TEST_EQ(res_int, val1 * val2))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 * val2 << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isinf(std::numeric_limits<decimal128_t>::infinity() * dist(rng)));
    BOOST_TEST(isinf(dist(rng) * std::numeric_limits<decimal128_t>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<decimal128_t>::quiet_NaN() * dist(rng)));
    BOOST_TEST(isnan(dist(rng) * std::numeric_limits<decimal128_t>::quiet_NaN()));
}

template <typename T>
void random_division(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal128_t dec1 {val1};
        const decimal128_t dec2 {val2};

        const decimal128_t res {dec1 / dec2};
        const decimal128_t res_int {static_cast<double>(val1) / static_cast<double>(val2)};

        if (isinf(res) && isinf(res_int))
        {
        }
        else if (!BOOST_TEST_EQ(static_cast<float>(res), static_cast<float>(res_int)))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << static_cast<double>(val1) / static_cast<double>(val2) << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    BOOST_TEST(isinf(std::numeric_limits<decimal128_t>::infinity() / decimal128_t(dist(rng))));
    BOOST_TEST(!isinf(decimal128_t(dist(rng)) / std::numeric_limits<decimal128_t>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<decimal128_t>::quiet_NaN() / decimal128_t(dist(rng))));
    BOOST_TEST(isnan(decimal128_t(dist(rng)) / std::numeric_limits<decimal128_t>::quiet_NaN()));
    BOOST_TEST(isinf(decimal128_t(dist(rng)) / decimal128_t(0)));
}

template <typename T>
void random_mixed_division(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const decimal128_t dec1 {val1};
        const T dec2 {static_cast<T>(decimal128_t(val2))};

        const decimal128_t res {dec1 / dec2};
        const decimal128_t res_int {static_cast<double>(val1) / static_cast<double>(val2)};

        if (isinf(res) && isinf(res_int))
        {
        }
        else if (!BOOST_TEST_EQ(static_cast<float>(res), static_cast<float>(res_int)))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << static_cast<double>(val1) / static_cast<double>(val2) << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    for (std::size_t i {}; i < N; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const T dec1 {static_cast<T>(decimal128_t(val1))};
        const decimal128_t dec2 {val2};

        const decimal128_t res {dec1 / dec2};
        const decimal128_t res_int {static_cast<double>(val1) / static_cast<double>(val2)};

        if (isinf(res) && isinf(res_int))
        {
        }
        else if (!BOOST_TEST(abs(res - res_int) < decimal128_t(1, -1)))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << static_cast<double>(val1) / static_cast<double>(val2) << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    // Edge cases
    const decimal128_t val1 {dist(rng)};
    const decimal128_t zero {0, 0};
    BOOST_TEST(isnan(std::numeric_limits<decimal128_t>::quiet_NaN() / dist(rng)));
    BOOST_TEST(isinf(std::numeric_limits<decimal128_t>::infinity() / dist(rng)));
    BOOST_TEST(isnan(dist(rng) / std::numeric_limits<decimal128_t>::quiet_NaN()));
    BOOST_TEST_EQ(abs(dist(rng) / std::numeric_limits<decimal128_t>::infinity()), zero);
    BOOST_TEST(isinf(decimal128_t(dist(rng)) / 0));
    BOOST_TEST(isinf(val1 / zero));
}

void random_and()
{
    std::uniform_int_distribution<std::uint64_t> dist(0, 9'999'999'999'999'999);

    for (std::size_t i {}; i < N; ++i)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};

        decimal128_t dec1 {};
        std::memcpy(&dec1, &val1, sizeof(std::uint64_t));
        decimal128_t dec2 {};
        std::memcpy(&dec2, &val2, sizeof(std::uint64_t));

        const decimal128_t res {dec1 & dec2};
        std::uint64_t dec_int {};
        std::memcpy(&dec_int, &res, sizeof(std::uint64_t));
        const auto res_int {val1 & val2};

        if (!BOOST_TEST_EQ(dec_int, res_int))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << res_int << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

void random_mixed_and()
{
    std::uniform_int_distribution<std::uint64_t> dist(0, 9'999'999'999'999'999);

    for (std::size_t i {}; i < N; ++i)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};

        decimal128_t dec1 {};
        std::memcpy(&dec1, &val1, sizeof(std::uint64_t));

        const decimal128_t res {dec1 & val2};
        std::uint64_t dec_int {};
        std::memcpy(&dec_int, &res, sizeof(std::uint64_t));
        const auto res_int {val1 & val2};

        if (!BOOST_TEST_EQ(dec_int, res_int))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec res: " << res
                      << "\nInt res: " << res_int << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    for (std::size_t i {}; i < N; ++i)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};

        decimal128_t dec2 {};
        std::memcpy(&dec2, &val2, sizeof(std::uint64_t));

        const decimal128_t res {val1 & dec2};
        std::uint64_t dec_int {};
        std::memcpy(&dec_int, &res, sizeof(std::uint64_t));
        const auto res_int {val1 & val2};

        if (!BOOST_TEST_EQ(dec_int, res_int))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << res_int << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

void random_or()
{
    std::uniform_int_distribution<std::uint64_t> dist(0, 9'999'999'999'999'999);

    for (std::size_t i {}; i < N; ++i)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};

        decimal128_t dec1 {};
        std::memcpy(&dec1, &val1, sizeof(std::uint64_t));
        decimal128_t dec2 {};
        std::memcpy(&dec2, &val2, sizeof(std::uint64_t));

        const decimal128_t res {dec1 | dec2};
        std::uint64_t dec_int {};
        std::memcpy(&dec_int, &res, sizeof(std::uint64_t));
        const auto res_int {val1 | val2};

        if (!BOOST_TEST_EQ(dec_int, res_int))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << res_int << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

void random_mixed_or()
{
    std::uniform_int_distribution<std::uint64_t> dist(0, 9'999'999'999'999'999);

    for (std::size_t i {}; i < N; ++i)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};

        decimal128_t dec1 {};
        std::memcpy(&dec1, &val1, sizeof(std::uint64_t));

        const decimal128_t res {dec1 | val2};
        std::uint64_t dec_int {};
        std::memcpy(&dec_int, &res, sizeof(std::uint64_t));
        const auto res_int {val1 | val2};

        if (!BOOST_TEST_EQ(dec_int, res_int))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec res: " << res
                      << "\nInt res: " << res_int << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    for (std::size_t i {}; i < N; ++i)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};

        decimal128_t dec2 {};
        std::memcpy(&dec2, &val2, sizeof(std::uint64_t));

        const decimal128_t res {val1 | dec2};
        std::uint64_t dec_int {};
        std::memcpy(&dec_int, &res, sizeof(std::uint64_t));
        const auto res_int {val1 | val2};

        if (!BOOST_TEST_EQ(dec_int, res_int))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << res_int << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

void random_xor()
{
    std::uniform_int_distribution<std::uint64_t> dist(0, 9'999'999'999'999'999);

    for (std::size_t i {}; i < N; ++i)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};

        decimal128_t dec1 {};
        std::memcpy(&dec1, &val1, sizeof(std::uint64_t));
        decimal128_t dec2 {};
        std::memcpy(&dec2, &val2, sizeof(std::uint64_t));

        const decimal128_t res {dec1 ^ dec2};
        std::uint64_t dec_int {};
        std::memcpy(&dec_int, &res, sizeof(std::uint64_t));
        const auto res_int {val1 ^ val2};

        if (!BOOST_TEST_EQ(dec_int, res_int))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << res_int << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

void random_mixed_xor()
{
    std::uniform_int_distribution<std::uint64_t> dist(0, 9'999'999'999'999'999);

    for (std::size_t i {}; i < N; ++i)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};

        decimal128_t dec1 {};
        std::memcpy(&dec1, &val1, sizeof(std::uint64_t));

        const decimal128_t res {dec1 ^ val2};
        std::uint64_t dec_int {};
        std::memcpy(&dec_int, &res, sizeof(std::uint64_t));
        const auto res_int {val1 ^ val2};

        if (!BOOST_TEST_EQ(dec_int, res_int))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec res: " << res
                      << "\nInt res: " << res_int << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    for (std::size_t i {}; i < N; ++i)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};

        decimal128_t dec2 {};
        std::memcpy(&dec2, &val2, sizeof(std::uint64_t));

        const decimal128_t res {val1 ^ dec2};
        std::uint64_t dec_int {};
        std::memcpy(&dec_int, &res, sizeof(std::uint64_t));
        const auto res_int {val1 ^ val2};

        if (!BOOST_TEST_EQ(dec_int, res_int))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << res_int << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

void random_left_shift()
{
    std::uniform_int_distribution<std::uint64_t> dist(0, 10);

    for (std::size_t i {}; i < N; ++i)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};

        decimal128_t dec1 {};
        std::memcpy(&dec1, &val1, sizeof(std::uint64_t));
        decimal128_t dec2 {};
        std::memcpy(&dec2, &val2, sizeof(std::uint64_t));

        const decimal128_t res {dec1 << dec2};
        std::uint64_t dec_int {};
        std::memcpy(&dec_int, &res, sizeof(std::uint64_t));
        const auto res_int {val1 << val2};

        if (!BOOST_TEST_EQ(dec_int, res_int))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << res_int << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

void random_mixed_left_shift()
{
    std::uniform_int_distribution<std::uint64_t> dist(0, 10);

    for (std::size_t i {}; i < N; ++i)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};

        decimal128_t dec1 {};
        std::memcpy(&dec1, &val1, sizeof(std::uint64_t));

        const decimal128_t res {dec1 << val2};
        std::uint64_t dec_int {};
        std::memcpy(&dec_int, &res, sizeof(std::uint64_t));
        const auto res_int {val1 << val2};

        if (!BOOST_TEST_EQ(dec_int, res_int))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec res: " << res
                      << "\nInt res: " << res_int << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    for (std::size_t i {}; i < N; ++i)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};

        decimal128_t dec2 {};
        std::memcpy(&dec2, &val2, sizeof(std::uint64_t));

        const decimal128_t res {val1 << dec2};
        std::uint64_t dec_int {};
        std::memcpy(&dec_int, &res, sizeof(std::uint64_t));
        const auto res_int {val1 << val2};

        if (!BOOST_TEST_EQ(dec_int, res_int))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << res_int << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

void random_right_shift()
{
    std::uniform_int_distribution<std::uint64_t> dist(0, 10);

    for (std::size_t i {}; i < N; ++i)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};

        decimal128_t dec1 {};
        std::memcpy(&dec1, &val1, sizeof(std::uint64_t));
        decimal128_t dec2 {};
        std::memcpy(&dec2, &val2, sizeof(std::uint64_t));

        const decimal128_t res {dec1 >> dec2};
        std::uint64_t dec_int {};
        std::memcpy(&dec_int, &res, sizeof(std::uint64_t));
        const auto res_int {val1 >> val2};

        if (!BOOST_TEST_EQ(dec_int, res_int))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << res_int << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

void random_mixed_right_shift()
{
    std::uniform_int_distribution<std::uint64_t> dist(0, 10);

    for (std::size_t i {}; i < N; ++i)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};

        decimal128_t dec1 {};
        std::memcpy(&dec1, &val1, sizeof(std::uint64_t));

        const decimal128_t res {dec1 >> val2};
        std::uint64_t dec_int {};
        std::memcpy(&dec_int, &res, sizeof(std::uint64_t));
        const auto res_int {val1 >> val2};

        if (!BOOST_TEST_EQ(dec_int, res_int))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec res: " << res
                      << "\nInt res: " << res_int << std::endl;
            // LCOV_EXCL_STOP
        }
    }

    for (std::size_t i {}; i < N; ++i)
    {
        const auto val1 {dist(rng)};
        const auto val2 {dist(rng)};

        decimal128_t dec2 {};
        std::memcpy(&dec2, &val2, sizeof(std::uint64_t));

        const decimal128_t res {val1 >> dec2};
        std::uint64_t dec_int {};
        std::memcpy(&dec_int, &res, sizeof(std::uint64_t));
        const auto res_int {val1 >> val2};

        if (!BOOST_TEST_EQ(dec_int, res_int))
        {
            // LCOV_EXCL_START
            std::cerr << "Val 1: " << val1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << res_int << std::endl;
            // LCOV_EXCL_STOP
        }
    }
}

template <typename T>
void test_spot_sub(T lhs, T rhs)
{
    decimal128_t dec_lhs{lhs};
    decimal128_t dec_rhs{rhs};
    const auto dec_res{dec_lhs - dec_rhs};
    const T dec_to_int_res{static_cast<T>(dec_res)};
    BOOST_TEST_EQ(dec_to_int_res, (lhs - rhs));
}

int main()
{
    // Values that won't exceed the range of the significand
    // Only positive values
    random_addition(0, 5'000'000);
    random_addition(0LL, 4'000'000'000'000LL);
    random_mixed_addition(0, 5'000'000);
    random_mixed_addition(0LL, 4'000'000'000'000LL);

    // Only two negative values
    random_addition(-5'000'000, 0);
    random_addition(-4'000'000'000'000LL, 0LL);
    random_mixed_addition(-5'000'000, 0);
    random_mixed_addition(-4'000'000'000'000LL, 0LL);

    // Mixed values
    random_addition(-5'000'000, 5'000'000);
    random_addition(-5'000'000'000'000LL, 5'000'000'000'000LL);
    random_mixed_addition(-5'000'000, 5'000'000);
    random_mixed_addition(-5'000'000'000'000LL, 5'000'000'000'000LL);

    // Subtraction
    random_subtraction(0, 5'000'000);
    random_subtraction(0LL, 4'000'000'000'000LL);
    random_mixed_subtraction(0, 5'000'000);
    random_mixed_subtraction(0LL, 4'000'000'000'000LL);

    // Only two negative values
    random_subtraction(-5'000'000, 0);
    random_subtraction(-4'000'000'000'000LL, 0LL);
    random_mixed_subtraction(-5'000'000, 0);
    random_mixed_subtraction(-4'000'000'000'000LL, 0LL);

    // Mixed values
    random_subtraction(-5'000'000, 5'000'000);
    random_subtraction(-4'000'000'000'000LL, 4'000'000'000'000LL);
    random_mixed_subtraction(-5'000'000, 5'000'000);
    random_mixed_subtraction(-4'000'000'000'000LL, 4'000'000'000'000LL);

    // Multiplication
    const auto sqrt_int_max = static_cast<int>(std::sqrt(static_cast<double>((std::numeric_limits<int>::max)())));

    // Positive
    random_multiplication(0, 5'000);
    random_multiplication(0LL, 5'000LL);
    random_multiplication(0, sqrt_int_max);
    random_mixed_multiplication(0, 5'000);
    random_mixed_multiplication(0LL, 5'000LL);
    random_mixed_multiplication(0, sqrt_int_max);

    // Negative
    random_multiplication(-5'000, 0);
    random_multiplication(-5'000LL, 0LL);
    random_multiplication(-sqrt_int_max, 0);
    random_mixed_multiplication(-5'000, 0);
    random_mixed_multiplication(-5'000LL, 0LL);
    random_mixed_multiplication(-sqrt_int_max, 0);

    // Mixed
    random_multiplication(-5'000, 5'000);
    random_multiplication(-5'000LL, 5'000LL);
    random_multiplication(-sqrt_int_max, sqrt_int_max);
    random_mixed_multiplication(-5'000, 5'000);
    random_mixed_multiplication(-5'000LL, 5'000LL);
    random_mixed_multiplication(-sqrt_int_max, sqrt_int_max);

    // Division

    // Positive
    random_division(0, 5'000);
    random_division(0LL, 5'000LL);
    random_division(0, sqrt_int_max);
    random_mixed_division(0, 5'000);
    random_mixed_division(0LL, 5'000LL);
    random_mixed_division(0, sqrt_int_max);

    // Negative
    random_division(-5'000, 0);
    random_division(-5'000LL, 0LL);
    random_division(-sqrt_int_max, 0);
    random_mixed_division(-5'000, 0);
    random_mixed_division(-5'000LL, 0LL);
    random_mixed_division(-sqrt_int_max, 0);

    // Mixed
    random_division(-5'000, 5'000);
    random_division(-5'000LL, 5'000LL);
    random_division(-sqrt_int_max, sqrt_int_max);
    random_mixed_division(-5'000, 5'000);
    random_mixed_division(-5'000LL, 5'000LL);
    random_mixed_division(-sqrt_int_max, sqrt_int_max);

    // Bitwise operators
    #if BOOST_DECIMAL_ENDIAN_LITTLE_BYTE
    random_and();
    random_mixed_and();
    random_or();
    random_mixed_or();
    random_xor();
    random_mixed_xor();

    random_left_shift();
    random_mixed_left_shift();
    random_right_shift();
    random_mixed_right_shift();
    #endif

    test_spot_sub(-813150, -905406);

    return boost::report_errors();
}

#ifdef _MSC_VER
#  pragma warning(pop)
#endif

#if defined(__GNUC__) && __GNUC__ >= 8
#  pragma GCC diagnostic pop
#endif

