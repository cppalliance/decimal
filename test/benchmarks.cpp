// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <chrono>
#include <random>
#include <vector>
#include <type_traits>
#include <iostream>
#include <iomanip>
#define BOOST_DECIMAL_RUN_BENCHMARKS
#ifdef BOOST_DECIMAL_RUN_BENCHMARKS

using namespace boost::decimal;
using namespace std::chrono_literals;

#ifdef __clang__
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wfloat-equal"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#  define BOOST_DECIMAL_NO_INLINE __attribute__ ((__noinline__))
#elif defined(_MSC_VER)
#  define BOOST_DECIMAL_NO_INLINE __declspec(noinline)
#endif

template <typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
std::vector<T> generate_random_vector(std::size_t size = 2'000'000U, unsigned seed = 42U)
{
    if (seed == 0)
    {
        std::random_device rd;
        seed = rd();
    }
    std::vector<T> v(size);

    std::mt19937_64 gen(seed);

    std::uniform_real_distribution<T> dis(0, 1);
    for (std::size_t i = 0; i < v.size(); ++i)
    {
        v[i] = dis(gen);
    }
    return v;
}

template <typename T, std::enable_if_t<!std::is_floating_point<T>::value, bool> = true>
std::vector<T> generate_random_vector(std::size_t size = 2'000'000U, unsigned seed = 42U)
{
    if (seed == 0)
    {
        std::random_device rd;
        seed = rd();
    }
    std::vector<T> v(size);

    std::mt19937_64 gen(seed);

    std::uniform_real_distribution<double> dis(0, 1);
    for (std::size_t i = 0; i < v.size(); ++i)
    {
        v[i] = T{dis(gen)};
    }
    return v;
}

template <typename T>
BOOST_DECIMAL_NO_INLINE void test_comparisons(const std::vector<T>& data_vec, const char* label)
{
    const auto t1 = std::chrono::steady_clock::now();
    std::size_t s = 0; // discard variable

    for (std::size_t trials {}; trials < 5; ++trials)
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

    std::cout << "comparisons<" << std::left << std::setw(10) << label << ">: " << std::setw( 10 ) << ( t2 - t1 ) / 1us << " us (s=" << s << ")\n";
}

template <typename T, typename Func>
BOOST_DECIMAL_NO_INLINE void test_two_element_operation(const std::vector<T>& data_vec, Func op, const char* operation, const char* type)
{
    const auto t1 = std::chrono::steady_clock::now();
    std::size_t s = 0; // discard variable

    for (std::size_t trials {}; trials < 5; ++trials)
    {
        for (std::size_t i {}; i < data_vec.size() - 1U; ++i)
        {
            const auto val1 = data_vec[i];
            const auto val2 = data_vec[i + 1];
            s += static_cast<std::size_t>(op(val1, val2));
        }
    }

    const auto t2 = std::chrono::steady_clock::now();

    std::cout << operation << "<" << std::left << std::setw(10) << type << ">: " << std::setw( 10 ) << ( t2 - t1 ) / 1us << " us (s=" << s << ")\n";
}

template <typename T, typename Func>
BOOST_DECIMAL_NO_INLINE void test_one_element_operation(const std::vector<T>& data_vec, Func op, const char* operation, const char* type, std::size_t max_element = 2'000'000U)
{
    const auto t1 = std::chrono::steady_clock::now();
    std::size_t s = 0; // discard variable

    for (std::size_t trials {}; trials < 5; ++trials)
    {
        for (std::size_t i {}; i < max_element; ++i)
        {
            s += static_cast<std::size_t>(op(data_vec[i]));
        }
    }

    const auto t2 = std::chrono::steady_clock::now();

    std::cout << operation << "<" << std::left << std::setw(10) << type << ">: " << std::setw( 10 ) << ( t2 - t1 ) / 1us << " us (s=" << s << ")\n";
}

int main()
{
    const auto float_vector = generate_random_vector<float>();
    const auto double_vector = generate_random_vector<double>();
    const auto dec32_vector = generate_random_vector<decimal32>();
    const auto dec64_vector = generate_random_vector<decimal64>();
    const auto dec128_vector = generate_random_vector<decimal128>();

    std::cout << "===== Comparisons =====\n";

    test_comparisons(float_vector, "float");
    test_comparisons(double_vector, "double");
    test_comparisons(dec32_vector, "decimal32");
    test_comparisons(dec64_vector, "decimal64");
    test_comparisons(dec128_vector, "decimal128");

    std::cout << "\n===== Addition =====\n";

    test_two_element_operation(float_vector, std::plus<>(), "Addition", "float");
    test_two_element_operation(double_vector, std::plus<>(), "Addition", "double");
    test_two_element_operation(dec32_vector, std::plus<>(), "Addition", "decimal32");
    test_two_element_operation(dec64_vector, std::plus<>(), "Addition", "decimal64");
    test_two_element_operation(dec128_vector, std::plus<>(), "Addition", "decimal128");

    std::cout << "\n===== Subtraction =====\n";

    test_two_element_operation(double_vector, std::minus<>(), "Subtraction", "double");
    test_two_element_operation(float_vector, std::minus<>(), "Subtraction", "float");
    test_two_element_operation(dec32_vector, std::minus<>(), "Subtraction", "decimal32");
    test_two_element_operation(dec64_vector, std::minus<>(), "Subtraction", "decimal64");
    test_two_element_operation(dec128_vector, std::minus<>(), "Subtraction", "decimal128");

    std::cout << "\n===== Multiplication =====\n";

    test_two_element_operation(float_vector, std::multiplies<>(), "Multiplication", "float");
    test_two_element_operation(double_vector, std::multiplies<>(), "Multiplication", "double");
    test_two_element_operation(dec32_vector, std::multiplies<>(), "Multiplication", "decimal32");
    test_two_element_operation(dec64_vector, std::multiplies<>(), "Multiplication", "decimal64");
    test_two_element_operation(dec128_vector, std::multiplies<>(), "Multiplication", "decimal128");

    std::cout << "\n===== Division =====\n";

    test_two_element_operation(float_vector, std::divides<>(), "Division", "float");
    test_two_element_operation(double_vector, std::divides<>(), "Division", "double");
    test_two_element_operation(dec32_vector, std::divides<>(), "Division", "decimal32");
    test_two_element_operation(dec64_vector, std::divides<>(), "Division", "decimal64");
    test_two_element_operation(dec128_vector, std::divides<>(), "Division", "decimal128");

    std::cout << "\n===== sqrt =====\n";

    test_one_element_operation(float_vector, (float(*)(float))std::sqrt, "sqrt", "float");
    test_one_element_operation(double_vector, (double(*)(double))std::sqrt, "sqrt", "double");
    test_one_element_operation(dec32_vector, (decimal32(*)(decimal32))sqrt, "sqrt", "decimal32");
    test_one_element_operation(dec64_vector, (decimal64(*)(decimal64))sqrt, "sqrt", "decimal64");
    test_one_element_operation(dec128_vector, (decimal128(*)(decimal128))sqrt, "sqrt", "decimal128");

    std::cout << std::endl;

    return 1;
}

#else

int main()
{
    std::cout << "Benchmarks not run" << std::endl;
    return 1;
}

#endif
