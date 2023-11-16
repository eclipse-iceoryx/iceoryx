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
set(test_iceoryx_posh_VERSION 0)
project(test_iceoryx_posh VERSION ${test_iceoryx_posh_VERSION})

find_package(iceoryx_hoofs_testing REQUIRED)
find_package(iceoryx_posh_testing REQUIRED)

set(ICEORYX_POSH_TEST_STACK_SIZE 5000000)

if(BUILD_TEST AND NOT GTest_FOUND)
    find_package(GTest CONFIG REQUIRED)
endif(BUILD_TEST AND NOT GTest_FOUND)

set(PROJECT_PREFIX "posh")

file(GLOB_RECURSE MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/*.cpp")
file(GLOB_RECURSE INTEGRATIONTESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/integrationtests/*.cpp")
file(GLOB_RECURSE COMPONENTTESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/componenttests/*.cpp")
file(GLOB_RECURSE MOCKS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/mocks/*.cpp")


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_PREFIX}/test)

set(TEST_LINK_LIBS
    ${CODE_COVERAGE_LIBS}
    GTest::gtest
    GTest::gmock
    iceoryx_platform::iceoryx_platform
    iceoryx_hoofs::iceoryx_hoofs
    iceoryx_hoofs_testing::iceoryx_hoofs_testing
    iceoryx_posh::iceoryx_posh
    iceoryx_posh::iceoryx_posh_config
    iceoryx_posh::iceoryx_posh_gateway
    iceoryx_posh::iceoryx_posh_roudi
    iceoryx_posh_testing::iceoryx_posh_testing
)

# unittests
iox_add_executable( TARGET                  ${PROJECT_PREFIX}_moduletests
                    INCLUDE_DIRECTORIES     .
                                            ${CMAKE_BINARY_DIR}/generated
                    LIBS                    ${TEST_LINK_LIBS}
                    LIBS_LINUX              dl stdc++fs
                    STACK_SIZE              ${ICEORYX_POSH_TEST_STACK_SIZE}
                    FILES
                        ${MODULETESTS_SRC}
                        ${MOCKS_SRC}
    )

# integration tests
iox_add_executable( TARGET                  ${PROJECT_PREFIX}_integrationtests
                    INCLUDE_DIRECTORIES     .
                    LIBS                    ${TEST_LINK_LIBS}
                    LIBS_LINUX              dl
                    STACK_SIZE              ${ICEORYX_POSH_TEST_STACK_SIZE}
                    FILES
                        ${INTEGRATIONTESTS_SRC}
                        ${TESTUTILS_SRC}
    )

target_compile_options(${PROJECT_PREFIX}_moduletests PRIVATE ${ICEORYX_TEST_CXX_FLAGS})
target_compile_options(${PROJECT_PREFIX}_integrationtests PRIVATE ${ICEORYX_TEST_CXX_FLAGS})
