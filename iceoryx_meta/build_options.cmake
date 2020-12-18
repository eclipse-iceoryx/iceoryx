# Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

option(BUILD_ALL        "Build with all extensions and all tests" OFF)
option(TOML_CONFIG      "Activates or deactivates TOML support - without TOML RouDi will not be build" ON)
option(ONE_TO_MANY_ONLY "Restricts communication to 1:n pattern" OFF)
option(BUILD_STRICT     "Build is performed with '-Werror'" OFF)
option(BUILD_TEST       "Build all tests" OFF)
option(COVERAGE         "Build iceoryx with gcov flags" OFF)
option(EXAMPLES         "Build all iceoryx examples" OFF)
option(INTROSPECTION    "Builds the introspection client which requires the ncurses library with an activated terminfo feature" ON)
option(DDS_GATEWAY      "Builds the iceoryx dds gateway - enables internode communication via dds" OFF)
option(BINDING_C        "Builds the C language bindings" OFF)
option(SANITIZE         "Build with sanitizers" OFF)
option(ROUDI_ENVIRONMENT"Build RouDi Environment for testing, is enabled when building tests" OFF)

if(BUILD_ALL)
    set(EXAMPLES ON)
    set(INTROSPECTION ON)
    set(BUILD_TEST ON)
    set(INTROSPECTION ON)
    set(BINDING_C ON)
    set(DDS_GATEWAY ON)
    set(BUILD_STRICT ON)
endif(BUILD_ALL)

if(BUILD_TEST)
    set(ROUDI_ENVIRONMENT ON)
endif(BUILD_TEST)


message("")
message("       Configured Options")
message("          TOML support......................: " ${TOML_CONFIG})
message("          1:n communication only ...........: " ${ONE_TO_MANY_ONLY})
message("          Strict build......................: " ${BUILD_STRICT})
message("          Test build........................: " ${BUILD_TEST})
message("          Coverage Scan.....................: " ${COVERAGE})
message("          Example build.....................: " ${EXAMPLES})
message("          Introspection build...............: " ${INTROSPECTION})
message("          DDS-gateway.......................: " ${DDS_GATEWAY})
message("          C-binding.........................: " ${BINDING_C})
message("          Sanitizer.........................: " ${SANITIZE})




