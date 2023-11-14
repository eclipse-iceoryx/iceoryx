# Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
# Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
project(googletest-build CXX)

if(BUILD_TEST AND NOT GTest_DIR)

    # set download config, source and build paths
    set(DOWNLOAD_CONFIG_DIR ${CMAKE_BINARY_DIR}/dependencies/googletest/download)
    set(SOURCE_DIR ${CMAKE_BINARY_DIR}/dependencies/googletest/src)
    set(BUILD_DIR ${CMAKE_BINARY_DIR}/dependencies/googletest/build)
    set(INSTALL_DIR ${CMAKE_BINARY_DIR}/dependencies/install)

    include(ProcessorCount)
    ProcessorCount(N)

    if(NOT N EQUAL 0)
        if(((${CMAKE_VERSION} VERSION_GREATER "3.12.0") OR ${CMAKE_VERSION} VERSION_EQUAL "3.12.0"))
            set(CMAKE_BUILD_FLAGS -j ${N})
        elseif(LINUX OR QNX)
            set(CMAKE_BUILD_FLAGS -- -j ${N})
        endif()
    endif()


    set(EXTRA_CMAKE_ARGS)
    if(DEFINED CMAKE_CXX_FLAGS)
        list(APPEND EXTRA_CMAKE_ARGS -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS})
    endif()

    set(GTEST_BUILD_ARGS "-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}" "${EXTRA_CMAKE_ARGS}")
    set(GTEST_DOWNLOAD_ARGS "${DOWNLOAD_CONFIG_DIR}")

    if(WIN32)
        list(APPEND GTEST_BUILD_ARGS "-Dgtest_force_shared_crt=ON")
    endif()

    if(DEFINED CMAKE_TOOLCHAIN_FILE)
        list(APPEND EXTRA_CMAKE_ARGS "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
        list(APPEND GTEST_DOWNLOAD_ARGS "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
        set(GTest_DIR ${CMAKE_BINARY_DIR}/dependencies/install/lib/cmake/GTest)
        set(GTest_DIR ${GTest_DIR} CACHE PATH "" FORCE)
    endif()


    # Download and unpack googletest at configure time
    configure_file(googletest.cmake.in ${DOWNLOAD_CONFIG_DIR}/CMakeLists.txt)

    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" ${GTEST_DOWNLOAD_ARGS}
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${DOWNLOAD_CONFIG_DIR} )
    if(result)
        message(FATAL_ERROR "CMake step [configure download] for googletest failed: ${result}")
    endif()

    execute_process(COMMAND ${CMAKE_COMMAND} --build . ${CMAKE_BUILD_FLAGS}
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${DOWNLOAD_CONFIG_DIR} )
    if(result)
        message(FATAL_ERROR "Build step [download] for googletest failed: ${result}")
    endif()

    file(MAKE_DIRECTORY ${BUILD_DIR})

    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" ${GTEST_BUILD_ARGS} "${SOURCE_DIR}"
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${BUILD_DIR} )
    if(result)
        message(FATAL_ERROR "CMake step [configure] for googletest failed: ${result}")
    endif()

    execute_process(COMMAND ${CMAKE_COMMAND} --build . --target install ${CMAKE_BUILD_FLAGS}
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${BUILD_DIR} )
    if(result)
        message(FATAL_ERROR "Build step [build and install] for googletest failed: ${result}")
    endif()

    list(APPEND CMAKE_PREFIX_PATH ${INSTALL_DIR})
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} CACHE INTERNAL "" FORCE)

endif()
