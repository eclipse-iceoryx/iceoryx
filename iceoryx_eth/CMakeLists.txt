# Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

cmake_minimum_required(VERSION 3.7)

file (STRINGS "../VERSION" iceoryx_dds_VERSION)
project(iceoryx_dds VERSION ${iceoryx_dds_VERSION})

option(test "build test binaries")

find_package(cpptoml REQUIRED)
find_package(iceoryx_posh REQUIRED)
find_package(iceoryx_utils REQUIRED)

include(IceoryxPackageHelper)
include(IceoryxPlatformDetection)

#
########## set variables for export ##########
#
setup_package_name_and_create_files(
    NAME ${PROJECT_NAME}
    NAMESPACE iceoryx_dds
    PROJECT_PREFIX ${PREFIX}
)

#
########## feature flags ##########
#
option(USE_CYCLONE_DDS "Bind to CycloneDDS implementation" on)


#
########## build building-block library ##########
#


###################################################################################
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/iceoryx_eth
)

add_library(iceoryx_eth
    STATIC
    source/iceoryx_eth/log/logging.cpp
)

target_compile_options(iceoryx_eth PUBLIC -std=c++11)

target_include_directories(iceoryx_eth
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/dependencies/install/include>
    $<INSTALL_INTERFACE:include/${PREFIX}>
)

target_link_libraries(iceoryx_eth
    PRIVATE
    iceoryx_posh::iceoryx_posh
    iceoryx_utils::iceoryx_utils
    iceoryx_posh::iceoryx_posh_gateway
    iceoryx_posh::iceoryx_posh_config
)

################################################


if(USE_CYCLONE_DDS)
    target_sources(iceoryx_eth
        PRIVATE
        source/iceoryx_eth/eth/eth_context.cpp
        source/iceoryx_eth/eth/eth_writer.cpp
        source/iceoryx_eth/eth/eth_reader.cpp
        source/iceoryx_eth/eth/eth_gatewayconf.cpp
    )
    target_compile_definitions(iceoryx_eth PUBLIC -DUSE_CYCLONE_DDS)
    target_include_directories(iceoryx_eth
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/iceoryx_dds_messages>
    )
  #  target_link_libraries(iceoryx_eth
  #      PUBLIC
        #CycloneDDS_CXX_API::ddscxx
        #${IDLPP_GENERATE_DIR}/libiceoryx_dds_messages.a
  #  )
endif()

################################################

#target_compile_options(iceoryx_dds PRIVATE ${ICEORYX_WARNINGS})

#
########## build gateway apps ##########
#
add_executable(iox-gw-iceoryx2eth
    source/iceoryx2eth_app/main.cpp
)

target_link_libraries(iox-gw-iceoryx2eth
    PRIVATE
    iceoryx_posh::iceoryx_posh
    iceoryx_posh::iceoryx_posh_config
    iceoryx_posh::iceoryx_posh_gateway
    iceoryx_eth
)

target_compile_options(iox-gw-iceoryx2eth PRIVATE ${ICEORYX_WARNINGS})



add_executable(iox-gw-eth2iceoryx
    source/eth2iceoryx_app/main.cpp
)

target_link_libraries(iox-gw-eth2iceoryx
    PRIVATE
    iceoryx_posh::iceoryx_posh
    iceoryx_posh::iceoryx_posh_config
    iceoryx_posh::iceoryx_posh_gateway
    iceoryx_eth
)

target_compile_options(iox-gw-iceoryx2eth PRIVATE ${ICEORYX_WARNINGS})
#
########## build test executables ##########
#
if(test)
    add_subdirectory(test)
endif(test)


#
########## export library ##########
#
setup_install_directories_and_export_package(
    TARGETS  iox-gw-iceoryx2eth iox-gw-eth2iceoryx
    INCLUDE_DIRECTORY include/
)