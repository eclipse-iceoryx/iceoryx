# Copyright (c) 2019 - 2021 by Robert Bosch GmbH. All rights reserved.
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

set(IOX_VERSION_STRING "2.95.6")
project(iceoryx_platform VERSION ${IOX_VERSION_STRING})

if(UNIX AND NOT APPLE)
    if(CMAKE_SYSTEM_NAME MATCHES Linux)
        set(LINUX true)
    elseif(CMAKE_SYSTEM_NAME MATCHES QNX)
        set(QNX true)
    endif()
endif()

if(CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES Darwin)
    option(BUILD_SHARED_LIBS "Create shared libraries by default" ON)
endif()

set(PUBLIC_LIBS)
set(PRIVATE_LIBS)

if(IOX_PLATFORM_PATH)
    set(ICEORYX_PLATFORM ${IOX_PLATFORM_PATH})
elseif(IOX_PLATFORM_MINIMAL_POSIX)
    set(ICEORYX_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/minimal_posix/)
elseif(LINUX)
    set(ICEORYX_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/linux/)
    set(PUBLIC_LIBS atomic rt pthread)
elseif(QNX)
    set(ICEORYX_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/qnx/)
    set(PRIVATE_LIBS socket)
elseif(APPLE)
    set(ICEORYX_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/mac/)
elseif(WIN32)
    set(ICEORYX_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/win/)
elseif(UNIX)
    set(ICEORYX_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/unix/)
    set(PUBLIC_LIBS rt pthread)
elseif(FREERTOS)
    set(ICEORYX_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/freertos/)
    set(PUBLIC_LIBS freertos_sdk::FreeRTOS_POSIX)
else()
    set(ICEORYX_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/unix/)
    message(WARNING "Could not detect supported platform, but I'm feeling lucky today. Maybe its Unix." )
endif()

set(PREFIX iceoryx/v${CMAKE_PROJECT_VERSION})
include("${CMAKE_CURRENT_LIST_DIR}/cmake/IceoryxConfigureOptionMacro.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/iceoryxversions.cmake")

include("${ICEORYX_PLATFORM}/cmake/IceoryxPlatformDeployment.cmake")

include("${CMAKE_CURRENT_LIST_DIR}/cmake/IceoryxPackageHelper.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/IceoryxPlatform.cmake")
include("${ICEORYX_PLATFORM}/cmake/IceoryxPlatformSettings.cmake")

if(CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES Darwin)
    option(BUILD_SHARED_LIBS "Create shared libraries by default" ON)
endif()

set(ICEORYX_PLATFORM ${ICEORYX_PLATFORM} CACHE PATH "" FORCE)

# this must be recurse since every platform has potentially different cpp files
file ( GLOB_RECURSE ICEORYX_PLATFORM_FILES
    generic/source/*.cpp
    ${ICEORYX_PLATFORM}/source/*.cpp
)

if(IOX_PLATFORM_FEATURE_ACL)
    set(IOX_CFG_FEATURE_ACL "1")
    set(ACL_LIB acl)
else()
    set(IOX_CFG_FEATURE_ACL "0")
    set(ACL_LIB)
endif()

iox_add_library(
    TARGET                      iceoryx_platform
    NAMESPACE                   iceoryx_platform
    PROJECT_PREFIX              ${PREFIX}
    EXPORT_INCLUDE_DIRS         generic/include/
                                ${ICEORYX_PLATFORM}/include/
    PUBLIC_LIBS                 ${PUBLIC_LIBS}
                                ${ACL_LIB}
    PRIVATE_LIBS                ${PRIVATE_LIBS}
                                ${ICEORYX_SANITIZER_FLAGS}
                                ${ICEORYX_GRCOV_FLAGS}
    BUILD_INTERFACE             ${PROJECT_SOURCE_DIR}/generic/include/
                                ${ICEORYX_PLATFORM}/include/
                                ${CMAKE_BINARY_DIR}/generated/iceoryx_platform/include
    INSTALL_INTERFACE           include/${PREFIX}
    FILES                       ${ICEORYX_PLATFORM_FILES}
)

configure_file("${ICEORYX_PLATFORM}/cmake/platform_settings.hpp.in"
  "${CMAKE_BINARY_DIR}/generated/iceoryx_platform/include/iceoryx_platform/platform_settings.hpp" @ONLY)

install(
    FILES ${CMAKE_BINARY_DIR}/generated/iceoryx_platform/include/iceoryx_platform/platform_settings.hpp
    DESTINATION include/${PREFIX}/iceoryx_platform/
    COMPONENT dev
)

install(
    FILES
        cmake/IceoryxConfigureOptionMacro.cmake
        cmake/IceoryxPackageHelper.cmake
        cmake/IceoryxPlatform.cmake
        cmake/IceoryxVersion.cmake
        ${ICEORYX_PLATFORM}/cmake/IceoryxPlatformSettings.cmake
    DESTINATION ${DESTINATION_CONFIGDIR}
    COMPONENT dev
)

if(BUILD_TEST)
    add_subdirectory(test)
endif()
