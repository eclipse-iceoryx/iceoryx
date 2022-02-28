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

find_package(iceoryx_posh CONFIG REQUIRED)
find_package(iceoryx_hoofs CONFIG REQUIRED)
find_package(iceoryx_binding_c CONFIG REQUIRED)

get_target_property(ICEORYX_CXX_STANDARD iceoryx_posh::iceoryx_posh CXX_STANDARD)
include(IceoryxPlatform)

add_executable(iox-c-offer-service ./iox_c_offer_service.c)
set_source_files_properties(./ice_c_offer_service.c PROPERTIES LANGUAGE C)
target_link_libraries(iox-c-offer-service
    iceoryx_binding_c::iceoryx_binding_c
)
set_target_properties(iox-c-offer-service PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
target_compile_options(iox-c-offer-service PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS})

add_executable(iox-c-find-service ./iox_c_find_service.c)
set_source_files_properties(./ice_c_find_service.c PROPERTIES LANGUAGE C)
target_link_libraries(iox-c-find-service
    iceoryx_binding_c::iceoryx_binding_c
)
set_target_properties(iox-c-offer-service PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
target_compile_options(iox-c-find-service PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS})

# ========================================================== //

install(TARGETS iox-c-offer-service
                iox-c-find-service
        RUNTIME DESTINATION bin)
