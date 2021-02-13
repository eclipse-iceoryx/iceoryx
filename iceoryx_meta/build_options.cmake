# Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.5)

option(BUILD_ALL "Build with all extensions and all tests" OFF)
option(TOML_CONFIG "TOML support for RouDi with dynamic configuration" ON)
option(ONE_TO_MANY_ONLY "Restricts communication to 1:n pattern" OFF)
option(BUILD_STRICT "Build is performed with '-Werror'" OFF)
option(BUILD_SHARED_LIBS "Build iceoryx as shared libraries" OFF)
option(BUILD_TEST "Build all tests" OFF)
option(BUILD_DOC "Build and generate documentation" OFF)
option(COVERAGE "Build iceoryx with gcov flags" OFF)
option(EXAMPLES "Build all iceoryx examples" OFF)
option(INTROSPECTION "Builds the introspection client which requires the ncurses library with an activated terminfo feature" ON)
option(DDS_GATEWAY "Builds the iceoryx dds gateway - enables internode communication via dds" OFF)
option(BINDING_C "Builds the C language bindings" ON)
option(SANITIZE "Build with sanitizers" OFF)
option(CLANG_TIDY "Execute Clang-Tidy" OFF)
option(ROUDI_ENVIRONMENT "Build RouDi Environment for testing, is enabled when building tests" OFF)
option(TEST_WITH_ADDITIONAL_USER "Build Test with additional user accounts for testing access control" OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # "Create compile_commands.json file"

if(BUILD_ALL)
  set(EXAMPLES ON)
  set(INTROSPECTION ON)
  set(BUILD_TEST ON)
  set(INTROSPECTION ON)
  set(BINDING_C ON)
  set(DDS_GATEWAY ON)
endif()

if(COVERAGE)
  set(BUILD_TEST ON)
endif()

if(SANITIZE)
  set(BUILD_TEST ON)
endif()

if(BUILD_TEST)
  set(ROUDI_ENVIRONMENT ON)
  set(BINDING_C ON)
endif()

if(EXAMPLES)
  set(BINDING_C ON)
endif()

message("")
message("       Configured Options")
message("          CMAKE_BUILD_TYPE.....................: " ${CMAKE_BUILD_TYPE})
message("          TOML_CONFIG..........................: " ${TOML_CONFIG})
message("          BUILD_ALL............................: " ${BUILD_ALL})
message("          BUILD_STRICT.........................: " ${BUILD_STRICT})
message("          BUILD_SHARED_LIBS....................: " ${BUILD_SHARED_LIBS})
message("          BUILD_TEST...........................: " ${BUILD_TEST})
message("          EXAMPLES.............................: " ${EXAMPLES})
message("          INTROSPECTION........................: " ${INTROSPECTION})
message("          DDS_GATEWAY..........................: " ${DDS_GATEWAY})
message("          BINDING_C............................: " ${BINDING_C})
message("          COVERAGE.............................: " ${COVERAGE})
message("          SANITIZE.............................: " ${SANITIZE})
message("          CLANG_TIDY...........................: " ${CLANG_TIDY})
message("          ROUDI_ENVIRONMENT....................: " ${ROUDI_ENVIRONMENT})
message("          CMAKE_EXPORT_COMPILE_COMMANDS........: " ${CMAKE_EXPORT_COMPILE_COMMANDS})
message("          ONE_TO_MANY_ONLY ....................: " ${ONE_TO_MANY_ONLY})
message("          TEST_WITH_ADDITIONAL_USER ...........: " ${TEST_WITH_ADDITIONAL_USER})
message("          BUILD_DOC............................: " ${BUILD_DOC})
