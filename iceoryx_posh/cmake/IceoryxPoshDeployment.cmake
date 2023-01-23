# Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
# Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
# Copyright (c) 2022 by NXP. All rights reserved.
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

# define macro for option configuration
macro(configure_option)
    set(ONE_VALUE_ARGS NAME DEFAULT_VALUE)
    cmake_parse_arguments(CONFIGURE_OPTION "" "${ONE_VALUE_ARGS}" "" ${ARGN})

    if(NOT ${CONFIGURE_OPTION_NAME})
        set(${CONFIGURE_OPTION_NAME} ${CONFIGURE_OPTION_DEFAULT_VALUE})
    endif()
    message(STATUS "[i] ${CONFIGURE_OPTION_NAME}: " ${${CONFIGURE_OPTION_NAME}})
endmacro()

# configure deployment
message(STATUS "[i] <<<<<<<<<<<<< Start iceoryx_posh configuration: >>>>>>>>>>>>>")
if(ONE_TO_MANY_ONLY)
     message(STATUS "[i] Using 1:n communication only!")
     set(IOX_COMMUNICATION_POLICY OneToManyPolicy)
endif()

if(NOT IOX_COMMUNICATION_POLICY)
    message(STATUS "[i] Using m:n communication!")
    set(IOX_COMMUNICATION_POLICY ManyToManyPolicy)
endif()

# Refer to iceoryx_hoofs/include/iceoryx_hoofs/internal/posix_wrapper/ipc_channel.hpp
# for info why this is needed.
if(APPLE)
    set(IOX_MAX_RUNTIME_NAME_LENGTH_DEFAULT 98)
else()
    set(IOX_MAX_RUNTIME_NAME_LENGTH_DEFAULT 100)
endif()

configure_option(
    NAME IOX_MAX_PUBLISHERS
    DEFAULT_VALUE 512
)
configure_option(
    NAME IOX_MAX_SUBSCRIBERS
    DEFAULT_VALUE 1024
)
configure_option(
    NAME IOX_MAX_INTERFACE_NUMBER
    DEFAULT_VALUE 4
)
configure_option(
    NAME IOX_MAX_SUBSCRIBERS_PER_PUBLISHER
    DEFAULT_VALUE 256
)
configure_option(
    NAME IOX_MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY
    DEFAULT_VALUE 8
)
configure_option(
    NAME IOX_MAX_PUBLISHER_HISTORY
    DEFAULT_VALUE 16
)
configure_option(
    NAME IOX_MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY
    DEFAULT_VALUE 256
)
configure_option(
    NAME IOX_MAX_PROCESS_NUMBER
    DEFAULT_VALUE 300
)
configure_option(
    NAME IOX_MAX_NODE_NUMBER
    DEFAULT_VALUE 1000
)
configure_option(
    NAME IOX_MAX_NODE_PER_PROCESS
    DEFAULT_VALUE 50
)
configure_option(
    NAME IOX_MAX_SHM_SEGMENTS
    DEFAULT_VALUE 100
)
configure_option(
    NAME IOX_MAX_NUMBER_OF_MEMPOOLS
    DEFAULT_VALUE 32
)
configure_option(
    NAME IOX_MAX_NUMBER_OF_CONDITION_VARIABLES
    DEFAULT_VALUE 1024
)
configure_option(
    NAME IOX_MAX_NODE_NAME_LENGTH
    DEFAULT_VALUE 100
)
configure_option(
    NAME IOX_MAX_ID_STRING_LENGTH
    DEFAULT_VALUE 100
)
configure_option(
    NAME IOX_MAX_RUNTIME_NAME_LENGTH
    DEFAULT_VALUE ${IOX_MAX_RUNTIME_NAME_LENGTH_DEFAULT}
)
configure_option(
    NAME IOX_MAX_RESPONSES_PROCESSED_SIMULTANEOUSLY
    DEFAULT_VALUE 16
)
configure_option(
    NAME IOX_MAX_RESPONSE_QUEUE_CAPACITY
    DEFAULT_VALUE 16
)
configure_option(
    NAME IOX_MAX_REQUEST_QUEUE_CAPACITY
    DEFAULT_VALUE 1024
)
configure_option(
    NAME IOX_MAX_CLIENTS_PER_SERVER
    DEFAULT_VALUE 256
)

# note: don't change IOX_INTERNAL_MAX_NUMBER_OF_NOTIFIERS value because it could break the C-Binding
#configure_option(
#    NAME IOX_MAX_NUMBER_OF_NOTIFIERS
#    DEFAULT_VALUE 256
#)
set(IOX_INTERNAL_MAX_NUMBER_OF_NOTIFIERS 256)


message(STATUS "[i] <<<<<<<<<<<<<< End iceoryx_posh configuration: >>>>>>>>>>>>>>")

