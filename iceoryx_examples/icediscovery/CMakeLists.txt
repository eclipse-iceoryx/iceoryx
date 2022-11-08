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

find_package(iceoryx_platform REQUIRED)
find_package(iceoryx_posh CONFIG REQUIRED)
find_package(iceoryx_hoofs CONFIG REQUIRED)

include(IceoryxPackageHelper)
include(IceoryxPlatform)
include(IceoryxPlatformSettings)

iox_add_executable(
    TARGET  iox-cpp-offer-service
    FILES   ./iox_offer_service.cpp
    LIBS    iceoryx_posh::iceoryx_posh
)

iox_add_executable(
    TARGET  iox-cpp-find-service
    FILES   ./iox_find_service.cpp
    LIBS    iceoryx_posh::iceoryx_posh
)

iox_add_executable(
    TARGET              iox-cpp-wait-for-service
    FILES               ./iox_wait_for_service.cpp ./src/discovery_blocking.cpp
    LIBS                iceoryx_posh::iceoryx_posh
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
)

iox_add_executable(
    TARGET              iox-cpp-discovery-monitor
    FILES               ./iox_discovery_monitor.cpp ./src/discovery_monitor.cpp
    LIBS                iceoryx_posh::iceoryx_posh
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
)
