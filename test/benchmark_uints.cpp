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

// 0 = 1 word
// 1 = 2 words
// 2 = 2 word / 1 word alternating
// 3 = 1 word / 2 word alternating
// 4 = Random width

template <int words, typename T>
std::vector<T> generate_random_vector(std::size_t size = N, unsigned seed = 42U)
{
    if (seed == 0)
    {
        std::random_device rd;
        seed = rd();
    }

    std::mt19937_64 gen(seed);
    std::uniform_int_distribution<std::uint64_t> dist(UINT64_C(0), UINT64_MAX);
    std::uniform_int_distribution<int> size_dist(0, 1);

    std::vector<T> result(size);
    for (std::size_t i = 0; i < size; ++i)
    {
        switch (words)
        {
            case 0:
                result[i] = T{dist(gen)};
                break;

            case 1:
                result[i] = T{dist(gen), dist(gen)};
                break;

            case 2:
                if (i % 2 == 0)
                {
                    result[i] = T{dist(gen), dist(gen)};
                }
                else
                {
                    result[i] = T{dist(gen)};
                }
                break;

            case 3:
                if (i % 2 == 1)
                {
                    result[i] = T{dist(gen), dist(gen)};
                }
                else
                {
                    result[i] = T{dist(gen)};
                }
                break;

            case 4:
                if (size_dist(gen) == 1)
                {
                    result[i] = T{dist(gen), dist(gen)};
                }
                else
                {
                    result[i] = T{dist(gen)};
                }
                break;

            default:
                BOOST_DECIMAL_UNREACHABLE;
        }
    }

    return result;
}

template <int words>
std::vector<unsigned __int128> generate_random_builtin_vector(std::size_t size = N, unsigned seed = 42U)
{
    if (seed == 0)
    {
        std::random_device rd;
        seed = rd();
    }

    std::mt19937_64 gen(seed);
    std::uniform_int_distribution<std::uint64_t> dist(UINT64_C(0), UINT64_MAX);
    std::uniform_int_distribution<int> size_dist(0, 1);

    std::vector<unsigned __int128> result(size);
    for (std::size_t i = 0; i < size; ++i)
    {
        switch (words)
        {
            case 0:
                result[i] = dist(gen);
                break;

            case 1:
                result[i] = static_cast<unsigned __int128>(boost::decimal::detail::u128{dist(gen), dist(gen)});
                break;

            case 2:
                if (i % 2 == 0)
                {
                    result[i] = static_cast<unsigned __int128>(boost::decimal::detail::u128{dist(gen), dist(gen)});
                }
                else
                {
                    result[i] = dist(gen);
                }
                break;

            case 3:
                if (i % 2 == 1)
                {
                    result[i] = static_cast<unsigned __int128>(boost::decimal::detail::u128{dist(gen), dist(gen)});
                }
                else
                {
                    result[i] = dist(gen);
                }
                break;

            case 4:
                if (size_dist(gen) == 1)
                {
                    result[i] = static_cast<unsigned __int128>(boost::decimal::detail::u128{dist(gen), dist(gen)});
                }
                else
                {
                    result[i] = dist(gen);
                }
                break;

            default:
                BOOST_DECIMAL_UNREACHABLE;
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

    std::cout << "comp<" << std::left << std::setw(11) << label << ">: " << std::setw( 10 ) << ( t2 - t1 ) / 1us << " us (s=" << s << ")\n";
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

    std::cout << operation << " <" << std::left << std::setw(11) << type << ">: " << std::setw( 10 ) << ( t2 - t1 ) / 1us << " us (s=" << s << ")\n";
}


int main()
{
    using namespace boost::decimal::detail;

    // Two word operations
    {
        std::cout << "\n---------------------------\n";
        std::cout << "Two Word Operations\n";
        std::cout << "---------------------------\n\n";

        const auto old_vector = generate_random_vector<0, uint128>();
        const auto new_vector = generate_random_vector<0, u128>();

        #ifdef BOOST_DECIMAL_HAS_INT128
        const auto builtin_vector = generate_random_builtin_vector<0>();
        test_comparisons(builtin_vector, "builtin");
        #endif

        test_comparisons(old_vector, "old");
        test_comparisons(new_vector, "new");

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_two_element_operation(builtin_vector, std::plus<>(), "add", "Builtin");
        #endif

        test_two_element_operation(old_vector, std::plus<>(), "add", "Old");
        test_two_element_operation(new_vector, std::plus<>(), "add", "New");

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_two_element_operation(builtin_vector, std::minus<>(), "sub", "Builtin");
        #endif

        test_two_element_operation(old_vector, std::minus<>(), "sub", "Old");
        test_two_element_operation(new_vector, std::minus<>(), "sub", "New");

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_two_element_operation(builtin_vector, std::multiplies<>(), "mul", "Builtin");
        #endif

        test_two_element_operation(old_vector, std::multiplies<>(), "mul", "Old");
        test_two_element_operation(new_vector, std::multiplies<>(), "mul", "New");

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_two_element_operation(builtin_vector, std::divides<>(), "div", "Builtin");
        #endif

        test_two_element_operation(old_vector, std::divides<>(), "div", "Old");
        test_two_element_operation(new_vector, std::divides<>(), "div", "New");
    }
    // Single word operations
    {
        std::cout << "\n---------------------------\n";
        std::cout << "One Word Operations\n";
        std::cout << "---------------------------\n\n";

        const auto old_vector = generate_random_vector<1, uint128>();
        const auto new_vector = generate_random_vector<1, u128>();

        #ifdef BOOST_DECIMAL_HAS_INT128
        const auto builtin_vector = generate_random_builtin_vector<1>();
        test_comparisons(builtin_vector, "builtin");
        #endif

        test_comparisons(old_vector, "old");
        test_comparisons(new_vector, "new");

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_two_element_operation(builtin_vector, std::plus<>(), "add", "Builtin");
        #endif

        test_two_element_operation(old_vector, std::plus<>(), "add", "Old");
        test_two_element_operation(new_vector, std::plus<>(), "add", "New");

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_two_element_operation(builtin_vector, std::minus<>(), "sub", "Builtin");
        #endif

        test_two_element_operation(old_vector, std::minus<>(), "sub", "Old");
        test_two_element_operation(new_vector, std::minus<>(), "sub", "New");

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_two_element_operation(builtin_vector, std::multiplies<>(), "mul", "Builtin");
        #endif

        test_two_element_operation(old_vector, std::multiplies<>(), "mul", "Old");
        test_two_element_operation(new_vector, std::multiplies<>(), "mul", "New");

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_two_element_operation(builtin_vector, std::divides<>(), "div", "Builtin");
        #endif

        test_two_element_operation(old_vector, std::divides<>(), "div", "Old");
        test_two_element_operation(new_vector, std::divides<>(), "div", "New");
    }
    {
        // Two word and one word operations Even = 2, odd = 1

        std::cout << "\n---------------------------\n";
        std::cout << "Two-One Word Operations\n";
        std::cout << "---------------------------\n\n";

        const auto old_vector = generate_random_vector<2, uint128>();
        const auto new_vector = generate_random_vector<2, u128>();

        #ifdef BOOST_DECIMAL_HAS_INT128
        const auto builtin_vector = generate_random_builtin_vector<2>();
        test_comparisons(builtin_vector, "builtin");
        #endif

        test_comparisons(old_vector, "old");
        test_comparisons(new_vector, "new");

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_two_element_operation(builtin_vector, std::plus<>(), "add", "Builtin");
        #endif

        test_two_element_operation(old_vector, std::plus<>(), "add", "Old");
        test_two_element_operation(new_vector, std::plus<>(), "add", "New");

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_two_element_operation(builtin_vector, std::minus<>(), "sub", "Builtin");
        #endif

        test_two_element_operation(old_vector, std::minus<>(), "sub", "Old");
        test_two_element_operation(new_vector, std::minus<>(), "sub", "New");

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_two_element_operation(builtin_vector, std::multiplies<>(), "mul", "Builtin");
        #endif

        test_two_element_operation(old_vector, std::multiplies<>(), "mul", "Old");
        test_two_element_operation(new_vector, std::multiplies<>(), "mul", "New");

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_two_element_operation(builtin_vector, std::divides<>(), "div", "Builtin");
        #endif

        test_two_element_operation(old_vector, std::divides<>(), "div", "Old");
        test_two_element_operation(new_vector, std::divides<>(), "div", "New");
    }
    {
        // Two word and one word operations Even = 1, odd = 2

        std::cout << "\n---------------------------\n";
        std::cout << "One-Two Word Operations\n";
        std::cout << "---------------------------\n\n";

        const auto old_vector = generate_random_vector<3, uint128>();
        const auto new_vector = generate_random_vector<3, u128>();

        #ifdef BOOST_DECIMAL_HAS_INT128
        const auto builtin_vector = generate_random_builtin_vector<3>();
        test_comparisons(builtin_vector, "builtin");
        #endif

        test_comparisons(old_vector, "old");
        test_comparisons(new_vector, "new");

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_two_element_operation(builtin_vector, std::plus<>(), "add", "Builtin");
        #endif

        test_two_element_operation(old_vector, std::plus<>(), "add", "Old");
        test_two_element_operation(new_vector, std::plus<>(), "add", "New");

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_two_element_operation(builtin_vector, std::minus<>(), "sub", "Builtin");
        #endif

        test_two_element_operation(old_vector, std::minus<>(), "sub", "Old");
        test_two_element_operation(new_vector, std::minus<>(), "sub", "New");

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_two_element_operation(builtin_vector, std::multiplies<>(), "mul", "Builtin");
        #endif

        test_two_element_operation(old_vector, std::multiplies<>(), "mul", "Old");
        test_two_element_operation(new_vector, std::multiplies<>(), "mul", "New");

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_two_element_operation(builtin_vector, std::divides<>(), "div", "Builtin");
        #endif

        test_two_element_operation(old_vector, std::divides<>(), "div", "Old");
        test_two_element_operation(new_vector, std::divides<>(), "div", "New");
    }
    {
        // Two word and one word operations Even = 1, odd = 2

        std::cout << "\n---------------------------\n";
        std::cout << "Random Width Operations\n";
        std::cout << "---------------------------\n\n";

        const auto old_vector = generate_random_vector<4, uint128>();
        const auto new_vector = generate_random_vector<4, u128>();

        #ifdef BOOST_DECIMAL_HAS_INT128
        const auto builtin_vector = generate_random_builtin_vector<4>();
        test_comparisons(builtin_vector, "builtin");
        #endif

        test_comparisons(old_vector, "old");
        test_comparisons(new_vector, "new");

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_two_element_operation(builtin_vector, std::plus<>(), "add", "Builtin");
        #endif

        test_two_element_operation(old_vector, std::plus<>(), "add", "Old");
        test_two_element_operation(new_vector, std::plus<>(), "add", "New");

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_two_element_operation(builtin_vector, std::minus<>(), "sub", "Builtin");
        #endif

        test_two_element_operation(old_vector, std::minus<>(), "sub", "Old");
        test_two_element_operation(new_vector, std::minus<>(), "sub", "New");

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_two_element_operation(builtin_vector, std::multiplies<>(), "mul", "Builtin");
        #endif

        test_two_element_operation(old_vector, std::multiplies<>(), "mul", "Old");
        test_two_element_operation(new_vector, std::multiplies<>(), "mul", "New");

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_two_element_operation(builtin_vector, std::divides<>(), "div", "Builtin");
        #endif

        test_two_element_operation(old_vector, std::divides<>(), "div", "Old");
        test_two_element_operation(new_vector, std::divides<>(), "div", "New");
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
