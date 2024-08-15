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

set(IOX_VERSION_STRING "2.95.2")

project(iceoryx_introspection VERSION ${IOX_VERSION_STRING})

find_package(iceoryx_platform REQUIRED)
find_package(iceoryx_hoofs REQUIRED)
find_package(iceoryx_posh REQUIRED)

include(IceoryxPackageHelper)
include(IceoryxPlatform)
include(IceoryxPlatformSettings)

set(PREFIX iceoryx/v${CMAKE_PROJECT_VERSION})

if(CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES Darwin)
    option(BUILD_SHARED_LIBS "Create shared libraries by default" ON)
endif()

SET(CURSES_USE_NCURSES TRUE)
find_package(Curses QUIET)
if(NOT CURSES_FOUND)
    message(STATUS "The build of the introspection client is skipped due to missing the curses library")
    return()
endif()
if(${CURSES_HAVE_NCURSES_H} MATCHES ".*-NOTFOUND")
    message(SEND_ERROR "ncurses header not found! Please install libncurses5-dev!")
endif(${CURSES_HAVE_NCURSES_H} MATCHES ".*-NOTFOUND")

iox_add_library(
    TARGET                      iceoryx_introspection
    NAMESPACE                   iceoryx_introspection
    PROJECT_PREFIX              ${PREFIX}
    PUBLIC_LIBS                 iceoryx_hoofs::iceoryx_hoofs
                                iceoryx_posh::iceoryx_posh
                                ncurses
    PUBLIC_LIBS_LINUX           tinfo
    BUILD_INTERFACE             ${CMAKE_CURRENT_SOURCE_DIR}/include
    INSTALL_INTERFACE           include/${PREFIX}
    EXPORT_INCLUDE_DIRS         include/
    PRIVATE_INCLUDES            ${CURSES_INCLUDE_DIR}
    FILES
        source/iceoryx_introspection_app.cpp
        source/introspection_app.cpp
)

iox_add_executable(
    TARGET                      iox-introspection-client
    LIBS                        iceoryx_introspection::iceoryx_introspection
    FILES
        source/introspection_main.cpp
)
