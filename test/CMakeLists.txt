# Copyright 2018, 2019 Peter Dimov
# Copyright 2023 Matt Borland
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

include(BoostTestJamfile OPTIONAL RESULT_VARIABLE HAVE_BOOST_TEST)

if(HAVE_BOOST_TEST)

    if(BOOST_DECIMAL_ENABLE_CUDA)

        message(STATUS "Building boost.decimal with CUDA")
        enable_language(CUDA)
        set(CMAKE_CUDA_EXTENSIONS OFF)

        enable_testing()

        boost_test_jamfile(FILE cuda_jamfile LINK_LIBRARIES Boost::decimal Boost::core Boost::math Boost::multiprecision Boost::charconv Boost::type_traits )

    elseif(BOOST_DECIMAL_ENABLE_SYCL)

        message(STATUS "Building boost.decimal with SYCL")

        set(CMAKE_CXX_COMPILER "icpx")
        set(CMAKE_C_COMPILER "icx")

        boost_test_jamfile(FILE sycl_jamfile LINK_LIBRARIES Boost::decimal Boost::core Boost::math Boost::multiprecision Boost::charconv Boost::type_traits sycl COMPILE_OPTIONS -fsycl  )

    else()

        enable_testing()
        boost_test_jamfile(FILE Jamfile LINK_LIBRARIES Boost::decimal Boost::core Boost::math Boost::multiprecision Boost::charconv Boost::type_traits )

    endif()

endif()
