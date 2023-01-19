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

SANITIZER=${1:-asan}

msg() {
    printf "\033[1;32m%s: %s\033[0m\n" ${FUNCNAME[1]} "$1"
}

if [ "$SANITIZER" != "asan" && "$SANITIZER" != "tsan" ]; then
    msg "Invalid sanitizer."
    exit 1
fi

WORKSPACE=$(git rev-parse --show-toplevel)
cd ${WORKSPACE}

msg "compiler version:
$(clang --version)"

msg "building and installing dependencies"
# tinfo library which is required by iceoryx_introspection isn't available in mac
brew install ncurses wget
cd "${WORKSPACE}"

msg "building sources"
export LDFLAGS="-L/usr/local/opt/ncurses/lib"
export CFLAGS="-I/usr/local/opt/ncurses/include"
./tools/iceoryx_build_test.sh build-strict build-all $SANITIZER

msg "running tests (excluding timing_tests)"
cd ./build
if [ "$SANITIZER" == "asan" ]; then
    tools/run_tests.sh asan-only
elif [ "$SANITIZER" == "tsan" ]; then
    tools/run_tests.sh tsan-only continue-on-error
else
    tools/run_tests.sh all
fi
cd -
