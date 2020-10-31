# Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

cmake_minimum_required(VERSION 3.5)
file (STRINGS "../VERSION" iceoryx_VERSION)
project(iceoryx_meta VERSION ${iceoryx_VERSION})

option(TOML_CONFIG "activates or deactivates TOML support - without TOML RouDi will not be build" ON)
option(ONE_TO_MANY_ONLY "Restricts communication to 1:n pattern" OFF)
option(BUILD_STRICT "Build is performed with '-Werror'" OFF)
option(test "Build tests" ON)
option(coverage "Build iceoryx with gcov" OFF)
option(examples "build with iceoryx examples" ON)
option(introspection "builds the introspection client which requires the ncurses library with an activated terminfo feature" OFF)
option(dds_gateway "builds the iceoryx dds gateway - enables internode communication via dds" OFF)
option(binding_c "builds the C language bindings" ON)

# ===== Dependencies
if(TOML_CONFIG)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/cpptoml/ ${CMAKE_BINARY_DIR}/dependencies/cpptoml/prebuild)
endif(TOML_CONFIG)

if(coverage)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(test ON)
        set(examples OFF)
        set(COVERAGE_FLAGS "-fprofile-arcs -ftest-coverage" CACHE INTERNAL "")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COVERAGE_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_FLAGS}")
    else(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message( FATAL_ERROR "You need to run gcov with gcc compiler." )
    endif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
endif(coverage)


if(test)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/googletest ${CMAKE_BINARY_DIR}/dependencies/googletest/prebuild)
endif(test)

# ===== Core
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_utils ${CMAKE_BINARY_DIR}/utils)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_posh ${CMAKE_BINARY_DIR}/posh)

# ===== Extras
if(introspection)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../tools/introspection ${CMAKE_BINARY_DIR}/iceoryx_introspection)
endif(introspection)

# ===== Gateways
if(dds_gateway)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/cyclonedds ${CMAKE_BINARY_DIR}/dependencies/cyclonedds/prebuild)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_dds ${CMAKE_BINARY_DIR}/iceoryx_dds)
endif(dds_gateway)

# ===== Language binding for C
if(binding_c)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_binding_c ${CMAKE_BINARY_DIR}/iceoryx_binding_c)
endif(binding_c)

# ===== Examples
if(examples)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/icedelivery ${CMAKE_BINARY_DIR}/iceoryx_examples/icedelivery)
    if(binding_c)
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/icedelivery_on_c ${CMAKE_BINARY_DIR}/iceoryx_examples/icedelivery_on_c)
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/icecallback_on_c ${CMAKE_BINARY_DIR}/iceoryx_examples/icecallback_on_c)
    endif(binding_c)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/iceperf ${CMAKE_BINARY_DIR}/iceoryx_examples/iceperf)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/singleprocess ${CMAKE_BINARY_DIR}/iceoryx_examples/singleprocess)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/benchmark_optional_and_expected ${CMAKE_BINARY_DIR}/iceoryx_examples/benchmark_optional_and_expected)
endif(examples)

if (test)
    add_custom_target( tests
        COMMAND ./posh/test/posh_moduletests --gtest_filter=-*.TimingTest_*
        COMMAND ./posh/test/posh_integrationtests --gtest_filter=-*.TimingTest_*
        COMMAND ./posh/test/posh_componenttests --gtest_filter=-*.TimingTest_*
        COMMAND ./utils/test/utils_moduletests --gtest_filter=-*.TimingTest_*
        COMMAND ./utils/test/utils_integrationtests --gtest_filter=-*.TimingTest_*
        COMMAND ./utils/test/utils_componenttests --gtest_filter=-*.TimingTest_*
        if (binding_c)
            COMMAND ./binding_c/test/binding_c_integrationtests --gtest_filter=-*.TimingTest_*
        endif(binding_c)
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )

    add_custom_target( timing_tests
        COMMAND ./posh/test/posh_moduletests --gtest_filter=*.TimingTest_*
        COMMAND ./posh/test/posh_integrationtests --gtest_filter=*.TimingTest_*
        COMMAND ./posh/test/posh_componenttests --gtest_filter=*.TimingTest_*
        COMMAND ./utils/test/utils_moduletests --gtest_filter=*.TimingTest_*
        COMMAND ./utils/test/utils_integrationtests --gtest_filter=*.TimingTest_*
        COMMAND ./utils/test/utils_componenttests --gtest_filter=*.TimingTest_*
        if (binding_c)
            COMMAND ./binding_c/test/binding_c_integrationtests --gtest_filter=*.TimingTest_*
        endif(binding_c)
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )
endif(test)


message("")
message("       Configured Options")
message("          BUILD_STRICT..............: " ${BUILD_STRICT})
message("          TOML_CONFIG...............: " ${TOML_CONFIG})
message("          binding_c.................: " ${binding_c})
message("          dds_gateway...............: " ${dds_gateway})
message("          examples..................: " ${examples})
message("          introspection.............: " ${introspection})
message("          test......................: " ${test})

message("")
message("       Build Properties")
message("          project name..............: " ${CMAKE_PROJECT_NAME})
message("          c compiler................: " ${CMAKE_C_COMPILER})
message("          c++ compiler..............: " ${CMAKE_CXX_COMPILER})
message("")

