// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/core/lightweight_test.hpp>
#include <random>
#include <limits>

using namespace boost::decimal;

#if !defined(BOOST_DECIMAL_REDUCE_TEST_DEPTH)
static constexpr auto N = static_cast<std::size_t>(1024U); // Number of trials
#else
static constexpr auto N = static_cast<std::size_t>(1024U >> 4U); // Number of trials
#endif

// NOLINTNEXTLINE : Seed with a constant for repeatability
static std::mt19937_64 rng(42); // NOSONAR : Global rng is not const

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable: 4146)
#endif

template <typename Decimal1, typename Decimal2, typename T>
void random_mixed_addition(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    constexpr auto max_iter {(std::is_same<Decimal2, decimal128>::value || std::is_same<Decimal1, decimal128>::value) ? N / 4 : N};
    for (std::size_t i {}; i < max_iter; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const Decimal1 dec1 {val1};
        const Decimal2 dec2 {val2};

        const auto res = dec1 + dec2;
        const auto res_int = static_cast<T>(res);

        if (!BOOST_TEST_EQ(res_int, val1 + val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 + val2 << std::endl;
        }
    }

    BOOST_TEST(isinf(std::numeric_limits<Decimal1>::infinity() + Decimal2(dist(rng))));
    BOOST_TEST(isinf(Decimal2(dist(rng)) + std::numeric_limits<Decimal1>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<Decimal1>::quiet_NaN() + Decimal2(dist(rng))));
    BOOST_TEST(isnan(Decimal2(dist(rng)) + std::numeric_limits<Decimal1>::quiet_NaN()));
    BOOST_TEST(isnan(std::numeric_limits<Decimal1>::signaling_NaN() + Decimal2(dist(rng))));
    BOOST_TEST(isnan(Decimal2(dist(rng)) + std::numeric_limits<Decimal1>::signaling_NaN()));
}

template <typename Decimal1, typename Decimal2, typename T>
void random_mixed_subtraction(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    constexpr auto max_iter {(std::is_same<Decimal2, decimal128>::value || std::is_same<Decimal1, decimal128>::value) ? N / 4 : N};
    for (std::size_t i {}; i < max_iter; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const Decimal1 dec1 {val1};
        const Decimal2 dec2 {val2};

        const auto res = dec1 - dec2;
        const auto res_int = static_cast<T>(res);

        if (!BOOST_TEST_EQ(res_int, val1 - val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 - val2 << std::endl;
        }
    }

    BOOST_TEST(isinf(std::numeric_limits<Decimal1>::infinity() - Decimal2(dist(rng))));
    BOOST_TEST(isinf(Decimal2(dist(rng)) - std::numeric_limits<Decimal1>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<Decimal1>::quiet_NaN() - Decimal2(dist(rng))));
    BOOST_TEST(isnan(Decimal2(dist(rng)) - std::numeric_limits<Decimal1>::quiet_NaN()));
    BOOST_TEST(isnan(std::numeric_limits<Decimal1>::signaling_NaN() - Decimal2(dist(rng))));
    BOOST_TEST(isnan(Decimal2(dist(rng)) - std::numeric_limits<Decimal1>::signaling_NaN()));
}

template <typename Decimal1, typename Decimal2, typename T>
void random_mixed_multiplication(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    constexpr auto max_iter {(std::is_same<Decimal2, decimal128>::value || std::is_same<Decimal1, decimal128>::value) ? N / 4 : N};
    for (std::size_t i {}; i < max_iter; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const Decimal1 dec1 {val1};
        const Decimal2 dec2 {val2};

        const auto res = dec1 * dec2;
        const auto res_int = static_cast<T>(res);

        if (!BOOST_TEST_EQ(res_int, val1 * val2))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << val1 * val2 << std::endl;
        }
    }

    BOOST_TEST(isinf(std::numeric_limits<Decimal1>::infinity() * Decimal2(dist(rng))));
    BOOST_TEST(isinf(Decimal2(dist(rng)) * std::numeric_limits<Decimal1>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<Decimal1>::quiet_NaN() * Decimal2(dist(rng))));
    BOOST_TEST(isnan(Decimal2(dist(rng)) * std::numeric_limits<Decimal1>::quiet_NaN()));
    BOOST_TEST(isnan(std::numeric_limits<Decimal1>::signaling_NaN() * Decimal2(dist(rng))));
    BOOST_TEST(isnan(Decimal2(dist(rng)) * std::numeric_limits<Decimal1>::signaling_NaN()));
}

template <typename Decimal1, typename Decimal2, typename T>
void random_mixed_division(T lower, T upper)
{
    std::uniform_int_distribution<T> dist(lower, upper);

    constexpr auto max_iter {(std::is_same<Decimal2, decimal128>::value || std::is_same<Decimal1, decimal128>::value) ? N / 4 : N};
    for (std::size_t i {}; i < max_iter; ++i)
    {
        const T val1 {dist(rng)};
        const T val2 {dist(rng)};

        const Decimal1 dec1 {val1};
        const Decimal2 dec2 {val2};

        const auto res {dec1 / dec2};
        const decltype(res) res_int {static_cast<double>(val1) / static_cast<double>(val2)};

        if (isinf(res) && isinf(res_int))
        {
        }
        else if (!BOOST_TEST_EQ(static_cast<float>(res), static_cast<float>(res_int)))
        {
            std::cerr << "Val 1: " << val1
                      << "\nDec 1: " << dec1
                      << "\nVal 2: " << val2
                      << "\nDec 2: " << dec2
                      << "\nDec res: " << res
                      << "\nInt res: " << static_cast<double>(val1) / static_cast<double>(val2) << std::endl;
        }
    }

    BOOST_TEST(isinf(std::numeric_limits<Decimal1>::infinity() / Decimal2(dist(rng))));
    BOOST_TEST(!isinf(Decimal2(dist(rng)) / std::numeric_limits<Decimal1>::infinity()));
    BOOST_TEST(isnan(std::numeric_limits<Decimal1>::quiet_NaN() / Decimal2(dist(rng))));
    BOOST_TEST(isnan(Decimal2(dist(rng)) / std::numeric_limits<Decimal1>::quiet_NaN()));
    BOOST_TEST(isnan(std::numeric_limits<Decimal1>::signaling_NaN() / Decimal2(dist(rng))));
    BOOST_TEST(isnan(Decimal2(dist(rng)) / std::numeric_limits<Decimal1>::signaling_NaN()));
    BOOST_TEST(isinf(Decimal2(dist(rng)) / Decimal1(0)));
}

int main()
{
    random_mixed_addition<decimal32, decimal64>(0, 5'000'000);
    random_mixed_addition<decimal32, decimal64>(0LL, 5'000'000LL);
    random_mixed_addition<decimal64, decimal32>(0, 5'000'000);
    random_mixed_addition<decimal64, decimal32>(0LL, 5'000'000LL);

    random_mixed_addition<decimal32, decimal64>(-5'000'000, 0);
    random_mixed_addition<decimal32, decimal64>(-5'000'000LL, 0LL);
    random_mixed_addition<decimal64, decimal32>(-5'000'000, 0);
    random_mixed_addition<decimal64, decimal32>(-5'000'000LL, 0LL);

    random_mixed_addition<decimal32, decimal64>(-5'000'000, 5'000'000);
    random_mixed_addition<decimal32, decimal64>(-5'000'000LL, 5'000'000LL);
    random_mixed_addition<decimal64, decimal32>(-5'000'000, 5'000'000);
    random_mixed_addition<decimal64, decimal32>(-5'000'000LL, 5'000'000LL);

    random_mixed_subtraction<decimal32, decimal64>(0, 5'000'000);
    random_mixed_subtraction<decimal32, decimal64>(0LL, 5'000'000LL);
    random_mixed_subtraction<decimal64, decimal32>(0, 5'000'000);
    random_mixed_subtraction<decimal64, decimal32>(0LL, 5'000'000LL);

    random_mixed_subtraction<decimal32, decimal64>(-5'000'000, 0);
    random_mixed_subtraction<decimal32, decimal64>(-5'000'000LL, 0LL);
    random_mixed_subtraction<decimal64, decimal32>(-5'000'000, 0);
    random_mixed_subtraction<decimal64, decimal32>(-5'000'000LL, 0LL);

    random_mixed_subtraction<decimal32, decimal64>(-5'000'000, 5'000'000);
    random_mixed_subtraction<decimal32, decimal64>(-5'000'000LL, 5'000'000LL);
    random_mixed_subtraction<decimal64, decimal32>(-5'000'000, 5'000'000);
    random_mixed_subtraction<decimal64, decimal32>(-5'000'000LL, 5'000'000LL);

    random_mixed_multiplication<decimal32, decimal64>(0, 5'000);
    random_mixed_multiplication<decimal32, decimal64>(0LL, 5'000LL);
    random_mixed_multiplication<decimal64, decimal32>(0, 5'000);
    random_mixed_multiplication<decimal64, decimal32>(0LL, 5'000LL);

    random_mixed_multiplication<decimal32, decimal64>(-5'000, 0);
    random_mixed_multiplication<decimal32, decimal64>(-5'000LL, 0LL);
    random_mixed_multiplication<decimal64, decimal32>(-5'000, 0);
    random_mixed_multiplication<decimal64, decimal32>(-5'000LL, 0LL);

    random_mixed_multiplication<decimal32, decimal64>(-5'000, 5'000);
    random_mixed_multiplication<decimal32, decimal64>(-5'000LL, 5'000LL);
    random_mixed_multiplication<decimal64, decimal32>(-5'000, 5'000);
    random_mixed_multiplication<decimal64, decimal32>(-5'000LL, 5'000LL);

    random_mixed_division<decimal32, decimal64>(0, 5'000);
    random_mixed_division<decimal32, decimal64>(0LL, 5'000LL);
    random_mixed_division<decimal64, decimal32>(0, 5'000);
    random_mixed_division<decimal64, decimal32>(0LL, 5'000LL);

    random_mixed_division<decimal32, decimal64>(-5'000, 0);
    random_mixed_division<decimal32, decimal64>(-5'000LL, 0LL);
    random_mixed_division<decimal64, decimal32>(-5'000, 0);
    random_mixed_division<decimal64, decimal32>(-5'000LL, 0LL);

    random_mixed_division<decimal32, decimal64>(-5'000, 5'000);
    random_mixed_division<decimal32, decimal64>(-5'000LL, 5'000LL);
    random_mixed_division<decimal64, decimal32>(-5'000, 5'000);
    random_mixed_division<decimal64, decimal32>(-5'000LL, 5'000LL);

    random_mixed_addition<decimal32, decimal128>(0, 5'000'000);
    random_mixed_addition<decimal32, decimal128>(0LL, 5'000'000LL);
    random_mixed_addition<decimal128, decimal32>(0, 5'000'000);
    random_mixed_addition<decimal128, decimal32>(0LL, 5'000'000LL);

    random_mixed_addition<decimal32, decimal128>(-5'000'000, 0);
    random_mixed_addition<decimal32, decimal128>(-5'000'000LL, 0LL);
    random_mixed_addition<decimal128, decimal32>(-5'000'000, 0);
    random_mixed_addition<decimal128, decimal32>(-5'000'000LL, 0LL);

    random_mixed_addition<decimal32, decimal128>(-5'000'000, 5'000'000);
    random_mixed_addition<decimal32, decimal128>(-5'000'000LL, 5'000'000LL);
    random_mixed_addition<decimal128, decimal32>(-5'000'000, 5'000'000);
    random_mixed_addition<decimal128, decimal32>(-5'000'000LL, 5'000'000LL);

    random_mixed_subtraction<decimal32, decimal128>(0, 5'000'000);
    random_mixed_subtraction<decimal32, decimal128>(0LL, 5'000'000LL);
    random_mixed_subtraction<decimal128, decimal32>(0, 5'000'000);
    random_mixed_subtraction<decimal128, decimal32>(0LL, 5'000'000LL);

    random_mixed_subtraction<decimal32, decimal128>(-5'000'000, 0);
    random_mixed_subtraction<decimal32, decimal128>(-5'000'000LL, 0LL);
    random_mixed_subtraction<decimal128, decimal32>(-5'000'000, 0);
    random_mixed_subtraction<decimal128, decimal32>(-5'000'000LL, 0LL);

    random_mixed_subtraction<decimal32, decimal128>(-5'000'000, 5'000'000);
    random_mixed_subtraction<decimal32, decimal128>(-5'000'000LL, 5'000'000LL);
    random_mixed_subtraction<decimal128, decimal32>(-5'000'000, 5'000'000);
    random_mixed_subtraction<decimal128, decimal32>(-5'000'000LL, 5'000'000LL);

    random_mixed_multiplication<decimal32, decimal128>(0, 5'000);
    random_mixed_multiplication<decimal32, decimal128>(0LL, 5'000LL);
    random_mixed_multiplication<decimal128, decimal32>(0, 5'000);
    random_mixed_multiplication<decimal128, decimal32>(0LL, 5'000LL);

    random_mixed_multiplication<decimal32, decimal128>(-5'000, 0);
    random_mixed_multiplication<decimal32, decimal128>(-5'000LL, 0LL);
    random_mixed_multiplication<decimal128, decimal32>(-5'000, 0);
    random_mixed_multiplication<decimal128, decimal32>(-5'000LL, 0LL);

    random_mixed_multiplication<decimal32, decimal128>(-5'000, 5'000);
    random_mixed_multiplication<decimal32, decimal128>(-5'000LL, 5'000LL);
    random_mixed_multiplication<decimal128, decimal32>(-5'000, 5'000);
    random_mixed_multiplication<decimal128, decimal32>(-5'000LL, 5'000LL);

    random_mixed_division<decimal32, decimal128>(0, 5'000);
    random_mixed_division<decimal32, decimal128>(0LL, 5'000LL);
    random_mixed_division<decimal128, decimal32>(0, 5'000);
    random_mixed_division<decimal128, decimal32>(0LL, 5'000LL);

    random_mixed_division<decimal32, decimal128>(-5'000, 0);
    random_mixed_division<decimal32, decimal128>(-5'000LL, 0LL);
    random_mixed_division<decimal128, decimal32>(-5'000, 0);
    random_mixed_division<decimal128, decimal32>(-5'000LL, 0LL);

    random_mixed_division<decimal32, decimal128>(-5'000, 5'000);
    random_mixed_division<decimal32, decimal128>(-5'000LL, 5'000LL);
    random_mixed_division<decimal128, decimal32>(-5'000, 5'000);
    random_mixed_division<decimal128, decimal32>(-5'000LL, 5'000LL);
    
    random_mixed_addition<decimal64, decimal128>(0, 5'000'000);
    random_mixed_addition<decimal64, decimal128>(0LL, 5'000'000LL);
    random_mixed_addition<decimal128, decimal64>(0, 5'000'000);
    random_mixed_addition<decimal128, decimal64>(0LL, 5'000'000LL);

    random_mixed_addition<decimal64, decimal128>(-5'000'000, 0);
    random_mixed_addition<decimal64, decimal128>(-5'000'000LL, 0LL);
    random_mixed_addition<decimal128, decimal64>(-5'000'000, 0);
    random_mixed_addition<decimal128, decimal64>(-5'000'000LL, 0LL);

    random_mixed_addition<decimal64, decimal128>(-5'000'000, 5'000'000);
    random_mixed_addition<decimal64, decimal128>(-5'000'000LL, 5'000'000LL);
    random_mixed_addition<decimal128, decimal64>(-5'000'000, 5'000'000);
    random_mixed_addition<decimal128, decimal64>(-5'000'000LL, 5'000'000LL);

    random_mixed_subtraction<decimal64, decimal128>(0, 5'000'000);
    random_mixed_subtraction<decimal64, decimal128>(0LL, 5'000'000LL);
    random_mixed_subtraction<decimal128, decimal64>(0, 5'000'000);
    random_mixed_subtraction<decimal128, decimal64>(0LL, 5'000'000LL);

    random_mixed_subtraction<decimal64, decimal128>(-5'000'000, 0);
    random_mixed_subtraction<decimal64, decimal128>(-5'000'000LL, 0LL);
    random_mixed_subtraction<decimal128, decimal64>(-5'000'000, 0);
    random_mixed_subtraction<decimal128, decimal64>(-5'000'000LL, 0LL);

    random_mixed_subtraction<decimal64, decimal128>(-5'000'000, 5'000'000);
    random_mixed_subtraction<decimal64, decimal128>(-5'000'000LL, 5'000'000LL);
    random_mixed_subtraction<decimal128, decimal64>(-5'000'000, 5'000'000);
    random_mixed_subtraction<decimal128, decimal64>(-5'000'000LL, 5'000'000LL);

    random_mixed_multiplication<decimal64, decimal128>(0, 5'000);
    random_mixed_multiplication<decimal64, decimal128>(0LL, 5'000LL);
    random_mixed_multiplication<decimal128, decimal64>(0, 5'000);
    random_mixed_multiplication<decimal128, decimal64>(0LL, 5'000LL);

    random_mixed_multiplication<decimal64, decimal128>(-5'000, 0);
    random_mixed_multiplication<decimal64, decimal128>(-5'000LL, 0LL);
    random_mixed_multiplication<decimal128, decimal64>(-5'000, 0);
    random_mixed_multiplication<decimal128, decimal64>(-5'000LL, 0LL);

    random_mixed_multiplication<decimal64, decimal128>(-5'000, 5'000);
    random_mixed_multiplication<decimal64, decimal128>(-5'000LL, 5'000LL);
    random_mixed_multiplication<decimal128, decimal64>(-5'000, 5'000);
    random_mixed_multiplication<decimal128, decimal64>(-5'000LL, 5'000LL);

    random_mixed_division<decimal64, decimal128>(0, 5'000);
    random_mixed_division<decimal64, decimal128>(0LL, 5'000LL);
    random_mixed_division<decimal128, decimal64>(0, 5'000);
    random_mixed_division<decimal128, decimal64>(0LL, 5'000LL);

    random_mixed_division<decimal64, decimal128>(-5'000, 0);
    random_mixed_division<decimal64, decimal128>(-5'000LL, 0LL);
    random_mixed_division<decimal128, decimal64>(-5'000, 0);
    random_mixed_division<decimal128, decimal64>(-5'000LL, 0LL);

    random_mixed_division<decimal64, decimal128>(-5'000, 5'000);
    random_mixed_division<decimal64, decimal128>(-5'000LL, 5'000LL);
    random_mixed_division<decimal128, decimal64>(-5'000, 5'000);
    random_mixed_division<decimal128, decimal64>(-5'000LL, 5'000LL);

    return boost::report_errors();
}

#ifdef _MSC_VER
#  pragma warning(pop)
#endif
