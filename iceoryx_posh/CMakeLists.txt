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

set(IOX_VERSION_STRING "2.95.2")

project(iceoryx_posh VERSION ${IOX_VERSION_STRING})

find_package(Threads REQUIRED)
find_package(iceoryx_platform REQUIRED)
find_package(iceoryx_hoofs REQUIRED)

option(DOWNLOAD_TOML_LIB "Download cpptoml via the CMake ExternalProject module" ON)
option(TOML_CONFIG "TOML support for RouDi with dynamic configuration" ON)
option(ONE_TO_MANY_ONLY "Restricts communication to 1:n pattern" OFF)

if(TOML_CONFIG)
    if (DOWNLOAD_TOML_LIB)
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cpptoml/ ${CMAKE_BINARY_DIR}/dependencies/posh/cpptoml/prebuild)
    endif()

    find_package(cpptoml REQUIRED)
endif()

include(IceoryxPackageHelper)
include(IceoryxPlatform)
include(IceoryxPlatformSettings)

include(cmake/IceoryxPoshDeployment.cmake)

if(CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES Darwin)
    option(BUILD_SHARED_LIBS "Create shared libraries by default" ON)
endif()

set(PREFIX iceoryx/v${CMAKE_PROJECT_VERSION})

########## Multiple libraries are exported, the setup and installation is done
#          manually without the iox_* macros
#
########## set variables for library export ##########
#
setup_package_name_and_create_files(
    NAME ${PROJECT_NAME}
    NAMESPACE iceoryx_posh
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
########## posh core lib ##########
#
iox_add_library(
    NO_EXPORT
    NO_PACKAGE_SETUP
    NO_FIND_PACKAGE_SUPPORT
    TARGET                  iceoryx_posh    # @todo iox-#590 Rename this to iceoryx_posh_core
    NAMESPACE               iceoryx_posh    # @todo iox-#590 Name of iceoryx_posh should stay to be backwards compatible
                                            # and include both iceoryx_posh_core and iceoryx_posh_runtime
    PROJECT_PREFIX          ${PREFIX}
    BUILD_INTERFACE         ${CMAKE_CURRENT_SOURCE_DIR}/include
                            ${CMAKE_CURRENT_SOURCE_DIR}/experimental/include
                            ${CMAKE_BINARY_DIR}/generated/iceoryx_posh/include
    INSTALL_INTERFACE       include/${PREFIX}
    PRIVATE_INCLUDES        source/capro
                            source/log
                            source/mepoo
                            source/popo
                            source/version
                            source/runtime
    PUBLIC_LIBS             iceoryx_hoofs::iceoryx_hoofs
    PRIVATE_LIBS            ${CMAKE_THREAD_LIBS_INIT}
    PRIVATE_LIBS_LINUX      rt
    FILES
        source/capro/capro_message.cpp
        source/capro/service_description.cpp
        source/iceoryx_posh_types.cpp
        source/mepoo/chunk_header.cpp
        source/mepoo/chunk_management.cpp
        source/mepoo/chunk_settings.cpp
        source/mepoo/mepoo_config.cpp
        source/mepoo/segment_config.cpp
        source/mepoo/memory_manager.cpp
        source/mepoo/mem_pool.cpp
        source/mepoo/shared_chunk.cpp
        source/mepoo/shm_safe_unmanaged_chunk.cpp
        source/mepoo/segment_manager.cpp
        source/mepoo/mepoo_segment.cpp
        source/mepoo/memory_info.cpp
        source/popo/ports/interface_port.cpp
        source/popo/ports/interface_port_data.cpp
        source/popo/ports/base_port_data.cpp
        source/popo/ports/base_port.cpp
        source/popo/ports/publisher_port_data.cpp
        source/popo/ports/publisher_port_user.cpp
        source/popo/ports/publisher_port_roudi.cpp
        source/popo/ports/subscriber_port_user.cpp
        source/popo/ports/subscriber_port_roudi.cpp
        source/popo/ports/subscriber_port_single_producer.cpp
        source/popo/ports/subscriber_port_multi_producer.cpp
        source/popo/ports/subscriber_port_data.cpp
        source/popo/ports/client_port_data.cpp
        source/popo/ports/client_port_roudi.cpp
        source/popo/ports/client_port_user.cpp
        source/popo/ports/server_port_data.cpp
        source/popo/ports/server_port_roudi.cpp
        source/popo/ports/server_port_user.cpp
        source/popo/building_blocks/condition_listener.cpp
        source/popo/building_blocks/condition_notifier.cpp
        source/popo/building_blocks/condition_variable_data.cpp
        source/popo/building_blocks/locking_policy.cpp
        source/popo/building_blocks/unique_port_id.cpp
        source/popo/client_options.cpp
        source/popo/listener.cpp
        source/popo/notification_info.cpp
        source/popo/rpc_header.cpp
        source/popo/publisher_options.cpp
        source/popo/server_options.cpp
        source/popo/subscriber_options.cpp
        source/popo/trigger.cpp
        source/popo/trigger_handle.cpp
        source/popo/user_trigger.cpp
        source/posh_error_reporting.cpp
        source/version/version_info.cpp
        source/runtime/heartbeat.cpp
        source/runtime/ipc_interface_base.cpp
        source/runtime/ipc_interface_user.cpp
        source/runtime/ipc_interface_creator.cpp
        source/runtime/ipc_runtime_interface.cpp
        source/runtime/ipc_message.cpp
        source/runtime/port_config_info.cpp
        source/runtime/posh_runtime.cpp                #
        source/runtime/posh_runtime_impl.cpp           # @todo iox-#590 These files should go into a separate library iceoryx_posh_runtime
        source/runtime/posh_runtime_single_process.cpp #
        source/runtime/service_discovery.cpp           #
        source/runtime/shared_memory_user.cpp
        source/roudi/service_registry.cpp              # @todo iox-#415 Move the service registry into runtime namespace?

        experimental/source/node.cpp
)

#
########## posh lib for gateway support ##########
#
iox_add_library(
    NO_EXPORT
    NO_PACKAGE_SETUP
    NO_FIND_PACKAGE_SUPPORT
    TARGET              iceoryx_posh_gateway
    NAMESPACE           iceoryx_posh
    PROJECT_PREFIX      ${PREFIX}
    BUILD_INTERFACE     ${CMAKE_CURRENT_SOURCE_DIR}/include
                        ${CMAKE_BINARY_DIR}/generated/iceoryx_posh/include
                        ${CMAKE_BINARY_DIR}/dependencies/install/include
    INSTALL_INTERFACE   include/${PREFIX}
    PUBLIC_LIBS         iceoryx_hoofs::iceoryx_hoofs
                        iceoryx_posh::iceoryx_posh
    FILES
        source/gateway/gateway_base.cpp
)

#
########## posh roudi lib ##########
#
iox_add_library(
    NO_EXPORT
    NO_PACKAGE_SETUP
    NO_FIND_PACKAGE_SUPPORT
    TARGET              iceoryx_posh_roudi
    NAMESPACE           iceoryx_posh
    PROJECT_PREFIX      ${PREFIX}
    PUBLIC_LIBS         iceoryx_posh::iceoryx_posh # @todo iox-#590 Only link against iceoryx_posh_core
    PRIVATE_LIBS        iceoryx_hoofs::iceoryx_hoofs
    PRIVATE_INCLUDES    source/runtime
                        source/version
                        source/popo
                        source/capro
                        source/roudi
    PRIVATE_LIBS_QNX    socket
    BUILD_INTERFACE     ${CMAKE_CURRENT_SOURCE_DIR}/include
    INSTALL_INTERFACE   include/${PREFIX}
    FILES
        source/roudi/application/iceoryx_roudi_app.cpp
        source/roudi/application/roudi_app.cpp
        source/roudi/memory/memory_block.cpp
        source/roudi/memory/memory_provider.cpp
        source/roudi/memory/mempool_collection_memory_block.cpp
        source/roudi/memory/mempool_segment_manager_memory_block.cpp
        source/roudi/memory/port_pool_memory_block.cpp
        source/roudi/memory/posix_shm_memory_provider.cpp
        source/roudi/memory/default_roudi_memory.cpp
        source/roudi/memory/roudi_memory_manager.cpp
        source/roudi/memory/iceoryx_roudi_memory_manager.cpp
        source/roudi/port_manager.cpp
        source/roudi/port_pool.cpp
        source/roudi/roudi.cpp
        source/roudi/process.cpp
        source/roudi/process_manager.cpp
        source/roudi/iceoryx_roudi_components.cpp
        source/roudi/roudi_cmd_line_parser.cpp
        source/roudi/roudi_cmd_line_parser_config_file_option.cpp
        source/roudi/roudi_config.cpp
)

if(TOML_CONFIG)
    #
    ######### posh config lib ##########
    #
    iox_add_library(
        NO_EXPORT
        NO_PACKAGE_SETUP
        NO_FIND_PACKAGE_SUPPORT
        TARGET              iceoryx_posh_config
        NAMESPACE           iceoryx_posh
        PROJECT_PREFIX      ${PREFIX}
        PRIVATE_LIBS        iceoryx_posh::iceoryx_posh
                            iceoryx_hoofs::iceoryx_hoofs
                            iceoryx_posh::iceoryx_posh_roudi
                            cpptoml
        BUILD_INTERFACE     ${CMAKE_CURRENT_SOURCE_DIR}/include
                            ${CMAKE_BINARY_DIR}/generated/iceoryx_posh/include
                            ${CMAKE_BINARY_DIR}/dependencies/install/include
        INSTALL_INTERFACE   include/${PREFIX}
        FILES
            source/gateway/gateway_config.cpp
            source/gateway/toml_gateway_config_parser.cpp
            source/roudi/roudi_config_toml_file_provider.cpp
    )

    install(
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/etc/iceoryx/roudi_config_example.toml
        DESTINATION etc/
        COMPONENT dev)

    #
    ######### posh roudi daemon ##########
    #
    iox_add_executable(
        PLACE_IN_BUILD_ROOT
        TARGET              iox-roudi
        LIBS                iceoryx_hoofs::iceoryx_hoofs
                            iceoryx_posh::iceoryx_posh_roudi
                            iceoryx_posh::iceoryx_posh_config
        BUILD_INTERFACE     ${CMAKE_CURRENT_SOURCE_DIR}/include
                            ${CMAKE_BINARY_DIR}/dependencies/install/include
        INSTALL_INTERFACE   include/${PREFIX}
        FILES
            source/roudi/application/roudi_main.cpp
        )
endif()

#
########## exporting library ##########
#
if(TOML_CONFIG)
    set(ROUDI_EXPORT iceoryx_posh_config iox-roudi)
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/iceoryx_posh_deployment.hpp.in"
  "${CMAKE_BINARY_DIR}/generated/iceoryx_posh/include/iceoryx_posh/iceoryx_posh_deployment.hpp" @ONLY)

install(
    FILES ${CMAKE_BINARY_DIR}/generated/iceoryx_posh/include/${PROJECT_NAME}/iceoryx_posh_deployment.hpp
    DESTINATION include/${PREFIX}/${PROJECT_NAME}/
    COMPONENT dev
)

# install deployment file to make posh config accessible by other packages
install(
    FILES
        cmake/IceoryxPoshDeployment.cmake
    DESTINATION ${DESTINATION_CONFIGDIR}
)

if(IOX_EXPERIMENTAL_POSH)
    set(IOX_EXPERIMENTAL_INCLUDE_EXPORT experimental/include/)
endif()

setup_install_directories_and_export_package(
    TARGETS iceoryx_posh iceoryx_posh_roudi iceoryx_posh_gateway ${ROUDI_EXPORT}
    INCLUDE_DIRECTORIES include/ ${IOX_EXPERIMENTAL_INCLUDE_EXPORT}
)


if(ROUDI_ENVIRONMENT OR BUILD_TEST)
    add_subdirectory(roudi_env)
endif()

if(BUILD_TEST)
    add_subdirectory(testing)
    add_subdirectory(test)
endif()
