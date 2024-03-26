# Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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
set(test_iceoryx_platform_VERSION 0)
project(test_iceoryx_platform VERSION ${test_iceoryx_platform_VERSION})

find_package(Threads REQUIRED)

if(BUILD_TEST AND NOT GTest_FOUND)
    find_package(GTest CONFIG REQUIRED)
endif(BUILD_TEST AND NOT GTest_FOUND)

set(PROJECT_PREFIX "platform")

file(GLOB_RECURSE MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/*.cpp")
file(GLOB_RECURSE INTEGRATIONTESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/integrationtests/*.cpp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_PREFIX}/test)

set(TEST_LINK_LIBS
    ${CODE_COVERAGE_LIBS}
    GTest::gtest
    GTest::gmock
    iceoryx_platform::iceoryx_platform
)

iox_add_executable( TARGET                  ${PROJECT_PREFIX}_moduletests
                    INCLUDE_DIRECTORIES     .
                    LIBS                    ${TEST_LINK_LIBS}
                    LIBS_LINUX              acl dl pthread rt
                    FILES                   ${MODULETESTS_SRC}
)

# not used in the platform, only added to make the scripts happy
iox_add_executable( TARGET                  ${PROJECT_PREFIX}_integrationtests
                    INCLUDE_DIRECTORIES     .
                    LIBS                    ${TEST_LINK_LIBS}
                    LIBS_LINUX              acl dl pthread rt
                    FILES                   ${INTEGRATIONTESTS_SRC}
)

target_compile_options(${PROJECT_PREFIX}_moduletests PRIVATE ${ICEORYX_TEST_CXX_FLAGS})
target_compile_options(${PROJECT_PREFIX}_integrationtests PRIVATE ${ICEORYX_TEST_CXX_FLAGS})
