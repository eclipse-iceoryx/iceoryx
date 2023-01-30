# Copyright (c) 2021 Apex.AI Inc. All rights reserved.
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
project(iceoryx_component VERSION 0.8.15)

find_package(iceoryx_hoofs REQUIRED)

include(IceoryxPlatform)

set(PREFIX iceoryx/v${CMAKE_PROJECT_VERSION})

#
########## lib ##########
#
add_library(iceoryx_component
    source/example_module/example_base_class.cpp
)

add_library(iceoryx_component::iceoryx_component ALIAS iceoryx_component)

set_target_properties(iceoryx_component PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

target_include_directories(iceoryx_component
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/generated/iceoryx/include>
    $<INSTALL_INTERFACE:include/${PREFIX}>
    PRIVATE
    source/example_module
)
target_link_libraries(iceoryx_component
    PRIVATE
    ${ICEORYX_SANITIZER_FLAGS}
    ${CMAKE_THREAD_LIBS_INIT}
)

target_compile_options(iceoryx_component PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS})


#
########## test ##########
#

if(BUILD_TEST AND NOT GTest_FOUND)
    find_package(GTest CONFIG REQUIRED)
endif(BUILD_TEST AND NOT GTest_FOUND)

set(PROJECT_PREFIX "iceoryx_component")

file(GLOB_RECURSE MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/test/moduletests/*.cpp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_PREFIX}/test)

set(TEST_LINK_LIBS
    GTest::gtest
    GTest::gmock
    iceoryx_component::iceoryx_component
)

if(LINUX)
    set(TEST_LINK_LIBS ${TEST_LINK_LIBS} dl )
endif()

# unittests
add_executable(${PROJECT_PREFIX}_moduletests ${MODULETESTS_SRC})
target_include_directories(${PROJECT_PREFIX}_moduletests PRIVATE . source/example_module)
# TODO: fix conversion warnings
target_compile_options(${PROJECT_PREFIX}_moduletests PRIVATE ${TEST_CXX_FLAGS})
target_link_libraries(${PROJECT_PREFIX}_moduletests ${TEST_LINK_LIBS})
set_target_properties(${PROJECT_PREFIX}_moduletests PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)

