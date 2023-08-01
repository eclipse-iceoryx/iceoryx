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
SKIP_TEST=false

while (( "$#" )); do
  case "$1" in
    "gcc"|"clang")
        COMPILER=$1
        shift 1
        ;;
    "asan"|"tsan")
        SANITIZER=$1
        shift 1
        ;;
    "skip-tests")
        SKIP_TEST=true
        shift 1
        ;;
  esac
done

msg() {
    printf "\033[1;32m%s: %s\033[0m\n" ${FUNCNAME[1]} "$1"
}

msg "COMPILER : $COMPILER "
msg "SANITIZER : $SANITIZER "
msg "SKIP_TEST : $SKIP_TEST "

WORKSPACE=$(git rev-parse --show-toplevel)
cd "${WORKSPACE}"

msg "installing build dependencies"
sudo apt-get update && sudo apt-get install -y libacl1-dev libncurses5-dev

msg "creating local test users and groups for testing access control"
sudo ./tools/scripts/add_test_users.sh

if [ "$SANITIZER" != "asan" ] && [ "$SANITIZER" != "tsan" ]; then
    msg "Invalid sanitizer."
    exit 1
fi

msg "building sources"
if [ "$COMPILER" == "gcc" ]; then
    ./tools/iceoryx_build_test.sh clean build-strict build-shared build-all debug $SANITIZER test-add-user out-of-tree
fi

if [ "$COMPILER" == "clang" ]; then
    ./tools/iceoryx_build_test.sh clean build-strict build-shared build-all clang debug $SANITIZER test-add-user out-of-tree
fi

if [ "$SKIP_TEST" == "true" ]; then
    msg "tests are skipped"
else
    msg "running all tests"
    cd ./build
    if [ "$SANITIZER" == "tsan" ]; then
        ./tools/run_tests.sh all continue-on-error
    else
        ./tools/run_tests.sh all
    fi
    cd -
fi
