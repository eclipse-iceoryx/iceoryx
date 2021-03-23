# Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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
cmake_minimum_required(VERSION 3.5)
project(example_waitset)

include(GNUInstallDirs)

find_package(iceoryx_posh CONFIG REQUIRED)
find_package(iceoryx_utils CONFIG REQUIRED)

get_target_property(ICEORYX_CXX_STANDARD iceoryx_posh::iceoryx_posh CXX_STANDARD)
include(IceoryxPlatform)

add_executable(iox-ex-waitset-publisher ./ice_waitset_publisher.cpp)
target_link_libraries(iox-ex-waitset-publisher
    iceoryx_posh::iceoryx_posh
)
target_compile_options(iox-ex-waitset-publisher PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS})

add_executable(iox-ex-waitset-gateway ./ice_waitset_gateway.cpp)
target_link_libraries(iox-ex-waitset-gateway
    iceoryx_posh::iceoryx_posh
)
target_compile_options(iox-ex-waitset-gateway PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS})

add_executable(iox-ex-waitset-grouping ./ice_waitset_grouping.cpp)
target_link_libraries(iox-ex-waitset-grouping
    iceoryx_posh::iceoryx_posh
)
target_compile_options(iox-ex-waitset-grouping PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS})

add_executable(iox-ex-waitset-individual ./ice_waitset_individual.cpp)
target_link_libraries(iox-ex-waitset-individual
    iceoryx_posh::iceoryx_posh
)
target_compile_options(iox-ex-waitset-individual PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS})

add_executable(iox-ex-waitset-sync ./ice_waitset_sync.cpp)
target_link_libraries(iox-ex-waitset-sync
    iceoryx_posh::iceoryx_posh
)
target_compile_options(iox-ex-waitset-sync PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS})

add_executable(iox-ex-waitset-trigger ./ice_waitset_trigger.cpp)
target_link_libraries(iox-ex-waitset-trigger
    iceoryx_posh::iceoryx_posh
)
target_compile_options(iox-ex-waitset-trigger PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS})

set_target_properties(
    iox-ex-waitset-trigger 
    iox-ex-waitset-sync
    iox-ex-waitset-individual
    iox-ex-waitset-grouping
    iox-ex-waitset-gateway
    iox-ex-waitset-publisher
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)

install(
    TARGETS iox-ex-waitset-publisher iox-ex-waitset-gateway iox-ex-waitset-grouping iox-ex-waitset-individual iox-ex-waitset-sync iox-ex-waitset-trigger
    RUNTIME DESTINATION bin
)
