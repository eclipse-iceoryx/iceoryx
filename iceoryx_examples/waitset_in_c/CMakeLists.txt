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
project(example_waitset_in_c)

include(GNUInstallDirs)

find_package(iceoryx_platform REQUIRED)
find_package(iceoryx_posh CONFIG REQUIRED)
find_package(iceoryx_binding_c CONFIG REQUIRED)
find_package(iceoryx_hoofs CONFIG REQUIRED)

include(IceoryxPackageHelper)
include(IceoryxPlatform)
include(IceoryxPlatformSettings)

iox_add_executable(
    USE_C_LANGUAGE
    TARGET  iox-c-waitset-publisher
    FILES   ./ice_c_waitset_publisher.c
    LIBS    iceoryx_binding_c::iceoryx_binding_c
)

iox_add_executable(
    USE_C_LANGUAGE
    TARGET  iox-c-waitset-gateway
    FILES   ./ice_c_waitset_gateway.c
    LIBS    iceoryx_binding_c::iceoryx_binding_c
)

iox_add_executable(
    USE_C_LANGUAGE
    TARGET  iox-c-waitset-grouping
    FILES   ./ice_c_waitset_grouping.c
    LIBS    iceoryx_binding_c::iceoryx_binding_c
)

iox_add_executable(
    USE_C_LANGUAGE
    TARGET  iox-c-waitset-individual
    FILES   ./ice_c_waitset_individual.c
    LIBS    iceoryx_binding_c::iceoryx_binding_c
)

iox_add_executable(
    USE_C_LANGUAGE
    TARGET  iox-c-waitset-timer-driven-execution
    FILES   ./ice_c_waitset_timer_driven_execution.c
    LIBS    iceoryx_binding_c::iceoryx_binding_c
)
