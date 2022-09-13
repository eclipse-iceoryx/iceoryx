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

# In the in-tree build the `iceoryx_platform` is the first project which is built.
# The platform already includes the `IceoryxPlatformSettings.cmake` from the autodetected platform
# or from the user provided on (see, CMake argument `-DPLATFORM_PATH`).
# The other targets (hoofs, posh ...) require a `IceoryxPlatformSettings.cmake` file at a static
# location (see detailed explanation below) which requires this dummy file.
#
# This is a dummy file required for the iceoryx in tree build.
#   During the out-of-tree build the installed IceoryxPlatformSettings.cmake contains all the
#   platform specific variables.
#   In the in-tree build those variables are already globally set by the platform cmake
#   since it selects the correct platform and includes its corresponding IceoryxPlatformSettings.cmake
#   file. Something we cannot implement here.
#
#   Detailed technical explanation:
#     In the in-tree build the actual iceoryx_platform target is found and cmake uses the files
#     provided in cmake, like for instance the IceoryxPlatformSettings.cmake. But in that file we
#     cannot source the actual CMakeLists.txt of the platform since CMAKE_CURRENT_LIST_DIR points
#     to iceoryx_hoofs or posh and so on. And in cmake <= 3.16 there is no way to get the path of
#     the current cmake file so that we can source the platform detection and select the right 
#     settings. With cmake 3.17 a new variable CMAKE_CURRENT_FUNCTION_LIST_DIR was introduced which
#     would solve the problem, see https://cmake.org/cmake/help/v3.17/release/3.17.html?highlight=cmake_current_function_list_dir#variables
#
#     But this is actually not a problem since the iceoryx_platform is the first package which is
#     sourced in iceoryx_meta and therefore all the correct variables are set up and it suffices
#     that this file exists.
