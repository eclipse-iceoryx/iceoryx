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

# Dummy cmake file for source tree builds. The actual variables are sourced from
# "IceoryxPlatformSettings.cmake" which resides either in the root path of the autodetected or
# user provided platorm. The out of tree build uses the installed copy of that file."
#
# Example:
#   1. "unix" is autodetected so the file "./unix/IceoryxPlatformSettings.cmake" is sourced
#   2. The user defines "/src/os2_warp/" as platform path then "/src/os2_warp/IceoryxPlatformSettings.cmake" is sourced
