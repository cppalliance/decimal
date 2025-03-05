// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#define BOOST_DECIMAL_BENCHMARK_U128

#include <iostream>

#ifdef BOOST_DECIMAL_BENCHMARK_U128

#include <boost/decimal/detail/u128.hpp>
#include <boost/decimal/detail/emulated128.hpp>
#include <chrono>
#include <random>
#include <vector>
#include <type_traits>
#include <iomanip>
#include <string>
#include <cmath>
#include <cstring>

constexpr unsigned N = 20'000'000;
constexpr unsigned K = 5;

using namespace boost::decimal;
using namespace std::chrono_literals;

#ifdef __clang__
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wfloat-equal"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#  define BOOST_DECIMAL_NO_INLINE __attribute__ ((__noinline__))
#elif defined(_MSC_VER)
#  define BOOST_DECIMAL_NO_INLINE __declspec(noinline)
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wfloat-equal"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wstringop-overread"
#  define BOOST_DECIMAL_NO_INLINE __attribute__ ((__noinline__))
#endif

template <bool mul, typename T>
std::vector<T> generate_random_vector(std::size_t size = N, unsigned seed = 42U)
{
    if (seed == 0)
    {
        std::random_device rd;
        seed = rd();
    }

    std::mt19937_64 gen(seed);
    std::uniform_int_distribution<std::uint64_t> dist(UINT64_C(0), UINT64_MAX);

    std::vector<T> result(size);
    for (std::size_t i = 0; i < size; ++i)
    {
        BOOST_DECIMAL_IF_CONSTEXPR (mul)
        {
            result[i] = T{dist(gen)};
        }
        else
        {
            BOOST_DECIMAL_IF_CONSTEXPR (std::is_same<T, unsigned __int128>::value)
            {
                result[i] = T{boost::decimal::detail::u128{dist(gen), dist(gen)}};
            }
            else
            {
                result[i] = T{dist(gen), dist(gen)};
            }
        }
    }

    return result;
}

template <typename T>
BOOST_DECIMAL_NO_INLINE void test_comparisons(const std::vector<T>& data_vec, const char* label)
{
    const auto t1 = std::chrono::steady_clock::now();
    std::size_t s = 0; // discard variable

    for (std::size_t k {}; k < K; ++k)
    {
        for (std::size_t i {}; i < data_vec.size() - 1U; ++i)
        {
            const auto val1 = data_vec[i];
            const auto val2 = data_vec[i + 1];
            s += static_cast<std::size_t>(val1 > val2);
            s += static_cast<std::size_t>(val1 >= val2);
            s += static_cast<std::size_t>(val1 < val2);
            s += static_cast<std::size_t>(val1 <= val2);
            s += static_cast<std::size_t>(val1 == val2);
            s += static_cast<std::size_t>(val1 != val2);
        }
    }

    const auto t2 = std::chrono::steady_clock::now();

    std::cout << "comparisons<" << std::left << std::setw(11) << label << ">: " << std::setw( 10 ) << ( t2 - t1 ) / 1us << " us (s=" << s << ")\n";
}

template <typename T, typename Func>
BOOST_DECIMAL_NO_INLINE void test_two_element_operation(const std::vector<T>& data_vec, Func op, const char* operation, const char* type)
{
    const auto t1 = std::chrono::steady_clock::now();
    std::size_t s = 0; // discard variable

    for (std::size_t k {}; k < K; ++k)
    {
        for (std::size_t i {}; i < data_vec.size() - 1U; ++i)
        {
            const auto val1 = data_vec[i];
            const auto val2 = data_vec[i + 1];
            s += static_cast<std::size_t>(op(val1, val2));
        }
    }

    const auto t2 = std::chrono::steady_clock::now();

    std::cout << operation << "<" << std::left << std::setw(11) << type << ">: " << std::setw( 10 ) << ( t2 - t1 ) / 1us << " us (s=" << s << ")\n";
}


int main()
{
    using namespace boost::decimal::detail;

    // Comp, add, sub
    {
        const auto old_vector = generate_random_vector<false, uint128>();
        const auto new_vector = generate_random_vector<false, u128>();
        const auto builtin_vector = generate_random_vector<false, unsigned __int128>();

        test_comparisons(builtin_vector, "builtin");
        test_comparisons(old_vector, "old");
        test_comparisons(new_vector, "new");

        test_two_element_operation(builtin_vector, std::plus<>(), "Addition", "Builtin");
        test_two_element_operation(old_vector, std::plus<>(), "Addition", "Old");
        test_two_element_operation(new_vector, std::plus<>(), "Addition", "New");

        test_two_element_operation(builtin_vector, std::minus<>(), "Subtraction", "Builtin");
        test_two_element_operation(old_vector, std::minus<>(), "Subtraction", "Old");
        test_two_element_operation(new_vector, std::minus<>(), "Subtraction", "New");

        test_two_element_operation(builtin_vector, std::multiplies<>(), "Two word mul", "Builtin");
        test_two_element_operation(old_vector, std::multiplies<>(), "Two word mul", "Old");
        test_two_element_operation(new_vector, std::multiplies<>(), "Two word mul", "New");

    }
    // Mul
    {
        const auto old_vector = generate_random_vector<true, uint128>();
        const auto new_vector = generate_random_vector<true, u128>();
        const auto builtin_vector = generate_random_vector<true, unsigned __int128>();

        test_two_element_operation(builtin_vector, std::multiplies<>(), "One word mul", "Builtin");
        test_two_element_operation(old_vector, std::multiplies<>(), "One word mul", "Old");
        test_two_element_operation(new_vector, std::multiplies<>(), "One word mul", "New");
    }

    return 1;
}

#else

int main()
{
    std::cerr << "Benchmarks not run" << std::endl;
    return 1;
}

#endif // BOOST_DECIMAL_BENCHMARK_U128
