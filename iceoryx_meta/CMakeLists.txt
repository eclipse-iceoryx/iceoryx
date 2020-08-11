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

cmake_minimum_required(VERSION 3.5)
file (STRINGS "../VERSION" iceoryx_VERSION)
project(iceoryx_meta VERSION ${iceoryx_VERSION})

option(TOML_CONFIG "activates or deactivates TOML support - without TOML RouDi will not be build" ON)
option(BUILD_STRICT "Build is performed with '-Werror'" OFF)
option(test "Build tests" ON)
option(examples "build with iceoryx examples" ON)
option(introspection "builds the introspection client which requires the ncurses library with an activated terminfo feature" OFF)
option(dds_gateway "builds the iceoryx dds gateway - enables internode communication via dds" OFF)

# ===== Dependencies
if(TOML_CONFIG)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/cpptoml/ ${CMAKE_BINARY_DIR}/dependencies/cpptoml/prebuild)
endif(TOML_CONFIG)

if(test)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/googletest ${CMAKE_BINARY_DIR}/dependencies/googletest/prebuild)
endif(test)

# ===== Core
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_utils ${CMAKE_BINARY_DIR}/utils)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_posh ${CMAKE_BINARY_DIR}/posh)

# ===== Extras
if(introspection)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../tools/introspection ${CMAKE_BINARY_DIR}/iceoryx_introspection)
endif(introspection)

if(examples)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/icedelivery ${CMAKE_BINARY_DIR}/iceoryx_examples/icedelivery)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/iceperf ${CMAKE_BINARY_DIR}/iceoryx_examples/iceperf)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_examples/singleprocess ${CMAKE_BINARY_DIR}/iceoryx_examples/singleprocess)
endif(examples)

# ===== Gateways
if(dds_gateway)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/cyclonedds ${CMAKE_BINARY_DIR}/dependencies/cyclonedds/prebuild)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../iceoryx_dds ${CMAKE_BINARY_DIR}/iceoryx_dds)
endif(dds_gateway)


message("")
message("       Configured Options")
message("          TOML_CONFIG...............: " ${TOML_CONFIG})
message("          BUILD_STRICT..............: " ${BUILD_STRICT})
message("          test......................: " ${test})
message("          examples..................: " ${examples})
message("          introspection.............: " ${introspection})
message("          dds_gateway...............: " ${dds_gateway})

message("")
message("       Build Properties")
message("          project name..............: " ${CMAKE_PROJECT_NAME})
message("          c compiler................: " ${CMAKE_C_COMPILER})
message("          c++ compiler..............: " ${CMAKE_CXX_COMPILER})
message("")

