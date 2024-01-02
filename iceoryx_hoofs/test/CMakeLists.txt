# Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
# Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.16)
set(test_iceoryx_hoofs_VERSION 0)
project(test_iceoryx_hoofs VERSION ${test_iceoryx_hoofs_VERSION})

find_package(Threads REQUIRED)
find_package(iceoryx_hoofs_testing REQUIRED)

set(PROJECT_PREFIX "hoofs")

file(GLOB_RECURSE MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/*.cpp")
file(GLOB_RECURSE INTEGRATIONTESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/integrationtests/*.cpp")
file(GLOB_RECURSE MOCKTESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/mocktests/*.cpp")
file(GLOB STRESSTESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/stresstests/*.cpp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_PREFIX}/test)

set(TEST_LINK_LIBS
    ${CODE_COVERAGE_LIBS}
    GTest::gtest
    GTest::gmock
    iceoryx_hoofs::iceoryx_hoofs
    iceoryx_hoofs_testing::iceoryx_hoofs_testing
)

iox_add_executable( TARGET                  ${PROJECT_PREFIX}_moduletests
                    INCLUDE_DIRECTORIES     .
                    LIBS                    ${TEST_LINK_LIBS}
                    LIBS_LINUX              acl dl pthread rt
                    FILES                   ${MODULETESTS_SRC}
)

iox_add_executable( TARGET                  ${PROJECT_PREFIX}_mocktests
                    INCLUDE_DIRECTORIES     .
                    LIBS                    ${TEST_LINK_LIBS}
                    LIBS_LINUX              acl dl pthread rt
                    FILES                   ${MOCKTESTS_SRC}
)

iox_add_executable( TARGET                  ${PROJECT_PREFIX}_integrationtests
                    INCLUDE_DIRECTORIES     .
                    LIBS                    ${TEST_LINK_LIBS}
                    LIBS_LINUX              acl dl pthread rt
                    FILES                   ${INTEGRATIONTESTS_SRC}
)

iox_add_executable( TARGET                  ${PROJECT_PREFIX}_stresstests
                    INCLUDE_DIRECTORIES     .
                    LIBS                    ${TEST_LINK_LIBS}
                    LIBS_LINUX              acl dl pthread rt
                    FILES                   ${STRESSTESTS_SRC}
)

iox_add_executable( TARGET                  test_stress_spsc_sofi
                    INCLUDE_DIRECTORIES     .
                    LIBS                    ${TEST_LINK_LIBS}
                    LIBS_LINUX              acl dl pthread rt
                    FILES                   stresstests/sofi/test_stress_spsc_sofi.cpp
)

add_subdirectory(stresstests/benchmark_optional_and_expected)

target_compile_options(${PROJECT_PREFIX}_moduletests PRIVATE ${ICEORYX_TEST_CXX_FLAGS})
target_compile_options(${PROJECT_PREFIX}_mocktests PRIVATE ${ICEORYX_TEST_CXX_FLAGS})
target_compile_options(${PROJECT_PREFIX}_integrationtests PRIVATE ${ICEORYX_TEST_CXX_FLAGS})
target_compile_options(test_stress_spsc_sofi PRIVATE ${ICEORYX_TEST_CXX_FLAGS})
