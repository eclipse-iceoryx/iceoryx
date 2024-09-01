#!/bin/bash

# Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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

# This script does a sanity check for the number of tests and the number of test IDs and
# checks for unique test IDs

# NOTE: don't use 'set -e' since this will abort the script whit an error when nothing is found

COLOR_OFF='\033[0m'
COLOR_RED='\033[1;31m'
COLOR_GREEN='\033[1;32m'
COLOR_YELLOW='\033[1;33m'

ICEORYX_PATH=$(git rev-parse --show-toplevel)
cd $ICEORYX_PATH

# search for 'std::atomic' and ignore 'std::atomic_thread_fence' and the files for the implementation and tests for 'iox::concurrent::Atomic'
INVALID_ATOMIC_USAGE=$(grep -Ern "std::atomic" iceoryx_* | rg -Fv "std::atomic_thread_fence" | rg -Fv "test_platform_atomic.cpp" | rg -Fv "iceoryx_platform/atomic.hpp")

if [[ -n "${INVALID_ATOMIC_USAGE}" ]]; then
    echo -e "${COLOR_RED}ERROR: ${COLOR_YELLOW}There are invalid occurrences of 'std::atomic'${COLOR_OFF}"
    echo -e "${INVALID_ATOMIC_USAGE}"
    echo -e "${COLOR_YELLOW}If these are false positives, please add them to 'tools/scripts/check_atomic_usage.sh'${COLOR_OFF}"
    exit 1
fi

echo -e "${COLOR_GREEN}No invalid use of 'std::atomic' found${COLOR_OFF}"
