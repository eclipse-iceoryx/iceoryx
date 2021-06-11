# Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
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

cmake_minimum_required(VERSION 3.5)

project(tiny-json-build C)

include(ExternalProject)
include(ProcessorCount)
ProcessorCount(N)

if(NOT N EQUAL 0)
        if(((${CMAKE_VERSION} VERSION_GREATER "3.12.0") OR ${CMAKE_VERSION} VERSION_EQUAL "3.12.0"))
            set(CMAKE_BUILD_FLAGS -j ${N})
        elseif(LINUX OR QNX)
            set(CMAKE_BUILD_FLAGS -- -j ${N})
        endif()
endif()
# set download confi, source and build paths
set(DEPENDENCY_BASE_DIR ${CMAKE_BINARY_DIR}/dependencies/tiny_json)
set(DOWNLOAD_CONFIG_DIR ${DEPENDENCY_BASE_DIR}/download)
set(SOURCE_DIR ${DEPENDENCY_BASE_DIR}/src)
set(BUILD_DIR ${DEPENDENCY_BASE_DIR}/build)
set(INSTALL_DIR ${CMAKE_BINARY_DIR}/dependencies/install)

set(ADDITIONAL_CMAKE_FLAGS "")
foreach(flag IN LISTS ARGN)
        list(APPEND ADDITIONAL_CMAKE_FLAGS ${flag})
endforeach()
string( REPLACE ";" " " ADDITIONAL_CMAKE_FLAGS "${ADDITIONAL_CMAKE_FLAGS}")

if(DEFINED CMAKE_TOOLCHAIN_FILE)
    set(TOOLCHAIN_FILE "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
endif()

if(WIN32)
    set(CREATE_PATH_COMMAND mkdir)
else()
    set(CREATE_PATH_COMMAND mkdir -p)
endif(WIN32)

configure_file(tiny_json_build.cmake.in ${DOWNLOAD_CONFIG_DIR}/CMakeLists.txt)
configure_file(CMakeLists.txt.in ${DEPENDENCY_BASE_DIR}/CMakeLists.txt @ONLY)
configure_file(tiny_jsonConfig.cmake.in ${DEPENDENCY_BASE_DIR}/tiny_jsonConfig.cmake @ONLY)

execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" "${TOOLCHAIN_FILE}" "${DOWNLOAD_CONFIG_DIR}"
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${DOWNLOAD_CONFIG_DIR} )
if(result)
        message(FATAL_ERROR "CMake step [configure download] for ${name} failed: ${result}")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} --build . 
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${DOWNLOAD_CONFIG_DIR} )
if(result)
        message(FATAL_ERROR "Build step [build and install] for ${name} failed: ${result}")
endif()

list(APPEND CMAKE_PREFIX_PATH ${INSTALL_DIR})
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} CACHE INTERNAL "" FORCE)
