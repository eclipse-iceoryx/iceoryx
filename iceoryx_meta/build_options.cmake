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

option(TOML_CONFIG      "Activates or deactivates TOML support - without TOML RouDi will not be build" ON)
option(ONE_TO_MANY_ONLY "Restricts communication to 1:n pattern" OFF)
option(BUILD_STRICT     "Build is performed with '-Werror'" OFF)
option(test             "Build all tests" ON)
option(coverage         "Build iceoryx with gcov flags" OFF)
option(examples         "Build all iceoryx examples" ON)
option(introspection    "Builds the introspection client which requires the ncurses library with an activated terminfo feature" OFF)
option(dds_gateway      "Builds the iceoryx dds gateway - enables internode communication via dds" OFF)
option(binding_c        "Builds the C language bindings" ON)
option(sanitize         "Build with sanitizers" OFF)

message("")
message("       Configured Options")
message("          TOML support......................: " ${TOML_CONFIG})
message("          1:n communication only ...........: " ${ONE_TO_MANY_ONLY})
message("          Strict build......................: " ${BUILD_STRICT})
message("          Test build........................: " ${test})
message("          Coverage Scan.....................: " ${coverage})
message("          Example build.....................: " ${examples})
message("          Introspection build...............: " ${introspection})
message("          DDS-gateway.......................: " ${dds_gateway})
message("          C-binding.........................: " ${binding_c})
message("          Sanitizer.........................: " ${sanitize})




