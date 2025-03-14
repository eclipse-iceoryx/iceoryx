# Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

message(STATUS "[i] <<<<<<<<<<<<< Start iceoryx_hoofs configuration: >>>>>>>>>>>>>")

configure_option(
    NAME IOX_MINIMAL_LOG_LEVEL
    DEFAULT_VALUE "Trace"
)
configure_option(
    NAME IOX_MAX_NAMED_PIPE_MESSAGE_SIZE
    DEFAULT_VALUE 4096
)
configure_option(
    NAME IOX_MAX_NAMED_PIPE_NUMBER_OF_MESSAGES
    DEFAULT_VALUE 10
)

message(STATUS "[i] <<<<<<<<<<<<<< End iceoryx_hoofs configuration: >>>>>>>>>>>>>>")
