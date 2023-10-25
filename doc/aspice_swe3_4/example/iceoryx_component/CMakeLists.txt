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
iox_add_library(
    TARGET              iceoryx_component
    NAMESPACE           iceoryx_component
    PRIVATE_INCLUDES    source/example_module
    BUILD_INTERFACE     ${CMAKE_CURRENT_SOURCE_DIR}/include
                        ${CMAKE_BINARY_DIR}/generated/iceoryx_component/include
    INSTALL_INTERFACE   include/${PREFIX}
    PRIVATE_LIBS        ${CMAKE_THREAD_LIBS_INIT}
    FILES
        source/example_module/example_base_class.cpp
)

#
########## test ##########
#

if(BUILD_TEST AND NOT GTest_FOUND)
    find_package(GTest CONFIG REQUIRED)
endif(BUILD_TEST AND NOT GTest_FOUND)

set(PROJECT_PREFIX "iceoryx_component")

file(GLOB_RECURSE MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/test/moduletests/*.cpp")

set(TEST_LINK_LIBS
    GTest::gtest
    GTest::gmock
    iceoryx_component::iceoryx_component
)

iox_add_executable(
    TARGET                  ${PROJECT_PREFIX}_moduletests
    INCLUDE_DIRECTORIES     .
                            source/example_module
    FILES                   ${MODULETESTS_SRC}
    LIBS                    ${TEST_LINK_LIBS}
    LIBS_LINUX              dl
)
