# Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
# limitations under the License.cmake_minimum_required(VERSION 3.5)
project(example_icecallback_on_c)

include(GNUInstallDirs)

find_package(iceoryx_posh CONFIG REQUIRED)

get_target_property(ICEORYX_CXX_STANDARD iceoryx_posh::iceoryx_posh CXX_STANDARD)
if ( NOT ICEORYX_CXX_STANDARD )
    include(IceoryxPlatformDetection)
endif ( NOT ICEORYX_CXX_STANDARD )

add_executable(iox-c-callback-publisher ./ice_c_callback_publisher.c)
set_source_files_properties(./ice_c_callback_publisher.c PROPERTIES LANGUAGE C)
target_link_libraries(iox-c-callback-publisher
    iceoryx_posh::iceoryx_posh
    iceoryx_binding_c::iceoryx_binding_c
)
set_target_properties(iox-c-callback-publisher PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

add_executable(iox-c-callback-subscriber ./ice_c_callback_subscriber.c)
set_source_files_properties(./ice_c_callback_subscriber.c PROPERTIES LANGUAGE C)
target_link_libraries(iox-c-callback-subscriber
    iceoryx_posh::iceoryx_posh
    iceoryx_binding_c::iceoryx_binding_c
)
set_target_properties(iox-c-callback-subscriber PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

install(
    TARGETS iox-c-callback-publisher iox-c-callback-subscriber
    RUNTIME DESTINATION bin
)
