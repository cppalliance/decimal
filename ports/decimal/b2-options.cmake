# Copyright 2024 Matt Borland
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt

if(APPLE)
    list(APPEND B2_OPTIONS cxxstd=14)
endif()
