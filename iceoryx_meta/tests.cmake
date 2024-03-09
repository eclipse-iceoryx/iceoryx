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
cmake_minimum_required(VERSION 3.16)

if (BUILD_TEST)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/googletest ${CMAKE_BINARY_DIR}/dependencies/googletest/prebuild)

    ### create component list
    set(COMPONENTS "platform" "hoofs" "posh")

    ### possible place for more extensions
    if (BINDING_C)
        list(APPEND COMPONENTS "binding_c")
    endif()

    ### create test targets without Timing tests

    ### Only hoofs has mock tests
    list(APPEND MOCKTEST_CMD COMMAND ./hoofs/test/hoofs_mocktests --gtest_filter=-*.TimingTest_* --gtest_output=xml:${CMAKE_BINARY_DIR}/testresults/hoofs_MockTestResults.xml)

    foreach(cmp IN ITEMS ${COMPONENTS})
        list(APPEND MODULETEST_CMD COMMAND ./${cmp}/test/${cmp}_moduletests --gtest_filter=-*.TimingTest_* --gtest_output=xml:${CMAKE_BINARY_DIR}/testresults/${cmp}_ModuleTestResults.xml)
    endforeach()

    foreach(cmp IN ITEMS ${COMPONENTS})
        list(APPEND INTEGRATIONTEST_CMD COMMAND ./${cmp}/test/${cmp}_integrationtests --gtest_filter=-*.TimingTest_* --gtest_output=xml:${CMAKE_BINARY_DIR}/testresults/${cmp}_IntegrationTestResults.xml)
    endforeach()

    add_custom_target( all_tests
        ${MODULETEST_CMD}
        ${MOCKTEST_CMD}
        ${INTEGRATIONTEST_CMD}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )

    add_custom_target( mock_tests
        ${MOCKTEST_CMD}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )

    ### we need to create separate test targets for coverage scan
    add_custom_target( module_tests
        ${MODULETEST_CMD}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )

    add_custom_target( integration_tests
        ${INTEGRATIONTEST_CMD}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )

    ### create test target with Timing tests
    foreach(cmp IN ITEMS ${COMPONENTS})
        list(APPEND TIMING_MODULETEST_CMD COMMAND ./${cmp}/test/${cmp}_moduletests --gtest_filter=*.TimingTest_* --gtest_output=xml:${CMAKE_BINARY_DIR}/testresults/${cmp}_TimingModuleTestResults.xml)
        list(APPEND TIMING_INTEGRATIONTEST_CMD COMMAND ./${cmp}/test/${cmp}_integrationtests --gtest_filter=*.TimingTest_* --gtest_output=xml:${CMAKE_BINARY_DIR}/testresults/${cmp}_TimingIntegrationTestResults.xml)
    endforeach()

    add_custom_target( timing_module_tests
        ${TIMING_MODULETEST_CMD}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )

    add_custom_target( timing_integration_tests
        ${TIMING_INTEGRATIONTEST_CMD}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )

endif()
