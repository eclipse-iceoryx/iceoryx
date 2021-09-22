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

COMPILER=${1:-gcc}

msg() {
    printf "\033[1;32m%s: %s\033[0m\n" ${FUNCNAME[1]} "$1"
}

WORKSPACE=$(git rev-parse --show-toplevel)
cd ${WORKSPACE}

msg "installing build dependencies"
sudo apt-get update && sudo apt-get install -y libacl1-dev libncurses5-dev

if [ "$COMPILER" == "clang" ]; then
    msg "installing latest stable clang"
    wget https://apt.llvm.org/llvm.sh -O /tmp/llvm.sh
    chmod +x /tmp/llvm.sh
    # set LLVM_VERSION
    eval $(cat /tmp/llvm.sh | grep LLVM_VERSION= -m 1)
    sudo /tmp/llvm.sh ${LLVM_VERSION}
fi

msg "creating local test users and groups for testing access control"
sudo ./tools/add_test_users.sh


msg "building sources"

if [ "$COMPILER" == "gcc" ]; then
    ./tools/iceoryx_build_test.sh clean release build-all out-of-tree build-shared test-add-user
fi

if [ "$COMPILER" == "clang" ]; then
    export CC=clang-${LLVM_VERSION}
    export CXX=clang++-${LLVM_VERSION}
    ./tools/iceoryx_build_test.sh clean clang release build-all out-of-tree build-shared test-add-user
fi
