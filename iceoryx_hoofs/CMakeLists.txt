# Copyright (c) 2019 - 2021 by Robert Bosch GmbH. All rights reserved.
# Copyright (c) 2020 - 2023 by Apex.AI Inc. All rights reserved.
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

project(iceoryx_hoofs VERSION ${IOX_VERSION_STRING})

find_package(iceoryx_platform REQUIRED)

include(IceoryxPackageHelper)
include(IceoryxPlatformSettings)

include(cmake/IceoryxHoofsDeployment.cmake)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/IceoryxPlatform.cmake")

if(CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES Darwin)
    option(BUILD_SHARED_LIBS "Create shared libraries by default" ON)
endif()

set(PREFIX iceoryx/v${CMAKE_PROJECT_VERSION})

#
########## build iceoryx hoofs lib ##########
#
iox_add_library(
    TARGET                      iceoryx_hoofs
    NAMESPACE                   iceoryx_hoofs
    PROJECT_PREFIX              ${PREFIX}
    PUBLIC_LIBS                 iceoryx_platform::iceoryx_platform
    PRIVATE_LIBS                ${ICEORYX_SANITIZER_FLAGS}
    PRIVATE_LIBS_LINUX          acl atomic ${CODE_COVERAGE_LIBS}
    BUILD_INTERFACE             ${PROJECT_SOURCE_DIR}/include
                                ${PROJECT_SOURCE_DIR}/legacy/include
                                ${PROJECT_SOURCE_DIR}/memory/include
                                ${PROJECT_SOURCE_DIR}/container/include
                                ${PROJECT_SOURCE_DIR}/vocabulary/include
                                ${PROJECT_SOURCE_DIR}/utility/include
                                ${PROJECT_SOURCE_DIR}/primitives/include
                                ${PROJECT_SOURCE_DIR}/design/include
                                ${PROJECT_SOURCE_DIR}/time/include
                                ${PROJECT_SOURCE_DIR}/buffer/include
                                ${PROJECT_SOURCE_DIR}/filesystem/include
                                ${PROJECT_SOURCE_DIR}/functional/include
                                ${PROJECT_SOURCE_DIR}/reporting/include

                                ${PROJECT_SOURCE_DIR}/posix/design/include
                                ${PROJECT_SOURCE_DIR}/posix/filesystem/include
                                ${PROJECT_SOURCE_DIR}/posix/time/include
                                ${PROJECT_SOURCE_DIR}/posix/vocabulary/include

                                ${CMAKE_BINARY_DIR}/generated/iceoryx_hoofs/include
    INSTALL_INTERFACE           include/${PREFIX}
    EXPORT_INCLUDE_DIRS         include/
                                legacy/include/
                                memory/include/
                                container/include/
                                vocabulary/include/
                                utility/include/
                                primitives/include/
                                design/include/
                                time/include/
                                buffer/include/
                                filesystem/include/
                                functional/include/
                                reporting/include/

                                posix/design/include/
                                posix/filesystem/include/
                                posix/time/include/
                                posix/vocabulary/include/
    FILES
        design/source/functional_interface.cpp
        filesystem/source/filesystem.cpp
        memory/source/bump_allocator.cpp
        memory/source/memory.cpp
        memory/source/relative_pointer_data.cpp
        primitives/source/type_traits.cpp
        reporting/source/log/building_blocks/console_logger.cpp
        reporting/source/log/building_blocks/logger.cpp
        source/concurrent/loffli.cpp
        source/cxx/requires.cpp
        source/error_handling/error_handler.cpp
        source/error_handling/error_handling.cpp
        source/posix_wrapper/access_control.cpp
        source/posix_wrapper/file_lock.cpp
        source/posix_wrapper/mutex.cpp
        source/posix_wrapper/named_semaphore.cpp
        source/posix_wrapper/posix_access_rights.cpp
        source/posix_wrapper/scheduler.cpp
        source/posix_wrapper/semaphore_interface.cpp
        source/posix_wrapper/shared_memory_object.cpp
        source/posix_wrapper/shared_memory_object/memory_map.cpp
        source/posix_wrapper/shared_memory_object/shared_memory.cpp
        source/posix_wrapper/signal_handler.cpp
        source/posix_wrapper/system_configuration.cpp
        source/posix_wrapper/thread.cpp
        source/posix_wrapper/types.cpp
        source/posix_wrapper/unnamed_semaphore.cpp
        source/posix_wrapper/unix_domain_socket.cpp
        source/error_reporting/custom/default/default_error_handler.cpp
        time/source/duration.cpp
        utility/source/unique_id.cpp

        posix/design/source/file_management_interface.cpp
        posix/filesystem/source/file.cpp
        posix/time/source/adaptive_wait.cpp
        posix/time/source/deadline_timer.cpp
        posix/vocabulary/source/file_name.cpp
        posix/vocabulary/source/file_path.cpp
        posix/vocabulary/source/group_name.cpp
        posix/vocabulary/source/path.cpp
        posix/vocabulary/source/user_name.cpp
)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/iceoryx_hoofs_deployment.hpp.in"
  "${CMAKE_BINARY_DIR}/generated/iceoryx_hoofs/include/iceoryx_hoofs/iceoryx_hoofs_deployment.hpp" @ONLY)

install(
    FILES ${CMAKE_BINARY_DIR}/generated/iceoryx_hoofs/include/${PROJECT_NAME}/iceoryx_hoofs_deployment.hpp
    DESTINATION include/${PREFIX}/${PROJECT_NAME}/
    COMPONENT dev
)

#
########## hoofs testing ##########
#

# Finding gtest and adding the subdirectories is split to support the use case of
# building the testing lib without the tests by providing gtest externally
if(NOT GTest_FOUND AND BUILD_TEST)
    find_package(GTest CONFIG REQUIRED)
endif()

if(GTest_FOUND)
    message(STATUS "GTest was found, building iceoryx_hoofs_testing enabled.")
    add_subdirectory(testing)
    if(BUILD_TEST)
        add_subdirectory(test)
    endif()
endif()

install(
    FILES
        cmake/IceoryxPlatform.cmake
        cmake/IceoryxVersion.cmake
    DESTINATION ${DESTINATION_CONFIGDIR}
)
