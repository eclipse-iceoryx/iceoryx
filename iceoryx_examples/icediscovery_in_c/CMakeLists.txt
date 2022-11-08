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
project(example_icediscovery_in_c)

include(GNUInstallDirs)

find_package(iceoryx_platform REQUIRED)
find_package(iceoryx_posh CONFIG REQUIRED)
find_package(iceoryx_hoofs CONFIG REQUIRED)
find_package(iceoryx_binding_c CONFIG REQUIRED)

include(IceoryxPackageHelper)
include(IceoryxPlatform)
include(IceoryxPlatformSettings)

iox_add_executable(
    USE_C_LANGUAGE
    TARGET  iox-c-offer-service
    FILES   ./iox_c_offer_service.c
    LIBS    iceoryx_binding_c::iceoryx_binding_c
)

iox_add_executable(
    USE_C_LANGUAGE
    TARGET  iox-c-find-service
    FILES   ./iox_c_find_service.c
    LIBS    iceoryx_binding_c::iceoryx_binding_c
)
