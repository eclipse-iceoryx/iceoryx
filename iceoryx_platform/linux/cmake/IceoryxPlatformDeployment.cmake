# Copyright (c) 2024 by Mathias Kraus <elboberido@m-hias.de>. All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Apache Software License 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0, or the MIT license
# which is available at https://opensource.org/licenses/MIT.
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT

# NOTE: Contrary to the 'IceoryxPlatformSettings.cmake' this file will not be installed and
# is only used to create a header with compile time constants.

message(STATUS "[i] <<<<<<<<<<<<< Start iceoryx_platform configuration: >>>>>>>>>>>>>")

configure_option(
    NAME IOX_PLATFORM_TEMP_DIR
    DEFAULT_VALUE "/tmp/"
)

configure_option(
    NAME IOX_PLATFORM_LOCK_FILE_PATH_PREFIX
    DEFAULT_VALUE "/tmp/"
)

configure_option(
    NAME IOX_PLATFORM_UDS_SOCKET_PATH_PREFIX
    DEFAULT_VALUE "/tmp/"
)

configure_option(
    NAME IOX_PLATFORM_DEFAULT_CONFIG_LOCATION
    DEFAULT_VALUE "/etc/"
)

include(CheckIncludeFile)
check_include_file("sys/acl.h" IOX_HAVE_SYS_ACL_H)
if(IOX_HAVE_SYS_ACL_H)
    set(IOX_PLATFORM_FEATURE_ACL_DEFAULT ON)
else()
    message(STATUS "[i] sys/acl.h not found - disabling ACL support (hermetic or minimal toolchain detected)")
    set(IOX_PLATFORM_FEATURE_ACL_DEFAULT OFF)
endif()

option(IOX_PLATFORM_FEATURE_ACL "Use ACLs for access control" ${IOX_PLATFORM_FEATURE_ACL_DEFAULT})
message(STATUS "[i] IOX_PLATFORM_FEATURE_ACL: ${IOX_PLATFORM_FEATURE_ACL}")

message(STATUS "[i] <<<<<<<<<<<<<< End iceoryx_platform configuration: >>>>>>>>>>>>>>")
