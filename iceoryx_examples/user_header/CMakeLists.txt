# Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

# Build user_header example
cmake_minimum_required(VERSION 3.16)
project(example_user_header)

include(GNUInstallDirs)

find_package(iceoryx_platform REQUIRED)
find_package(iceoryx_posh REQUIRED)
find_package(iceoryx_hoofs REQUIRED)

include(IceoryxPackageHelper)
include(IceoryxPlatform)
include(IceoryxPlatformSettings)

iox_add_executable(
    TARGET  iox-cpp-user-header-publisher
    FILES   ./publisher_cxx_api.cpp
    LIBS    iceoryx_posh::iceoryx_posh
)

iox_add_executable(
    TARGET  iox-cpp-user-header-untyped-publisher
    FILES   ./publisher_untyped_cxx_api.cpp
    LIBS    iceoryx_posh::iceoryx_posh
)

iox_add_executable(
    TARGET  iox-cpp-user-header-subscriber
    FILES   ./subscriber_cxx_api.cpp
    LIBS    iceoryx_posh::iceoryx_posh
)

iox_add_executable(
    TARGET  iox-cpp-user-header-untyped-subscriber
    FILES   ./subscriber_untyped_cxx_api.cpp
    LIBS    iceoryx_posh::iceoryx_posh
)

find_package(iceoryx_binding_c QUIET)
if(iceoryx_binding_c_FOUND)
    iox_add_executable(
        USE_C_LANGUAGE
        TARGET  iox-c-user-header-publisher
        FILES   ./publisher_c_api.c
        LIBS    iceoryx_binding_c::iceoryx_binding_c
    )

    iox_add_executable(
        USE_C_LANGUAGE
        TARGET  iox-c-user-header-subscriber
        FILES   ./subscriber_c_api.c
        LIBS    iceoryx_binding_c::iceoryx_binding_c
    )
endif()
