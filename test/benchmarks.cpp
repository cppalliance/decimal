// Copyright 2023 Peter Dimov
// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/decimal.hpp>
#include <boost/charconv.hpp>
#include <chrono>
#include <random>
#include <vector>
#include <type_traits>
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <cstring>

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
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wfloat-equal"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wstringop-overread"
#  define BOOST_DECIMAL_NO_INLINE __attribute__ ((__noinline__))
#endif

constexpr unsigned N = 20'000'000U;
constexpr int K = 5;

template <typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
std::vector<T> generate_random_vector(std::size_t size = N, unsigned seed = 42U)
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
std::vector<T> generate_random_vector(std::size_t size = N, unsigned seed = 42U)
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

template <typename T, typename Func>
BOOST_DECIMAL_NO_INLINE void test_one_element_operation(const std::vector<T>& data_vec, Func op, const char* operation, const char* type, std::size_t max_element = N)
{
    const auto t1 = std::chrono::steady_clock::now();
    std::size_t s = 0; // discard variable

    for (std::size_t k {}; k < K; ++k)
    {
        for (std::size_t i {}; i < max_element; ++i)
        {
            s += static_cast<std::size_t>(op(data_vec[i]));
        }
    }

    const auto t2 = std::chrono::steady_clock::now();

    std::cout << operation << "<" << std::left << std::setw(11) << type << ">: " << std::setw( 10 ) << ( t2 - t1 ) / 1us << " us (s=" << s << ")\n";
}

template <typename T>
static BOOST_DECIMAL_NO_INLINE void init_input_data( std::vector<T>& data )
{
    using std::isfinite;
    data.reserve( N );

    std::mt19937_64 rng(42);

    for( unsigned i = 0; i < N; ++i )
    {
        std::uint64_t tmp = rng();

        T x;
        std::memcpy( &x, &tmp, sizeof(x) );

        if( !isfinite(x) ) continue;

        data.push_back( x );
    }
}

template <typename T, std::enable_if_t<!std::is_floating_point<T>::value, bool> = true>
static BOOST_NOINLINE void test_boost_to_chars( std::vector<T> const& data, bool general, char const* label, int precision, const char* type )
{
    auto t1 = std::chrono::steady_clock::now();

    std::size_t s = 0;

    for( int i = 0; i < K; ++i )
    {
        char buffer[ 256 ];

        for( auto x: data )
        {
            boost::decimal::chars_format fmt = general? boost::decimal::chars_format::general: boost::decimal::chars_format::scientific;

            auto r = precision == 0?
                     boost::decimal::to_chars( buffer, buffer + sizeof( buffer ), x, fmt ):
                     boost::decimal::to_chars( buffer, buffer + sizeof( buffer ), x, fmt, precision );

            s += static_cast<std::size_t>( r.ptr - buffer );
            s += static_cast<unsigned char>( buffer[0] );
        }
    }

    auto t2 = std::chrono::steady_clock::now();

    std::cout << "boost::decimal::to_chars<" << std::left << std::setw(11) << type << ">,  " << label << ", " << precision << ": " << std::setw( 10 ) << ( t2 - t1 ) / 1us << " us (s=" << s << ")\n";
}

template <typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
static BOOST_NOINLINE void test_boost_to_chars( std::vector<T> const& data, bool general, char const* label, int precision, const char* type )
{
    auto t1 = std::chrono::steady_clock::now();

    std::size_t s = 0;

    for( int i = 0; i < K; ++i )
    {
        char buffer[ 256 ];

        for( auto x: data )
        {
            boost::charconv::chars_format fmt = general? boost::charconv::chars_format::general: boost::charconv::chars_format::scientific;

            auto r = precision == 0?
                     boost::charconv::to_chars( buffer, buffer + sizeof( buffer ), x, fmt ):
                     boost::charconv::to_chars( buffer, buffer + sizeof( buffer ), x, fmt, precision );

            s += static_cast<std::size_t>( r.ptr - buffer );
            s += static_cast<unsigned char>( buffer[0] );
        }
    }

    auto t2 = std::chrono::steady_clock::now();

    std::cout << "boost::charconv::to_chars<" << std::left << std::setw(11) << type << ">, " << label << ", " << precision << ": " << std::setw( 10 ) << ( t2 - t1 ) / 1us << " us (s=" << s << ")\n";
}


template <typename T>
void test_to_chars(const char* type)
{
    std::vector<T> data;
    init_input_data(data);
    test_boost_to_chars(data, false, "scientific", 0, type);
    test_boost_to_chars(data, false, "scientific", 6, type);
    test_boost_to_chars(data, true, "general   ", 0, type);
    test_boost_to_chars(data, true, "general   ", 6, type);
}

template<class T, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
BOOST_DECIMAL_NO_INLINE void init_from_chars_input_data( std::vector<std::string>& data, bool general )
{
    using std::isfinite;

    data.reserve( N );

    std::mt19937_64 rng;

    for( unsigned i = 0; i < N; ++i )
    {
        std::uint64_t tmp = rng();

        T x;
        std::memcpy( &x, &tmp, sizeof(x) );

        if( !isfinite(x) ) continue;

        char buffer[ 64 ];
        auto r = boost::charconv::to_chars( buffer, buffer + sizeof( buffer ), x, general? boost::charconv::chars_format::general: boost::charconv::chars_format::scientific );

        std::string y( buffer, r.ptr );
        data.push_back( y );
    }
}

template<class T, std::enable_if_t<!std::is_floating_point<T>::value, bool> = true>
BOOST_DECIMAL_NO_INLINE void init_from_chars_input_data( std::vector<std::string>& data, bool general )
{
    using std::isfinite;

    data.reserve( N );

    std::mt19937_64 rng;

    for( unsigned i = 0; i < N; ++i )
    {
        std::uint64_t tmp = rng();

        T x;
        std::memcpy( &x, &tmp, sizeof(x) );

        if( !isfinite(x) ) continue;

        char buffer[ 64 ];
        auto r = boost::decimal::to_chars( buffer, buffer + sizeof( buffer ), x, general? boost::decimal::chars_format::general: boost::decimal::chars_format::scientific );

        std::string y( buffer, r.ptr );
        data.push_back( y );
    }
}

template <typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
BOOST_DECIMAL_NO_INLINE void test_boost_from_chars( std::vector<std::string> const& data, bool general, char const* label, char const* type )
{
    auto t1 = std::chrono::steady_clock::now();

    std::size_t s = 0;

    for( int i = 0; i < K; ++i )
    {
        for( auto const& x: data )
        {
            T y;
            auto r = boost::charconv::from_chars( x.data(), x.data() + x.size(), y, general? boost::charconv::chars_format::general: boost::charconv::chars_format::scientific );

            s = static_cast<std::size_t>(r.ec);
        }
    }

    auto t2 = std::chrono::steady_clock::now();

    std::cout << "boost::charconv::from_chars<" << std::left << std::setw(11) << type << ">, " << label << ": " << std::setw( 10 ) << ( t2 - t1 ) / 1us << " us (s=" << s << ")\n";
}

template <typename T, std::enable_if_t<!std::is_floating_point<T>::value, bool> = true>
BOOST_DECIMAL_NO_INLINE void test_boost_from_chars( std::vector<std::string> const& data, bool general, char const* label, char const* type )
{
    auto t1 = std::chrono::steady_clock::now();
    std::size_t s = 0;

    for( int i = 0; i < K; ++i )
    {
        for( auto const& x: data )
        {
            T y;
            auto r = boost::decimal::from_chars( x.data(), x.data() + x.size(), y, general? boost::decimal::chars_format::general: boost::decimal::chars_format::scientific );

            s = static_cast<std::size_t>(r.ec);
        }
    }

    auto t2 = std::chrono::steady_clock::now();

    std::cout << "boost::decimal::from_chars<" << std::left << std::setw(11) << type << ">,  " << label << ": " << std::setw( 10 ) << ( t2 - t1 ) / 1us << " us (s=" << s << ")\n";
}

template <typename T>
void test_from_chars(bool general, const char* type)
{
    std::vector<std::string> data;
    init_from_chars_input_data<T>(data, general);

    char const* label = general? "general   ": "scientific";
    test_boost_from_chars<T>( data, general, label, type );
}

int main()
{
    const auto float_vector = generate_random_vector<float>();
    const auto double_vector = generate_random_vector<double>();
    const auto dec32_vector = generate_random_vector<decimal32>();
    const auto dec64_vector = generate_random_vector<decimal64>();
    const auto dec128_vector = generate_random_vector<decimal128>();

    const auto dec32_fast_vector = generate_random_vector<decimal32_fast>();
    const auto dec64_fast_vector = generate_random_vector<decimal64_fast>();
    const auto dec128_fast_vector = generate_random_vector<decimal128_fast>();

    std::cout << "===== Comparisons =====\n";

    test_comparisons(float_vector, "float");
    test_comparisons(double_vector, "double");
    test_comparisons(dec32_vector, "decimal32");
    test_comparisons(dec64_vector, "decimal64");
    test_comparisons(dec128_vector, "decimal128");
    test_comparisons(dec32_fast_vector, "dec32_fast");
    test_comparisons(dec64_fast_vector, "dec64_fast");
    test_comparisons(dec128_fast_vector, "dec128_fast");

    std::cout << "\n===== Addition =====\n";

    test_two_element_operation(float_vector, std::plus<>(), "Addition", "float");
    test_two_element_operation(double_vector, std::plus<>(), "Addition", "double");
    test_two_element_operation(dec32_vector, std::plus<>(), "Addition", "decimal32");
    test_two_element_operation(dec64_vector, std::plus<>(), "Addition", "decimal64");
    test_two_element_operation(dec128_vector, std::plus<>(), "Addition", "decimal128");
    test_two_element_operation(dec32_fast_vector, std::plus<>(), "Addition", "dec32_fast");
    test_two_element_operation(dec64_fast_vector, std::plus<>(), "Addition", "dec64_fast");
    test_two_element_operation(dec128_fast_vector, std::plus<>(), "Addition", "dec128_fast");

    std::cout << "\n===== Subtraction =====\n";

    test_two_element_operation(float_vector, std::minus<>(), "Subtraction", "float");
    test_two_element_operation(double_vector, std::minus<>(), "Subtraction", "double");
    test_two_element_operation(dec32_vector, std::minus<>(), "Subtraction", "decimal32");
    test_two_element_operation(dec64_vector, std::minus<>(), "Subtraction", "decimal64");
    test_two_element_operation(dec128_vector, std::minus<>(), "Subtraction", "decimal128");
    test_two_element_operation(dec32_fast_vector, std::minus<>(), "Subtraction", "dec32_fast");
    test_two_element_operation(dec64_fast_vector, std::minus<>(), "Subtraction", "dec64_fast");
    test_two_element_operation(dec128_fast_vector, std::minus<>(), "Subtraction", "dec128_fast");

    std::cout << "\n===== Multiplication =====\n";

    test_two_element_operation(float_vector, std::multiplies<>(), "Multiplication", "float");
    test_two_element_operation(double_vector, std::multiplies<>(), "Multiplication", "double");
    test_two_element_operation(dec32_vector, std::multiplies<>(), "Multiplication", "decimal32");
    test_two_element_operation(dec64_vector, std::multiplies<>(), "Multiplication", "decimal64");
    test_two_element_operation(dec128_vector, std::multiplies<>(), "Multiplication", "decimal128");
    test_two_element_operation(dec32_fast_vector, std::multiplies<>(), "Multiplication", "dec32_fast");
    test_two_element_operation(dec64_fast_vector, std::multiplies<>(), "Multiplication", "dec64_fast");
    test_two_element_operation(dec128_fast_vector, std::multiplies<>(), "Multiplication", "dec128_fast");

    std::cout << "\n===== Division =====\n";

    test_two_element_operation(float_vector, std::divides<>(), "Division", "float");
    test_two_element_operation(double_vector, std::divides<>(), "Division", "double");
    test_two_element_operation(dec32_vector, std::divides<>(), "Division", "decimal32");
    test_two_element_operation(dec64_vector, std::divides<>(), "Division", "decimal64");
    test_two_element_operation(dec128_vector, std::divides<>(), "Division", "decimal128");
    test_two_element_operation(dec32_fast_vector, std::divides<>(), "Division", "dec32_fast");
    test_two_element_operation(dec64_fast_vector, std::divides<>(), "Division", "dec64_fast");
    test_two_element_operation(dec64_fast_vector, std::divides<>(), "Division", "dec128_fast");

/*
    std::cout << "\n===== sqrt =====\n";

    test_one_element_operation(float_vector, (float(*)(float))std::sqrt, "sqrt", "float");
    test_one_element_operation(double_vector, (double(*)(double))std::sqrt, "sqrt", "double");
    test_one_element_operation(dec32_vector, (decimal32(*)(decimal32))sqrt, "sqrt", "decimal32");
    test_one_element_operation(dec64_vector, (decimal64(*)(decimal64))sqrt, "sqrt", "decimal64");
    test_one_element_operation(dec128_vector, (decimal128(*)(decimal128))sqrt, "sqrt", "decimal128");

    std::cout << "\n===== <charconv> to_chars =====\n";
    test_to_chars<float>("float");
    test_to_chars<double>("double");
    test_to_chars<decimal32>("decimal32");
    test_to_chars<decimal64>("decimal64");
    test_to_chars<decimal128>("decimal128");

    std::cout << "\n===== <charconv> from_chars =====\n";
    test_from_chars<float>(false, "float");
    test_from_chars<float>(true, "float");
    test_from_chars<double>(false, "double");
    test_from_chars<double>(true, "double");
    test_from_chars<decimal32>(false, "decimal32");
    test_from_chars<decimal32>(true, "decimal32");
    test_from_chars<decimal64>(false, "decimal64");
    test_from_chars<decimal64>(true, "decimal64");
    test_from_chars<decimal128>(false, "decimal128");
    test_from_chars<decimal128>(true, "decimal128");
*/
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
