# Copyright (c) 2019 - 2021 by Robert Bosch GmbH. All rights reserved.
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
#
########## set hoofs internal target, needed by tests in components ##########
#
# TODO: we need something like internal public for internal build dependencies
# instead of making the whole source folder public for internal dependency resolution

setup_package_name_and_create_files(
    NAME ${PROJECT_NAME}_testing
    NAMESPACE iceoryx_hoofs_testing
    PROJECT_PREFIX ${PREFIX}
)

add_library(iceoryx_hoofs_testing
    STATIC
    mocks/time_mock.cpp
    mocks/error_handler_mock.cpp
    timing_test.cpp
    compile_test.cpp
)

add_library(iceoryx_hoofs_testing::iceoryx_hoofs_testing ALIAS iceoryx_hoofs_testing)

set_target_properties(iceoryx_hoofs_testing PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(iceoryx_hoofs_testing PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)

if(TEST_WITH_ADDITIONAL_USER)
    target_compile_definitions(iceoryx_hoofs_testing PUBLIC -DTEST_WITH_ADDITIONAL_USER)
endif(TEST_WITH_ADDITIONAL_USER)

target_include_directories(iceoryx_hoofs_testing
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PREFIX}>
)

target_link_libraries(iceoryx_hoofs_testing
    PUBLIC
        GTest::gtest
        GTest::gmock
    PRIVATE
        ${CODE_COVERAGE_LIBS}
        iceoryx_hoofs
        ${CMAKE_DL_LIBS}
)

if(LINUX)
    target_link_libraries(iceoryx_hoofs_testing PRIVATE rt)
endif()

target_compile_options(iceoryx_hoofs_testing PRIVATE ${ICEORYX_WARNINGS})

setup_install_directories_and_export_package(
    TARGETS iceoryx_hoofs_testing
    INCLUDE_DIRECTORY include/
)

#
########## find_package in source tree ##########
#
set(${PROJECT_NAME}_testing_DIR ${PROJECT_SOURCE_DIR}/cmake
CACHE FILEPATH
"${PROJECT_NAME}_testingConfig.cmake to make find_package(${PROJECT_NAME}_testing) work in source tree!"
FORCE
)
