# Copyright (c) 2019, 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

cmake_minimum_required(VERSION 3.5)
file (STRINGS "../VERSION" iceoryx_VERSION)
project(iceoryx_meta VERSION ${iceoryx_VERSION})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/build_options.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/tests.cmake)

# ===== Dependencies
if(TOML_CONFIG)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/cpptoml/ ${CMAKE_BINARY_DIR}/dependencies/cpptoml/prebuild)
endif(TOML_CONFIG)

# ===== Core
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_utils ${CMAKE_BINARY_DIR}/utils)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_posh ${CMAKE_BINARY_DIR}/posh)

# ===== Extras
if(INTROSPECTION)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../tools/introspection ${CMAKE_BINARY_DIR}/iceoryx_introspection)
endif(INTROSPECTION)

# ===== Gateways
if(DDS_GATEWAY)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/cyclonedds ${CMAKE_BINARY_DIR}/dependencies/cyclonedds/prebuild)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_dds ${CMAKE_BINARY_DIR}/iceoryx_dds)
endif(DDS_GATEWAY)

# ===== Language binding for C
if(BINDING_C)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_binding_c ${CMAKE_BINARY_DIR}/iceoryx_binding_c)
endif(BINDING_C)

# ===== Examples
if(EXAMPLES)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/icedelivery ${CMAKE_BINARY_DIR}/iceoryx_examples/icedelivery)
    if(BINDING_C)
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/icedelivery_on_c ${CMAKE_BINARY_DIR}/iceoryx_examples/icedelivery_on_c)
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/waitset_on_c ${CMAKE_BINARY_DIR}/iceoryx_examples/waitset_on_c)
    endif(BINDING_C)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/iceperf ${CMAKE_BINARY_DIR}/iceoryx_examples/iceperf)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/waitset ${CMAKE_BINARY_DIR}/iceoryx_examples/waitset)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/singleprocess ${CMAKE_BINARY_DIR}/iceoryx_examples/singleprocess)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/benchmark_optional_and_expected ${CMAKE_BINARY_DIR}/iceoryx_examples/benchmark_optional_and_expected)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/ice_multi_publisher ${CMAKE_BINARY_DIR}/iceoryx_examples/ice_multi_publisher)
endif(EXAMPLES)

message("")
message("       Build Properties")
message("          project name..............: " ${CMAKE_PROJECT_NAME})
message("          c compiler................: " ${CMAKE_C_COMPILER})
message("          c++ compiler..............: " ${CMAKE_CXX_COMPILER})
message("")

