#!/usr/local/bin/bash
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

# This script builds iceoryx_hoofs und iceoryx_posh and executes all tests

set -e

ln -s /usr/local/bin/bash /bin/bash

msg() {
    printf "\033[1;32m%s: %s\033[0m\n" "${FUNCNAME[1]}" "$1"
}

WORKSPACE=$(git rev-parse --show-toplevel)
cd "${WORKSPACE}"

msg "compiler version:
$(clang --version)"

export OSTYPE="darwin"
./tools/iceoryx_build_test.sh clean build-strict build-test examples

msg "running tests (excluding timing_tests)"
cd "${WORKSPACE}"/build
make all_tests
cd "${WORKSPACE}"
