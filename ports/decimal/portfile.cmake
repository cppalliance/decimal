# Copyright 2024 Matt Borland
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt
#
# See: https://devblogs.microsoft.com/cppblog/registries-bring-your-own-libraries-to-vcpkg/

vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO cppalliance/decimal
        REF v2.3.0
        SHA512 a5dac21e3f786e028b5e526c615c1ff7f671f2759a6968edaf97de90f5c5ede59e956dca730d14edabce4d86b58d5dacca2dab0706cb9e17474871f6433171eb
        HEAD_REF master
)

file(COPY "${SOURCE_PATH}/include" DESTINATION "${CURRENT_PACKAGES_DIR}")
file(COPY "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
