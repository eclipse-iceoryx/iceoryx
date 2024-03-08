# Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
cmake_minimum_required(VERSION 3.16)
project(test_binding_c VERSION 0)

find_package(iceoryx_hoofs_testing REQUIRED)
find_package(iceoryx_posh_testing REQUIRED)
find_package(GTest CONFIG REQUIRED)

set(PROJECT_PREFIX "binding_c")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_PREFIX}/test)

file(GLOB_RECURSE MODULETESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/moduletests/*.cpp")
file(GLOB_RECURSE INTEGRATIONTESTS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/integrationtests/*.cpp")

set(TEST_LINK_LIBS
    ${CODE_COVERAGE_LIBS}
    GTest::gtest
    GTest::gmock
    iceoryx_binding_c::iceoryx_binding_c
    iceoryx_hoofs::iceoryx_hoofs
    iceoryx_hoofs_testing::iceoryx_hoofs_testing
    iceoryx_posh::iceoryx_posh
    iceoryx_posh::iceoryx_posh_config
    iceoryx_posh::iceoryx_posh_roudi
    iceoryx_posh_testing::iceoryx_posh_testing
)

iox_add_executable( TARGET                  ${PROJECT_PREFIX}_moduletests
                    INCLUDE_DIRECTORIES     .
                    FILES                   ${MODULETESTS_SRC}
                    LIBS                    ${TEST_LINK_LIBS}
                    LIBS_APPLE              dl
                    LIBS_LINUX              acl dl rt
)

# not used in the platform, only added to make the scripts happy
iox_add_executable( TARGET                  ${PROJECT_PREFIX}_integrationtests
                    INCLUDE_DIRECTORIES     .
                    FILES                   ${INTEGRATIONTESTS_SRC}
                    LIBS                    ${TEST_LINK_LIBS}
                    LIBS_APPLE              dl
                    LIBS_LINUX              acl dl rt
)
