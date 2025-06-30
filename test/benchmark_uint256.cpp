// Copyright 2025 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <iostream>

#ifdef BOOST_DECIMAL_BENCHMARK_U256

#include <boost/decimal/detail/u256.hpp>
#include <boost/decimal/detail/emulated256.hpp>
#include <boost/decimal/detail/integer_search_trees.hpp>
#include <chrono>
#include <random>
#include <vector>
#include <type_traits>
#include <iomanip>
#include <string>
#include <cmath>
#include <cstring>
#include <functional>

constexpr unsigned N = 200'000'000;
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
#  define BOOST_DECIMAL_NO_INLINE __attribute__ ((__noinline__))
#  if __GNUC__ >= 11
#    pragma GCC diagnostic ignored "-Wstringop-overread"
#  endif
#endif

// 4 = 4 words
// 5 = Random width

template <int words>
std::vector<detail::uint256_t> generate_random_vector_old(std::size_t size = N, unsigned seed = 42U)
{
    if (seed == 0)
    {
        std::random_device rd;
        seed = rd();
    }

    std::mt19937_64 gen(seed);
    std::uniform_int_distribution<std::uint64_t> dist(UINT64_C(0), UINT64_MAX);
    std::uniform_int_distribution<int> size_dist(0, 1);

    std::vector<detail::uint256_t> result(size);
    for (std::size_t i = 0; i < size; ++i)
    {
        switch (words)
        {
            case 4:
                result[i].low.low = dist(gen);
                result[i].low.high = dist(gen);
                result[i].high.low = dist(gen);
                result[i].high.high = dist(gen);
                break;
            case 3:
                result[i].low.low = dist(gen);
                result[i].low.high = dist(gen);
                result[i].high.low = dist(gen);
                break;
            case 2:
                result[i].low.low = dist(gen);
                result[i].low.high = dist(gen);
                break;
            case 1:
                result[i].low.low = dist(gen);
                break;
            default:
                BOOST_DECIMAL_UNREACHABLE;
        }
    }

    return result;
}

template <int words>
std::vector<detail::u256> generate_random_vector_new(std::size_t size = N, unsigned seed = 42U)
{
    if (seed == 0)
    {
        std::random_device rd;
        seed = rd();
    }

    std::mt19937_64 gen(seed);
    std::uniform_int_distribution<std::uint64_t> dist(UINT64_C(0), UINT64_MAX);
    std::uniform_int_distribution<int> size_dist(0, 1);

    std::vector<detail::u256> result(size);
    for (std::size_t i = 0; i < size; ++i)
    {
        switch (words)
        {
            case 4:
                result[i].bytes[0] = dist(gen);
                result[i].bytes[1] = dist(gen);
                result[i].bytes[2] = dist(gen);
                result[i].bytes[3] = dist(gen);
                break;
            case 3:
                result[i].bytes[0] = dist(gen);
                result[i].bytes[1] = dist(gen);
                result[i].bytes[2] = dist(gen);
                break;
            case 2:
                result[i].bytes[0] = dist(gen);
                result[i].bytes[1] = dist(gen);
                break;
            case 1:
                result[i].bytes[0] = dist(gen);
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
            //s += static_cast<std::size_t>(val1 > val2);
            //s += static_cast<std::size_t>(val1 >= val2);
            //s += static_cast<std::size_t>(val1 < val2);
            //s += static_cast<std::size_t>(val1 <= val2);
            s += static_cast<std::size_t>(val1 == val2);
            s += static_cast<std::size_t>(val1 != val2);
        }
    }

    const auto t2 = std::chrono::steady_clock::now();

    std::cout << "comp<" << std::left << std::setw(11) << label << ">: " << std::setw( 10 ) << ( t2 - t1 ) / 1us << " us (s=" << s << ")\n";
}

template <typename T>
BOOST_DECIMAL_NO_INLINE void test_bitwise_ops(const std::vector<T>& data_vec, const char* label)
{
    const auto t1 = std::chrono::steady_clock::now();
    std::size_t s = 0; // discard variable

    for (std::size_t k {}; k < K; ++k)
    {
        for (std::size_t i {}; i < data_vec.size() - 1U; ++i)
        {
            const auto val1 = data_vec[i];
            const auto val2 = data_vec[i + 1];
            s += static_cast<std::size_t>(val1 | val2);
            s += static_cast<std::size_t>(val1 & val2);
        }
    }

    const auto t2 = std::chrono::steady_clock::now();

    std::cout << "bits<" << std::left << std::setw(11) << label << ">: " << std::setw( 10 ) << ( t2 - t1 ) / 1us << " us (s=" << s << ")\n";
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

template <typename T>
BOOST_DECIMAL_NO_INLINE void test_digit_counting(const std::vector<T>& data_vec, const char* label)
{
    const auto t1 = std::chrono::steady_clock::now();
    std::size_t s = 0; // discard variable

    for (std::size_t k {}; k < K; ++k)
    {
        for (std::size_t i {}; i < data_vec.size(); ++i)
        {
            const auto val1 = data_vec[i];
            s += static_cast<std::size_t>(boost::decimal::detail::num_digits(val1));
        }
    }

    const auto t2 = std::chrono::steady_clock::now();

    std::cout << "digits<" << std::left << std::setw(11) << label << ">: " << std::setw( 10 ) << ( t2 - t1 ) / 1us << " us (s=" << s << ")\n";
}


int main()
{
    using namespace boost::decimal::detail;

    // Two word operations
    {
        std::cout << "\n---------------------------\n";
        std::cout << "Four Word Operations\n";
        std::cout << "---------------------------\n\n";

        const auto old_vector = generate_random_vector_old<4>();
        const auto new_vector = generate_random_vector_new<4>();

        test_comparisons(old_vector, "old");
        test_comparisons(new_vector, "new");

        std::cout << std::endl;

        test_two_element_operation(old_vector, std::plus<>(), "add", "Old");
        test_two_element_operation(new_vector, std::plus<>(), "add", "New");

        std::cout << std::endl;

        test_two_element_operation(old_vector, std::multiplies<>(), "mul", "Old");
        test_two_element_operation(new_vector, std::multiplies<>(), "mul", "New");

        std::cout << std::endl;

        test_two_element_operation(old_vector, std::divides<>(), "div", "Old");
        test_two_element_operation(new_vector, std::divides<>(), "div", "New");

        std::cout << std::endl;

        test_digit_counting(old_vector, "old");
        test_digit_counting(new_vector, "new");
    }
    // Single word operations
    /*
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

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_digit_counting(builtin_vector, "builtin");
        #endif

        test_digit_counting(old_vector, "old");
        test_digit_counting(new_vector, "new");
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

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_digit_counting(builtin_vector, "builtin");
        #endif

        test_digit_counting(old_vector, "old");
        test_digit_counting(new_vector, "new");
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

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_digit_counting(builtin_vector, "builtin");
        #endif

        test_digit_counting(old_vector, "old");
        test_digit_counting(new_vector, "new");
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

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_two_element_operation(builtin_vector, std::modulus<>(), "mod", "Builtin");
        #endif

        test_two_element_operation(old_vector, std::modulus<>(), "mod", "Old");
        test_two_element_operation(new_vector, std::modulus<>(), "mod", "New");

        std::cout << std::endl;

        #ifdef BOOST_DECIMAL_HAS_INT128
        test_digit_counting(builtin_vector, "builtin");
        #endif

        test_digit_counting(old_vector, "old");
        test_digit_counting(new_vector, "new");
    }
        */
    return 1;
}

#else

int main()
{
    std::cerr << "Benchmarks not run" << std::endl;
    return 1;
}

#endif // BOOST_DECIMAL_BENCHMARK_U128
