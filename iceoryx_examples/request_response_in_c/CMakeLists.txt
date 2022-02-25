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
project(example_request_response_in_c)

include(GNUInstallDirs)

find_package(iceoryx_posh CONFIG REQUIRED)
find_package(iceoryx_binding_c CONFIG REQUIRED)
find_package(iceoryx_hoofs CONFIG REQUIRED)

get_target_property(ICEORYX_CXX_STANDARD iceoryx_posh::iceoryx_posh CXX_STANDARD)
include(IceoryxPlatform)

if("-Wno-noexcept-type" IN_LIST ICEORYX_WARNINGS)
    list(REMOVE_ITEM ICEORYX_WARNINGS "-Wno-noexcept-type")
endif()

add_executable(iox-c-request-response-client-basic ./client_c_basic.c)
set_source_files_properties(./client_c_basic.c PROPERTIES LANGUAGE C)
target_link_libraries(iox-c-request-response-client-basic
    iceoryx_binding_c::iceoryx_binding_c
)
target_compile_options(iox-c-request-response-client-basic PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS})

add_executable(iox-c-request-response-client-waitset ./client_c_waitset.c)
set_source_files_properties(./client_c_waitset.c PROPERTIES LANGUAGE C)
target_link_libraries(iox-c-request-response-client-waitset
    iceoryx_binding_c::iceoryx_binding_c
)
target_compile_options(iox-c-request-response-client-waitset PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS})

add_executable(iox-c-request-response-server-basic ./server_c_basic.c)
set_source_files_properties(./server_c_basic.c PROPERTIES LANGUAGE C)
target_link_libraries(iox-c-request-response-server-basic
    iceoryx_binding_c::iceoryx_binding_c
)
target_compile_options(iox-c-request-response-server-basic PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS})

add_executable(iox-c-request-response-server-listener ./server_c_listener.c)
set_source_files_properties(./server_c_listener.c PROPERTIES LANGUAGE C)
target_link_libraries(iox-c-request-response-server-listener
    iceoryx_binding_c::iceoryx_binding_c
)
target_compile_options(iox-c-request-response-server-listener PRIVATE ${ICEORYX_WARNINGS} ${ICEORYX_SANITIZER_FLAGS})

set_target_properties(
    iox-c-request-response-client-basic
    iox-c-request-response-client-waitset
    iox-c-request-response-server-basic
    iox-c-request-response-server-listener
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)

install(
    TARGETS 
      iox-c-request-response-client-basic
      iox-c-request-response-client-waitset
      iox-c-request-response-server-basic
      iox-c-request-response-server-listener
    RUNTIME DESTINATION bin
)
