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
file (STRINGS "../VERSION" iceoryx_VERSION)
project(iceoryx-package)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "iceoryx")
set(CPACK_PACKAGE_VERSION "${iceoryx_VERSION}")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libacl1-dev,libncurses5-dev")
set(CPACK_PACKAGE_DESCRIPTION "iceoryx inter-process-communication (IPC) middleware")
set(CPACK_PACKAGE_CONTACT "iceoryx-dev@eclipse.org")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE")

include(CPack)
