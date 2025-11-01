#!/bin/bash
# Copyright (c) 2025 by ekxide IO GmbH. All rights reserved.
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

# This script builds iceoryx_hoofs und iceoryx_posh and executes all tests

set -e

COLOR_OFF='\033[0m'
COLOR_RED='\033[1;31m'
COLOR_GREEN='\033[1;32m'
COLOR_YELLOW='\033[1;33m'
COLOR_BLUE='\033[1;34m'

WORKSPACE=$(git rev-parse --show-toplevel)
cd ${WORKSPACE}

echo -e "${COLOR_BLUE}#${COLOR_OFF}"
echo -e "${COLOR_BLUE}# Building gTest${COLOR_OFF}"
echo -e "${COLOR_BLUE}#${COLOR_OFF}"
cmake -S cmake/googletest \
      -B build/googletest \
      -DBUILD_TEST=ON
# NOTE: gTest is build in the configure step and installed to the location below;
#       lets move it to a place where the other artifacts will be installed to simplify the setup
mv build/googletest/dependencies/install/ build/install

echo -e "${COLOR_BLUE}#${COLOR_OFF}"
echo -e "${COLOR_BLUE}# Building iceoryx_platform with minimal POSIX platform${COLOR_OFF}"
echo -e "${COLOR_BLUE}#${COLOR_OFF}"
cmake -S iceoryx_platform \
      -B build/platform \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=build/install \
      -DIOX_PLATFORM_MINIMAL_POSIX=ON \
      -DBUILD_TEST=ON
cmake --build build/platform
cmake --install build/platform

echo -e "${COLOR_BLUE}#${COLOR_OFF}"
echo -e "${COLOR_BLUE}# Building iceoryx_hoofs subset${COLOR_OFF}"
echo -e "${COLOR_BLUE}#${COLOR_OFF}"
cmake -S iceoryx_hoofs \
      -B build/hoofs \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=build/install \
      -DCMAKE_PREFIX_PATH="${WORKSPACE}/build/install" \
      -DIOX_USE_HOOFS_SUBSET_ONLY=ON \
      -DBUILD_TEST=ON
cmake --build build/hoofs
cmake --install build/hoofs

echo -e "${COLOR_BLUE}#${COLOR_OFF}"
echo -e "${COLOR_BLUE}# Running iceoryx_platform tests${COLOR_OFF}"
echo -e "${COLOR_BLUE}#${COLOR_OFF}"
build/platform/platform/test/platform_moduletests

echo -e "${COLOR_BLUE}#${COLOR_OFF}"
echo -e "${COLOR_BLUE}# Running iceoryx_hoofs tests${COLOR_OFF}"
echo -e "${COLOR_BLUE}#${COLOR_OFF}"
build/hoofs/hoofs/test/hoofs_moduletests
