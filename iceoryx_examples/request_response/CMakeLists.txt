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

# Build request_response_basic example
cmake_minimum_required(VERSION 3.16)
project(example_request_response)

include(GNUInstallDirs)

find_package(iceoryx_posh REQUIRED)
find_package(iceoryx_hoofs REQUIRED)

get_target_property(ICEORYX_CXX_STANDARD iceoryx_posh::iceoryx_posh CXX_STANDARD)
include(IceoryxPlatform)

## C++ untyped API client
add_executable(
    iox-cpp-request-response-untyped-client
    client_cxx_untyped.cpp
)
target_link_libraries(
    iox-cpp-request-response-untyped-client
    PRIVATE
    iceoryx_hoofs::iceoryx_hoofs
    iceoryx_posh::iceoryx_posh
)
target_compile_options(
    iox-cpp-request-response-untyped-client
    PRIVATE
    ${ICEORYX_WARNINGS}
    ${ICEORYX_SANITIZER_FLAGS}
)

## C++ untyped API server
add_executable(
    iox-cpp-request-response-untyped-server
    server_cxx_untpyed.cpp
)
target_link_libraries(
    iox-cpp-request-response-untyped-server
    PRIVATE
    iceoryx_hoofs::iceoryx_hoofs
    iceoryx_posh::iceoryx_posh
)
target_compile_options(
    iox-cpp-request-response-untyped-server
    PRIVATE
    ${ICEORYX_WARNINGS}
    ${ICEORYX_SANITIZER_FLAGS}
)

## C++ typed API client
add_executable(
    iox-cpp-request-response-basic-client
    client_cxx_basic.cpp
)
target_link_libraries(
    iox-cpp-request-response-basic-client
    PRIVATE
    iceoryx_hoofs::iceoryx_hoofs
    iceoryx_posh::iceoryx_posh
)
target_compile_options(
    iox-cpp-request-response-basic-client
    PRIVATE
    ${ICEORYX_WARNINGS}
    ${ICEORYX_SANITIZER_FLAGS}
)

## C++ typed API client with WaitSet
add_executable(
    iox-cpp-request-response-waitset-client
    client_cxx_waitset.cpp
)
target_link_libraries(
    iox-cpp-request-response-waitset-client
    PRIVATE
    iceoryx_hoofs::iceoryx_hoofs
    iceoryx_posh::iceoryx_posh
)
target_compile_options(
    iox-cpp-request-response-waitset-client
    PRIVATE
    ${ICEORYX_WARNINGS}
    ${ICEORYX_SANITIZER_FLAGS}
)

## C++ typed API server
add_executable(
    iox-cpp-request-response-basic-server
    server_cxx_basic.cpp
)
target_link_libraries(
    iox-cpp-request-response-basic-server
    PRIVATE
    iceoryx_hoofs::iceoryx_hoofs
    iceoryx_posh::iceoryx_posh
)
target_compile_options(
    iox-cpp-request-response-basic-server
    PRIVATE
    ${ICEORYX_WARNINGS}
    ${ICEORYX_SANITIZER_FLAGS}
)

## C++ typed API server with Listener
add_executable(
    iox-cpp-request-response-listener-server
    server_cxx_listener.cpp
)
target_link_libraries(
    iox-cpp-request-response-listener-server
    PRIVATE
    iceoryx_hoofs::iceoryx_hoofs
    iceoryx_posh::iceoryx_posh
)
target_compile_options(
    iox-cpp-request-response-listener-server
    PRIVATE
    ${ICEORYX_WARNINGS}
    ${ICEORYX_SANITIZER_FLAGS}
)

## additional properties
set_target_properties(
    iox-cpp-request-response-untyped-client
    iox-cpp-request-response-untyped-server
    iox-cpp-request-response-basic-client
    iox-cpp-request-response-basic-server
    iox-cpp-request-response-waitset-client
    iox-cpp-request-response-listener-server
    PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)

install(
    TARGETS
    iox-cpp-request-response-untyped-client
    iox-cpp-request-response-untyped-server
    iox-cpp-request-response-basic-client
    iox-cpp-request-response-basic-server
    iox-cpp-request-response-waitset-client
    iox-cpp-request-response-listener-server
    RUNTIME DESTINATION bin
)
