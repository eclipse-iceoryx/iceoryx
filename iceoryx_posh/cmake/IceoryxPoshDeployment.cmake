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

include(IceoryxConfigureOptionMacro)

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

# Refer to iceoryx_hoofs/posix/ipc/include/iox/posix_ipc_channel.hpp
# for info why this is needed.
if(APPLE)
    set(IOX_MAX_NODE_NAME_LENGTH_DEFAULT 85)
    set(IOX_MAX_RUNTIME_NAME_LENGTH_DEFAULT 85)
else()
    set(IOX_MAX_NODE_NAME_LENGTH_DEFAULT 87)
    set(IOX_MAX_RUNTIME_NAME_LENGTH_DEFAULT 87)
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
# NOTE: this is currently only used in the experimental API and corresponds to 'IOX_MAX_PROCESS_NUMBER'
# due to a limitation in the 'PointerRepository'
configure_option(
    NAME IOX_MAX_NODE_NUMBER
    DEFAULT_VALUE 300
)
# NOTE: this is currently set to 1 due to the limitation  in the 'PointerRepository'
configure_option(
    NAME IOX_MAX_NODE_PER_PROCESS
    DEFAULT_VALUE 1
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
    DEFAULT_VALUE ${IOX_MAX_NODE_NAME_LENGTH_DEFAULT}
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
configure_option(
    NAME IOX_MAX_NUMBER_OF_NOTIFIERS
    DEFAULT_VALUE 256
)
configure_option(
    NAME IOX_MAX_REQUESTS_PROCESSED_SIMULTANEOUSLY
    DEFAULT_VALUE 4
)

configure_option(
    NAME IOX_DEFAULT_RESOURCE_PREFIX
    DEFAULT_VALUE "iox1"
)

if(IOX_EXPERIMENTAL_POSH)
     set(IOX_EXPERIMENTAL_POSH_FLAG true)
else()
     set(IOX_EXPERIMENTAL_POSH_FLAG false)
endif()
message(STATUS "[i] IOX_EXPERIMENTAL_POSH_FLAG: ${IOX_EXPERIMENTAL_POSH_FLAG}")

message(STATUS "[i] <<<<<<<<<<<<<< End iceoryx_posh configuration: >>>>>>>>>>>>>>")

