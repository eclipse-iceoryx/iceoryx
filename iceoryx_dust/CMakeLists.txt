# Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

set(IOX_VERSION_STRING "2.90.0")

project(iceoryx_dust VERSION ${IOX_VERSION_STRING})

find_package(iceoryx_platform REQUIRED)
find_package(iceoryx_hoofs REQUIRED)

include(IceoryxPackageHelper)
include(IceoryxPlatform)
include(IceoryxPlatformSettings)

if(CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES Darwin)
    option(BUILD_SHARED_LIBS "Create shared libraries by default" ON)
endif()

set(PREFIX iceoryx/v${CMAKE_PROJECT_VERSION})

#
########## build iceoryx dust lib ##########
#

iox_add_library(
    TARGET                      iceoryx_dust
    NAMESPACE                   iceoryx_dust
    PROJECT_PREFIX              ${PREFIX}
    PRIVATE_LIBS                ${ICEORYX_SANITIZER_FLAGS}
    PRIVATE_LIBS_LINUX          ${CODE_COVERAGE_LIBS}
    PUBLIC_LIBS                 iceoryx_hoofs::iceoryx_hoofs
    BUILD_INTERFACE             ${PROJECT_SOURCE_DIR}/include
    INSTALL_INTERFACE           include/${PREFIX}
    EXPORT_INCLUDE_DIRS         include/
    FILES
        source/cli/arguments.cpp
        source/cli/command_line_parser.cpp
        source/cli/option.cpp
        source/cli/option_definition.cpp
        source/cli/option_manager.cpp
        source/cxx/file_reader.cpp
        source/posix_wrapper/named_pipe.cpp
        source/posix_wrapper/signal_watcher.cpp
        source/posix_wrapper/message_queue.cpp
)

#
########## dust testing ##########
#

# Finding gtest and adding the subdirectories is split to support the use case of
# building the testing lib without the tests by providing gtest externally
if(NOT GTest_FOUND AND BUILD_TEST)
    find_package(GTest CONFIG REQUIRED)
endif()

if(GTest_FOUND)
    if(BUILD_TEST)
        add_subdirectory(test)
    endif()
endif()
