// Copyright 2024 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <inttypes.h>

#define K 20000000
#define N 5

double float_rand(double min, double max)
{
    float scale = rand() / (double) RAND_MAX;
    return min + scale * (max - min);
}

__attribute__ ((__noinline__)) void generate_vector_32(_decimal32_t* buffer, size_t buffer_len)
{
    size_t i = 0;
    while (i < buffer_len)
    {
        buffer[i] = float_rand(0.0, 1.0);
        ++i;
    }
}

__attribute__ ((__noinline__)) void test_comparisons_32(_decimal32_t* data, const char* label)
{
    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    
    size_t s = 0;

    for (size_t n = 0; n < N; ++n)
    {
        for (size_t k = 0; k < K - 1; ++k)
        {
            _decimal32_t val1 = data[k];
            _decimal32_t val2 = data[k + 1];

            s += (size_t)(val1 > val2);
            s += (size_t)(val1 >= val2);
            s += (size_t)(val1 < val2);
            s += (size_t)(val1 <= val2);
            s += (size_t)(val1 == val2);
            s += (size_t)(val1 != val2);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t2);

    uint64_t elapsed_time_us = (uint64_t)((t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_nsec - t1.tv_nsec) / 1000);
    printf("Comparisons    <%-10s >: %-10" PRIu64 " us (s=%zu)\n", label, elapsed_time_us, s);
}

__attribute__ ((__noinline__)) void generate_vector_64(_decimal64_t* buffer, size_t buffer_len)
{
    size_t i = 0;
    while (i < buffer_len)
    {
        buffer[i] = float_rand(0.0, 1.0);
        ++i;
    }
}

__attribute__ ((__noinline__)) void test_comparisons_64(_decimal64_t* data, const char* label)
{
    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    
    size_t s = 0;

    for (size_t n = 0; n < N; ++n)
    {
        for (size_t k = 0; k < K - 1; ++k)
        {
            _decimal64_t val1 = data[k];
            _decimal64_t val2 = data[k + 1];

            s += (size_t)(val1 > val2);
            s += (size_t)(val1 >= val2);
            s += (size_t)(val1 < val2);
            s += (size_t)(val1 <= val2);
            s += (size_t)(val1 == val2);
            s += (size_t)(val1 != val2);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t2);

    uint64_t elapsed_time_us = (uint64_t)((t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_nsec - t1.tv_nsec) / 1000);
    printf("Comparisons    <%-10s >: %-10" PRIu64 " us (s=%zu)\n", label, elapsed_time_us, s);
}

__attribute__ ((__noinline__)) void generate_vector_128(_decimal128_t* buffer, size_t buffer_len)
{
    size_t i = 0;
    while (i < buffer_len)
    {
        buffer[i] = float_rand(0.0, 1.0);
        ++i;
    }
}

__attribute__ ((__noinline__)) void test_comparisons_128(_decimal128_t* data, const char* label)
{
    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    
    size_t s = 0;

    for (size_t n = 0; n < N; ++n)
    {
        for (size_t k = 0; k < K - 1; ++k)
        {
            _decimal128_t val1 = data[k];
            _decimal128_t val2 = data[k + 1];

            s += (size_t)(val1 > val2);
            s += (size_t)(val1 >= val2);
            s += (size_t)(val1 < val2);
            s += (size_t)(val1 <= val2);
            s += (size_t)(val1 == val2);
            s += (size_t)(val1 != val2);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t2);

    uint64_t elapsed_time_us = (uint64_t)((t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_nsec - t1.tv_nsec) / 1000);
    printf("Comparisons    <%-10s>: %-10" PRIu64 " us (s=%zu)\n", label, elapsed_time_us, s);
}

typedef _decimal32_t (*operation_32)(_decimal32_t, _decimal32_t);

__attribute__ ((__noinline__)) _decimal32_t add_32(_decimal32_t a, _decimal32_t b)
{
    return a + b;
}
__attribute__ ((__noinline__)) _decimal32_t sub_32(_decimal32_t a, _decimal32_t b)
{
    return a - b;
}

__attribute__ ((__noinline__)) _decimal32_t mul_32(_decimal32_t a, _decimal32_t b)
{
    return a * b;
}

__attribute__ ((__noinline__)) _decimal32_t div_32(_decimal32_t a, _decimal32_t b)
{
    return a / b;
}

__attribute__ ((__noinline__)) void test_two_element_operation_32(_decimal32_t* data, operation_32 op, const char* label, const char* op_label)
{
    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    
    size_t s = 0;

    for (size_t n = 0; n < N; ++n)
    {
        for (size_t k = 0; k < K - 1; ++k)
        {
            _decimal32_t val1 = data[k];
            _decimal32_t val2 = data[k + 1];

            s += (size_t)op(val1, val2);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t2);

    uint64_t elapsed_time_us = (uint64_t)((t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_nsec - t1.tv_nsec) / 1000);
    printf("%-15s<%-10s >: %-10" PRIu64 " us (s=%zu)\n", op_label, label, elapsed_time_us, s);
}

typedef _decimal64_t (*operation_64)(_decimal64_t, _decimal64_t);

__attribute__ ((__noinline__)) _decimal64_t add_64(_decimal64_t a, _decimal64_t b)
{
    return a + b;
}

__attribute__ ((__noinline__)) _decimal64_t sub_64(_decimal64_t a, _decimal64_t b)
{
    return a - b;
}

__attribute__ ((__noinline__)) _decimal64_t mul_64(_decimal64_t a, _decimal64_t b)
{
    return a * b;
}

__attribute__ ((__noinline__)) _decimal64_t div_64(_decimal64_t a, _decimal64_t b)
{
    return a / b;
}

__attribute__ ((__noinline__)) void test_two_element_operation_64(_decimal64_t* data, operation_64 op, const char* label, const char* op_label)
{
    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    
    size_t s = 0;

    for (size_t n = 0; n < N; ++n)
    {
        for (size_t k = 0; k < K - 1; ++k)
        {
            _decimal64_t val1 = data[k];
            _decimal64_t val2 = data[k + 1];

            s += (size_t)op(val1, val2);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t2);

    uint64_t elapsed_time_us = (uint64_t)((t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_nsec - t1.tv_nsec) / 1000);
    printf("%-15s<%-10s >: %-10" PRIu64 " us (s=%zu)\n", op_label, label, elapsed_time_us, s);
}

typedef _decimal128_t (*operation_128)(_decimal128_t, _decimal128_t);

__attribute__ ((__noinline__)) _decimal128_t add_128(_decimal128_t a, _decimal128_t b)
{
    return a + b;
}

__attribute__ ((__noinline__)) _decimal128_t sub_128(_decimal128_t a, _decimal128_t b)
{
    return a - b;
}

__attribute__ ((__noinline__)) _decimal128_t mul_128(_decimal128_t a, _decimal128_t b)
{
    return a * b;
}

__attribute__ ((__noinline__)) _decimal128_t div_128(_decimal128_t a, _decimal128_t b)
{
    return a / b;
}

__attribute__ ((__noinline__)) void test_two_element_operation_128(_decimal128_t* data, operation_128 op, const char* label, const char* op_label)
{
    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    
    size_t s = 0;

    for (size_t n = 0; n < N; ++n)
    {
        for (size_t k = 0; k < K - 1; ++k)
        {
            _decimal128_t val1 = data[k];
            _decimal128_t val2 = data[k + 1];

            s += (size_t)op(val1, val2);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t2);

    uint64_t elapsed_time_us = (uint64_t)((t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_nsec - t1.tv_nsec) / 1000);
    printf("%-15s<%-10s>: %-10" PRIu64 " us (s=%zu)\n", op_label, label, elapsed_time_us, s);
}

int main()
{
    // One time init of random number generator
    srand(time(NULL));
    
    _decimal32_t* d32_array = malloc(K * sizeof(_decimal32_t));
    _decimal64_t* d64_array = malloc(K * sizeof(_decimal64_t));
    _decimal128_t* d128_array = malloc(K * sizeof(_decimal128_t));
    
    if (d32_array == NULL || d64_array == NULL || d128_array == NULL)
    {
        return 1;
    }

    printf("===== Comparisons =====\n");

    generate_vector_32(d32_array, K);
    test_comparisons_32(d32_array, "_decimal32_t");

    generate_vector_64(d64_array, K);
    test_comparisons_64(d64_array, "_decimal64_t");

    generate_vector_128(d128_array, K);
    test_comparisons_128(d128_array, "_decimal128_t");
    
    printf("\n===== Addition =====\n");

    test_two_element_operation_32(d32_array, add_32, "_decimal32_t", "Addition");
    test_two_element_operation_64(d64_array, add_64, "_decimal64_t", "Addition");
    test_two_element_operation_128(d128_array, add_128, "_decimal128_t", "Addition");

    printf("\n===== Subtraction =====\n");

    test_two_element_operation_32(d32_array, sub_32, "_decimal32_t", "Subtraction");
    test_two_element_operation_64(d64_array, sub_64, "_decimal64_t", "Subtraction");
    test_two_element_operation_128(d128_array, sub_128, "_decimal128_t", "Subtraction");

    printf("\n===== Multiplication =====\n");

    test_two_element_operation_32(d32_array, mul_32, "_decimal32_t", "Multiplication");
    test_two_element_operation_64(d64_array, mul_64, "_decimal64_t", "Multiplication");
    test_two_element_operation_128(d128_array, mul_128, "_decimal128_t", "Multiplication");

    printf("\n===== Division =====\n");

    test_two_element_operation_32(d32_array, div_32, "_decimal32_t", "Division");
    test_two_element_operation_64(d64_array, div_64, "_decimal64_t", "Division");
    test_two_element_operation_128(d128_array, div_128, "_decimal128_t", "Division");

    free(d32_array);
    free(d64_array);
    free(d128_array);

    return 0;
}
