# Copyright (c) 2021 - 2023 by Apex.AI Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.16)
project(iceoryx_integrationtest)

find_package(ament_cmake REQUIRED)

if(BUILD_TESTING)
  find_package(launch_testing_ament_cmake REQUIRED)

  list(APPEND IOX_INT_TESTS test_roudi_startup_shutdown test_callback_example test_callback_in_c_example
    test_icehello_example test_icedelivery_example test_icedelivery_in_c_example
    test_singleprocess_example test_waitset_example test_waitset_in_c_example
    test_iceoptions_example test_complexdata_example test_user_header_example
    test_icediscovery_example test_icediscovery_in_c_example test_icediscovery_monitoring
    test_request_response_in_c_listener_and_waitset test_request_response_basic test_request_response_in_c_basic
    test_request_response_listener test_request_response_untyped
  )

  if(IOX_EXPERIMENTAL_POSH)
    list(APPEND IOX_INT_TESTS test_experimental_node)
  else()
    message(FATAL_ERROR "Please run colcon with '--meta=path/to/iceoryx_integrationtest/colcon.meta' to enable the experimental feature flag for the integration tests")
  endif()
  # add test_ice_access_control_example when activating integrationtest #1910

  foreach(TEST IN LISTS IOX_INT_TESTS)
    add_launch_test(
      "${PROJECT_NAME}/${TEST}.py"
      TIMEOUT 45
    )
  endforeach()
endif()

ament_package()
