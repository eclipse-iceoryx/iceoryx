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

set(IOX_VERSION_STRING "2.95.2")

project(iceoryx_hoofs VERSION ${IOX_VERSION_STRING})

find_package(iceoryx_platform REQUIRED)

include(IceoryxPackageHelper)
include(IceoryxPlatform)
include(IceoryxPlatformSettings)

include(cmake/IceoryxHoofsDeployment.cmake)

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
    PRIVATE_LIBS_LINUX          acl atomic ${CODE_COVERAGE_LIBS}
    BUILD_INTERFACE             ${PROJECT_SOURCE_DIR}/buffer/include
                                ${PROJECT_SOURCE_DIR}/cli/include
                                ${PROJECT_SOURCE_DIR}/concurrent/buffer/include
                                ${PROJECT_SOURCE_DIR}/concurrent/sync/include
                                ${PROJECT_SOURCE_DIR}/container/include
                                ${PROJECT_SOURCE_DIR}/design/include
                                ${PROJECT_SOURCE_DIR}/filesystem/include
                                ${PROJECT_SOURCE_DIR}/functional/include
                                ${PROJECT_SOURCE_DIR}/memory/include
                                ${PROJECT_SOURCE_DIR}/primitives/include
                                ${PROJECT_SOURCE_DIR}/reporting/include
                                ${PROJECT_SOURCE_DIR}/time/include
                                ${PROJECT_SOURCE_DIR}/utility/include
                                ${PROJECT_SOURCE_DIR}/vocabulary/include

                                ${PROJECT_SOURCE_DIR}/posix/auth/include
                                ${PROJECT_SOURCE_DIR}/posix/design/include
                                ${PROJECT_SOURCE_DIR}/posix/ipc/include
                                ${PROJECT_SOURCE_DIR}/posix/filesystem/include
                                ${PROJECT_SOURCE_DIR}/posix/sync/include
                                ${PROJECT_SOURCE_DIR}/posix/time/include
                                ${PROJECT_SOURCE_DIR}/posix/utility/include
                                ${PROJECT_SOURCE_DIR}/posix/vocabulary/include

                                ${CMAKE_BINARY_DIR}/generated/iceoryx_hoofs/include

                                ${PROJECT_SOURCE_DIR}/legacy/include
    INSTALL_INTERFACE           include/${PREFIX}
    EXPORT_INCLUDE_DIRS         buffer/include/
                                cli/include/
                                concurrent/buffer/include/
                                concurrent/sync/include/
                                container/include/
                                design/include/
                                filesystem/include/
                                functional/include/
                                memory/include/
                                primitives/include/
                                reporting/include/
                                time/include/
                                utility/include/
                                vocabulary/include/

                                posix/auth/include/
                                posix/design/include/
                                posix/ipc/include/
                                posix/filesystem/include/
                                posix/sync/include/
                                posix/time/include/
                                posix/utility/include/
                                posix/vocabulary/include/

                                legacy/include/
    FILES
        cli/source/arguments.cpp
        cli/source/command_line_parser.cpp
        cli/source/option.cpp
        cli/source/option_definition.cpp
        cli/source/option_manager.cpp
        concurrent/buffer/source/mpmc_loffli.cpp
        filesystem/source/file_reader.cpp
        filesystem/source/filesystem.cpp
        memory/source/bump_allocator.cpp
        memory/source/memory.cpp
        memory/source/relative_pointer_data.cpp
        primitives/source/type_traits.cpp
        reporting/source/default_error_handler.cpp
        reporting/source/hoofs_error_reporting.cpp
        reporting/source/console_logger.cpp
        reporting/source/logger.cpp
        reporting/source/logging.cpp
        time/source/duration.cpp
        utility/source/unique_id.cpp

        posix/auth/source/posix_group.cpp
        posix/auth/source/posix_user.cpp
        posix/design/source/file_management_interface.cpp
        posix/ipc/source/message_queue.cpp
        posix/ipc/source/named_pipe.cpp
        posix/ipc/source/posix_memory_map.cpp
        posix/ipc/source/posix_shared_memory.cpp
        posix/ipc/source/posix_shared_memory_object.cpp
        posix/ipc/source/unix_domain_socket.cpp
        posix/filesystem/source/file.cpp
        posix/filesystem/source/file_lock.cpp
        posix/filesystem/source/posix_acl.cpp
        posix/sync/source/mutex.cpp
        posix/sync/source/named_semaphore.cpp
        posix/sync/source/signal_handler.cpp
        posix/sync/source/signal_watcher.cpp
        posix/sync/source/semaphore_interface.cpp
        posix/sync/source/thread.cpp
        posix/sync/source/unnamed_semaphore.cpp
        posix/time/source/adaptive_wait.cpp
        posix/time/source/deadline_timer.cpp
        posix/utility/source/posix_scheduler.cpp
        posix/utility/source/system_configuration.cpp
        posix/vocabulary/source/file_name.cpp
        posix/vocabulary/source/file_path.cpp
        posix/vocabulary/source/group_name.cpp
        posix/vocabulary/source/path.cpp
        posix/vocabulary/source/user_name.cpp
)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/iceoryx_hoofs_deployment.hpp.in"
  "${CMAKE_BINARY_DIR}/generated/iceoryx_hoofs/include/iox/iceoryx_hoofs_deployment.hpp" @ONLY)

install(
    FILES ${CMAKE_BINARY_DIR}/generated/iceoryx_hoofs/include/iox/iceoryx_hoofs_deployment.hpp
    DESTINATION include/${PREFIX}/iox/
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
