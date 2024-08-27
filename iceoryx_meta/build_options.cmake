# Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

cmake_minimum_required(VERSION 3.16)

## please add new entries alphabetically sorted
option(BINDING_C "Builds the C language bindings" ON)
option(BUILD_ALL "Build with all extensions and all tests" OFF)
option(BUILD_DOC "Build and generate documentation" OFF)
option(BUILD_SHARED_LIBS "Build iceoryx as shared libraries" OFF)
option(BUILD_STRICT "Build is performed with '-Werror'" OFF)
option(BUILD_TEST "Build all tests" OFF)
option(CCACHE "Use ccache when it's available" ON)
option(COVERAGE "Build iceoryx with gcov flags" OFF)
option(DOWNLOAD_TOML_LIB "Download cpptoml via the CMake ExternalProject module" ON)
option(EXAMPLES "Build all iceoryx examples" OFF)
option(INTROSPECTION "Builds the introspection client which requires the ncurses library with an activated terminfo feature" OFF)
option(ONE_TO_MANY_ONLY "Restricts communication to 1:n pattern" OFF)
set(IOX_PLATFORM_PATH "" CACHE PATH "Overrides integrated platform detection and uses provided custom path")
option(ROUDI_ENVIRONMENT "Build RouDi Environment for testing, is enabled when building tests" OFF)
option(ADDRESS_SANITIZER "Build with address sanitizer" OFF)
option(THREAD_SANITIZER "Build with thread sanitizer" OFF)
option(TEST_WITH_ADDITIONAL_USER "Build Test with additional user accounts for testing access control" OFF)
option(TEST_WITH_HUGE_PAYLOAD "Build Tests which use payload bigger than 2GB" OFF)
option(TOML_CONFIG "TOML support for RouDi with dynamic configuration" ON)
option(IOX_EXPERIMENTAL_POSH "Export experimental posh features (no guarantees)" OFF)
option(IOX_REPRODUCIBLE_BUILD "Create reproducible builds by omit setting the build timestamp in the version header" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # "Create compile_commands.json file"

if(BUILD_ALL)
  set(EXAMPLES ON)
  set(BUILD_TEST ON)
  set(INTROSPECTION ON)
  set(BINDING_C ON)
endif()

## must be before the BUILD_TEST check
if(COVERAGE AND NOT BUILD_TEST)
  set(BUILD_TEST ON)
  set(BUILD_TEST_HINT "${BUILD_TEST_HINT} (activated by COVERAGE=ON)")
endif()

if(BUILD_TEST AND NOT ROUDI_ENVIRONMENT)
  set(ROUDI_ENV_HINT "${ROUDI_ENV_HINT} (activated by BUILD_TEST=ON)")
  set(ROUDI_ENVIRONMENT ON)
endif()

if(CCACHE)
  find_program(CCACHE_PROGRAM ccache)
  if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_PROGRAM})
  else()
    set(CCACHE_HINT "${CCACHE_HINT} (ccache not found)")
  endif()
endif()

function(show_config_options)
  message("")
  message("       CMake Options")
  message("          CMAKE_BUILD_TYPE.....................: " ${CMAKE_BUILD_TYPE})
  message("          CMAKE_TOOLCHAIN_FILE.................: " ${CMAKE_TOOLCHAIN_FILE})
  message("          CMAKE_EXPORT_COMPILE_COMMANDS........: " ${CMAKE_EXPORT_COMPILE_COMMANDS})
  message("")
  message("       iceoryx Options")
  message("          BINDING_C............................: " ${BINDING_C})
  message("          BUILD_ALL............................: " ${BUILD_ALL})
  message("          BUILD_DOC............................: " ${BUILD_DOC})
  message("          BUILD_SHARED_LIBS....................: " ${BUILD_SHARED_LIBS})
  message("          BUILD_STRICT.........................: " ${BUILD_STRICT})
  message("          BUILD_TEST...........................: " ${BUILD_TEST} ${BUILD_TEST_HINT})
  message("          CCACHE...............................: " ${CCACHE} ${CCACHE_HINT})
  message("          COVERAGE.............................: " ${COVERAGE})
  message("          DOWNLOAD_TOML_LIB....................: " ${DOWNLOAD_TOML_LIB})
  message("          EXAMPLES.............................: " ${EXAMPLES})
  message("          INTROSPECTION........................: " ${INTROSPECTION})
  message("          ONE_TO_MANY_ONLY ....................: " ${ONE_TO_MANY_ONLY})
  message("          IOX_PLATFORM_PATH....................: " ${IOX_PLATFORM_PATH})
  message("          ROUDI_ENVIRONMENT....................: " ${ROUDI_ENVIRONMENT} ${ROUDI_ENV_HINT})
  message("          ADDRESS_SANITIZER....................: " ${ADDRESS_SANITIZER})
  message("          THREAD_SANITIZER.....................: " ${THREAD_SANITIZER})
  message("          TEST_WITH_ADDITIONAL_USER ...........: " ${TEST_WITH_ADDITIONAL_USER})
  message("          TEST_WITH_HUGE_PAYLOAD ..............: " ${TEST_WITH_HUGE_PAYLOAD})
  message("          TOML_CONFIG..........................: " ${TOML_CONFIG})
  message("          IOX_EXPERIMENTAL_POSH................: " ${IOX_EXPERIMENTAL_POSH})
  message("          IOX_REPRODUCIBLE_BUILD...............: " ${IOX_REPRODUCIBLE_BUILD})
endfunction()
