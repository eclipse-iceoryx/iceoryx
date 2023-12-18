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

# Build iceperf example
cmake_minimum_required(VERSION 3.16)
project(example_iceperf)

option(TOML_CONFIG "TOML support for RouDi with dynamic configuration" ON)

find_package(iceoryx_platform REQUIRED)
find_package(iceoryx_posh CONFIG REQUIRED)
find_package(iceoryx_binding_c CONFIG REQUIRED)
find_package(iceoryx_hoofs CONFIG REQUIRED)

include(IceoryxPackageHelper)
include(IceoryxPlatform)
include(IceoryxPlatformSettings)

iox_add_executable(
    TARGET      iceperf-bench-leader
    FILES       main_leader.cpp iceperf_leader.cpp base.cpp iceoryx.cpp iceoryx_c.cpp iceoryx_wait.cpp uds.cpp mq.cpp
    LIBS        iceoryx_posh::iceoryx_posh iceoryx_binding_c::iceoryx_binding_c
    LIBS_QNX    socket
)

iox_add_executable(
    TARGET      iceperf-bench-follower
    FILES       main_follower.cpp iceperf_follower.cpp base.cpp iceoryx.cpp iceoryx_c.cpp iceoryx_wait.cpp uds.cpp mq.cpp
    LIBS        iceoryx_posh::iceoryx_posh iceoryx_binding_c::iceoryx_binding_c
    LIBS_QNX    socket
)

iox_add_executable(
    TARGET      iceperf-roudi
    FILES       ./roudi_main_static_config.cpp
    LIBS        iceoryx_hoofs::iceoryx_hoofs iceoryx_posh::iceoryx_posh_roudi
)
