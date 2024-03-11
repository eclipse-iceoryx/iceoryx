# Copyright (c) 2024 by Mathias Kraus <elboberido@m-hias.de>. All rights reserved.
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

# NOTE: Contrary to the 'IceoryxPlatformSettings.cmake' this file will not be installed and
# is only used to create a header with compile time constants.

message(STATUS "[i] <<<<<<<<<<<<< Start iceoryx_platform configuration: >>>>>>>>>>>>>")

configure_option(
    NAME IOX_PLATFORM_TEMP_DIR
    DEFAULT_VALUE "/tmp/"
)

configure_option(
    NAME IOX_PLATFORM_LOCK_FILE_PATH_PREFIX
    DEFAULT_VALUE "/var/lock/"
)

configure_option(
    NAME IOX_PLATFORM_UDS_SOCKET_PATH_PREFIX
    DEFAULT_VALUE "/tmp/"
)

message(STATUS "[i] <<<<<<<<<<<<<< End iceoryx_platform configuration: >>>>>>>>>>>>>>")
