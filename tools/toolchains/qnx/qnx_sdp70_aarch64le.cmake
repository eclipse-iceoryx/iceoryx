# Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
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

# @todo iox-#2026 update to QNX 7.1

SET(CMAKE_SYSTEM_PROCESSOR aarch64)
SET(arch gcc_ntoaarch64le)

include(${CMAKE_CURRENT_LIST_DIR}/qnx_sdp70_common.cmake)
