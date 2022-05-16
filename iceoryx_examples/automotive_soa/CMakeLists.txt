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
project(example_automotive_soa)

include(GNUInstallDirs)

find_package(iceoryx_posh CONFIG REQUIRED)
find_package(iceoryx_hoofs CONFIG REQUIRED)

get_target_property(ICEORYX_CXX_STANDARD iceoryx_posh::iceoryx_posh CXX_STANDARD)
include(IceoryxPlatform)

add_executable(iox-cpp-automotive-skeleton  ./iox_automotive_skeleton.cpp
                                            ./src/minimal_skeleton.cpp
                                            ./src/runtime.cpp
                                            ./src/owl/kom/method_server.cpp
)
target_link_libraries(iox-cpp-automotive-skeleton
    iceoryx_posh::iceoryx_posh
)
target_compile_options(iox-cpp-automotive-skeleton PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS})

target_include_directories(iox-cpp-automotive-skeleton PRIVATE include)

add_executable(iox-cpp-automotive-proxy ./iox_automotive_proxy.cpp
                                        ./src/minimal_proxy.cpp
                                        ./src/runtime.cpp
                                        ./src/owl/kom/method_client.cpp
)
target_link_libraries(iox-cpp-automotive-proxy
    iceoryx_posh::iceoryx_posh
)
target_compile_options(iox-cpp-automotive-proxy PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS})

target_include_directories(iox-cpp-automotive-proxy PRIVATE include)

set_target_properties(iox-cpp-automotive-proxy
                      iox-cpp-automotive-skeleton
                      PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)

install(TARGETS iox-cpp-automotive-skeleton
                iox-cpp-automotive-proxy
        RUNTIME DESTINATION bin)
