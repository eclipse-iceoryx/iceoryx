#!/bin/bash
# Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

COMPILER=gcc
SANITIZER=asan
BUILD_32BIT=""

while (( "$#" )); do
  case "$1" in
    "32-bit-x86")
        BUILD_32BIT="32-bit-x86"
        shift 1
        ;;
  esac
done

msg() {
    printf "\033[1;32m%s: %s\033[0m\n" ${FUNCNAME[1]} "$1"
}

WORKSPACE=$(git rev-parse --show-toplevel)
cd ${WORKSPACE}

msg "creating local test users and groups for testing access control"
sudo ./tools/scripts/add_test_users.sh

msg "compiler versions:
$(gcc --version)
$(clang --version)"

msg "building sources"
./tools/iceoryx_build_test.sh build-strict build-all out-of-tree build-shared test-add-user ${BUILD_32BIT}

msg "building debian package"
./tools/iceoryx_build_test.sh package ${BUILD_32BIT}

# there are tests which open quite a lot of file descriptors simultaneously to exhaust the creation of some resources
# therefore the limits needs to be increased
ulimit -n 2000
export IOX_RUN_ULIMIT_TESTS=on

msg "running all tests"
cd ./build
./tools/run_tests.sh all
cd -

msg "building roudi examples without toml support"
./tools/iceoryx_build_test.sh relwithdebinfo out-of-tree examples toml-config-off clean ${BUILD_32BIT}
