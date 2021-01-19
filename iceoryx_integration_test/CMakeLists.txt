# Copyright 2020 Apex.AI, Inc.
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

cmake_minimum_required(VERSION 3.5)
project(iceoryx_integration_test)

include(GNUInstallDirs)

find_package(iceoryx_posh REQUIRED)
find_package(iceoryx_utils REQUIRED)
find_package(cpptoml REQUIRED)


# find dependencies
find_package(ament_cmake REQUIRED)
find_package(launch_testing_ament_cmake)

add_launch_test(test/simple_roudi_test.py)

add_executable(integrationtest src/integrationtest.cpp)
target_include_directories(
  integrationtest PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:include>)

#
# posh roudi daemon ##########
#
add_executable(iox-roudi src/roudi/roudi_main.cpp)
set_target_properties(
  iox-roudi
  PROPERTIES CXX_STANDARD_REQUIRED ON
             POSITION_INDEPENDENT_CODE ON
             RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

target_link_libraries(
  iox-roudi
  PRIVATE iceoryx_utils::iceoryx_utils iceoryx_posh::iceoryx_posh_roudi
          iceoryx_posh::iceoryx_posh_config)

target_include_directories(
  iox-roudi
  PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
          $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/dependencies/install/include>
          $<INSTALL_INTERFACE:include/${PREFIX}>)

# We need to install it in lib folder because ros2 search in this folder for executables
install(TARGETS integrationtest iox-roudi DESTINATION lib/${PROJECT_NAME})

# Install launch files.
install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}/
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
