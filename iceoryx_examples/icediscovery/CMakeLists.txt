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

# Build icediscovery example
cmake_minimum_required(VERSION 3.16)
project(example_icediscovery)

include(GNUInstallDirs)

find_package(iceoryx_posh CONFIG REQUIRED)
find_package(iceoryx_hoofs CONFIG REQUIRED)

get_target_property(ICEORYX_CXX_STANDARD iceoryx_posh::iceoryx_posh CXX_STANDARD)
include(IceoryxPlatform)

add_executable(iox-cpp-offer-service ./iox_offer_service.cpp)
target_link_libraries(iox-cpp-offer-service
    iceoryx_posh::iceoryx_posh
)
target_compile_options(iox-cpp-offer-service PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS})

add_executable(iox-cpp-find-service ./iox_find_service.cpp)
target_link_libraries(iox-cpp-find-service
    iceoryx_posh::iceoryx_posh
)
target_compile_options(iox-cpp-find-service PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS})

add_executable(iox-cpp-wait-for-service ./iox_wait_for_service.cpp ./src/discovery_blocking.cpp)
target_link_libraries(iox-cpp-wait-for-service
    iceoryx_posh::iceoryx_posh
)

add_executable(iox-cpp-discovery-monitor ./iox_discovery_monitor.cpp ./src/discovery_monitor.cpp)
target_link_libraries(iox-cpp-discovery-monitor
    iceoryx_posh::iceoryx_posh
)

target_include_directories(iox-cpp-wait-for-service PRIVATE include)
target_include_directories(iox-cpp-discovery-monitor PRIVATE include)

target_compile_options(iox-cpp-wait-for-service PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS})
target_compile_options(iox-cpp-discovery-monitor PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS})
target_compile_options(iox-cpp-offer-service PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS})

set_target_properties(iox-cpp-offer-service
                      iox-cpp-find-service
                      iox-cpp-wait-for-service
                      iox-cpp-discovery-monitor
                      PROPERTIES
                      CXX_STANDARD_REQUIRED ON
                      CXX_STANDARD ${ICEORYX_CXX_STANDARD}
                      POSITION_INDEPENDENT_CODE ON
)

# ========================================================== //

install(TARGETS iox-cpp-offer-service
                iox-cpp-find-service
                iox-cpp-wait-for-service
                iox-cpp-discovery-monitor
        RUNTIME DESTINATION bin)
