# Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

set(IOX_VERSION_STRING "2.90.0")

project(iceoryx_binding_c VERSION ${IOX_VERSION_STRING})

find_package(iceoryx_hoofs REQUIRED)
find_package(iceoryx_posh REQUIRED)

include(IceoryxPackageHelper)
include(IceoryxPlatform)
include(IceoryxPoshDeployment)

if(CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES Darwin)
    option(BUILD_SHARED_LIBS "Create shared libraries by default" ON)
endif()

option(MAKE_UNIQUE_INCLUDEDIR
  "When ON headers are installed to a path ending with folders called \
  iceoryx/vX.Y.Z/ . This avoids include directory search order issues when \
  overriding this package from a merged catkin, ament, or colcon workspace."
  ON)

if(MAKE_UNIQUE_INCLUDEDIR)
  set(_unique_dir "iceoryx/v${IOX_VERSION_STRING}")
  if(PREFIX STREQUAL "")
    set(PREFIX "${_unique_dir}")
  else()
    set(PREFIX "${PREFIX}/${_unique_dir}")
  endif()
endif()

#
########## set variables for export ##########
#
setup_package_name_and_create_files(
    NAME ${PROJECT_NAME}
    NAMESPACE iceoryx_binding_c
    PROJECT_PREFIX ${PREFIX}
)

#
########## find_package in source tree ##########
#
set(${PROJECT_NAME}_DIR ${CMAKE_CURRENT_LIST_DIR}/cmake
    CACHE FILEPATH
    "${PROJECT_NAME}Config.cmake to make find_package(${PROJECT_NAME}) work in source tree!"
    FORCE
)

#
########## build building-block library ##########
#
iox_add_library(
    TARGET              ${PROJECT_NAME}
    NAMESPACE           ${PROJECT_NAMESPACE}
    BUILD_INTERFACE     ${CMAKE_CURRENT_SOURCE_DIR}/include
                        ${CMAKE_BINARY_DIR}/generated/iceoryx/include
    INSTALL_INTERFACE   include/${PREFIX}
    PUBLIC_LIBS_LINUX   stdc++ pthread
    PUBLIC_LIBS_UNIX    stdc++ pthread
    PUBLIC_LIBS_APPLE   stdc++ pthread
    PRIVATE_LIBS        iceoryx_posh::iceoryx_posh iceoryx_hoofs::iceoryx_hoofs
    FILES
        source/c_client.cpp
        source/c_config.cpp
        source/c_notification_info.cpp
        source/c_listener.cpp
        source/c_node.cpp
        source/c_publisher.cpp
        source/c_request_header.cpp
        source/c_response_header.cpp
        source/c_runtime.cpp
        source/c_server.cpp
        source/c_subscriber.cpp
        source/c_user_trigger.cpp
        source/c_wait_set.cpp
        source/c_chunk.cpp
        source/c2cpp_enum_translation.cpp
        source/c_log.cpp
        source/cpp2c_enum_translation.cpp
        source/cpp2c_publisher.cpp
        source/cpp2c_subscriber.cpp
        source/cpp2c_service_description_translation.cpp
        source/c_service_discovery.cpp
        source/error_handling.cpp
)

#
########## build test executables ##########
#
if(BUILD_TEST)
    add_subdirectory(test)
endif()

#
########## export library ##########
#
setup_install_directories_and_export_package(
    TARGETS ${PROJECT_NAME}
    INCLUDE_DIRECTORY include/
)

install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
  DESTINATION share/doc/${PROJECT_NAME}
  COMPONENT dev)
