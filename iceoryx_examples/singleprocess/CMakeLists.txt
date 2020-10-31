# Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

# Build singleprocess example
cmake_minimum_required(VERSION 3.5)
project(example_singleprocess)

include(GNUInstallDirs)

find_package(iceoryx_posh CONFIG REQUIRED)
find_package(cpptoml REQUIRED)

get_target_property(ICEORYX_CXX_STANDARD iceoryx_posh::iceoryx_posh CXX_STANDARD)
if ( NOT ICEORYX_CXX_STANDARD )
    include(IceoryxPlatformDetection)
endif ( NOT ICEORYX_CXX_STANDARD )

add_executable(single_process ./single_process.cpp)
target_link_libraries(single_process
    iceoryx_posh::iceoryx_posh
    iceoryx_posh::iceoryx_posh_roudi
    iceoryx_posh::iceoryx_posh_config
)
set_target_properties(single_process PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD ${ICEORYX_CXX_STANDARD}
    POSITION_INDEPENDENT_CODE ON
)

install(
    TARGETS single_process
    RUNTIME DESTINATION bin
)
