
//  Copyright John Maddock 2016.
//  Copyright Matt Borland 2024.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma nv_diag_suppress 186

#include <iostream>
#include <iomanip>
#include <vector>
#include <boost/decimal.hpp>
#include "cuda_managed_ptr.hpp"
#include "stopwatch.hpp"

// For the CUDA runtime routines (prefixed with "cuda_")
#include <cuda_runtime.h>

using float_type = boost::decimal::decimal32_fast;

/**
 * CUDA Kernel Device code
 *
 */
__global__ void cuda_test(const float_type *in, int *out, int numElements)
{
    using std::cos;
    int i = blockDim.x * blockIdx.x + threadIdx.x;

    if (i < numElements)
    {
        out[i] = static_cast<int>(signbit(in[i]) + isinf(in[i]) + isnan(in[i]) + issignaling(in[i]) + isnormal(in[i]) + isfinite(in[i]));
    }
}

/**
 * Host main routine
 */
int main(void)
{
    // Error code to check return values for CUDA calls
    cudaError_t err = cudaSuccess;

    // Print the vector length to be used, and compute its size
    int numElements = 50000;
    std::cout << "[Vector operation on " << numElements << " elements]" << std::endl;

    // Allocate the managed input vector A
    cuda_managed_ptr<float_type> input_vector(numElements);

    // Allocate the managed output vector C
    cuda_managed_ptr<int> output_vector(numElements);

    // Initialize the input vectors
    for (int i = 0; i < numElements; ++i)
    {
        input_vector[i] = static_cast<float_type>(rand());
    }

    // Launch the Vector Add CUDA Kernel
    int threadsPerBlock = 1024;
    int blocksPerGrid =(numElements + threadsPerBlock - 1) / threadsPerBlock;
    std::cout << "CUDA kernel launch with " << blocksPerGrid << " blocks of " << threadsPerBlock << " threads" << std::endl;

    watch w;

    cuda_test<<<blocksPerGrid, threadsPerBlock>>>(input_vector.get(), output_vector.get(), numElements);
    cudaDeviceSynchronize();

    std::cout << "CUDA kernal done in: " << w.elapsed() << "s" << std::endl;

    err = cudaGetLastError();

    if (err != cudaSuccess)
    {
        std::cerr << "Failed to launch vectorAdd kernel (error code " << cudaGetErrorString(err) << ")!" << std::endl;
        return EXIT_FAILURE;
    }

    // Verify that the result vector is correct
    std::vector<int> results;
    results.reserve(numElements);
    w.reset();
    for(int i = 0; i < numElements; ++i)
    {
       results.push_back(static_cast<int>(signbit(input_vector[i]) + isinf(input_vector[i]) + isnan(input_vector[i]) + issignaling(input_vector[i]) + isnormal(input_vector[i]) + isfinite(input_vector[i])));
    }
    double t = w.elapsed();
    // check the results
    for(int i = 0; i < numElements; ++i)
    {
        if (output_vector[i] != results[i])
        {
            std::cerr << "Result verification failed at element " << i << "!\n"
                      << "Cuda: " << output_vector[i] << '\n'
                      << "Serial: " << results[i] << std::endl;
            return EXIT_FAILURE;
        }
    }

    std::cout << "Test PASSED, normal calculation time: " << t << "s" << std::endl;
    std::cout << "Done\n";

    return 0;
}
