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

msg() {
    printf "\033[1;32m%s: %s\033[0m\n" "${FUNCNAME[1]}" "$1"
}

WORKSPACE=$(git rev-parse --show-toplevel)
cd "${WORKSPACE}"

msg "installing dependencies"
sudo apt-get update
sudo apt-get install -y libacl1-dev libncurses5-dev doxygen graphviz texlive-font-utils lcov \
    plantuml texlive-latex-base texlive-latex-extra texlive-latex-recommended texlive-fonts-recommended

msg "compiler versions:
$(gcc --version)
$(clang --version)"

msg "coverage"
mkdir -p ./lcov_results/unittest ./lcov_results/unittest_timing
sudo ./tools/scripts/add_test_users.sh
./tools/iceoryx_build_test.sh build-strict build-all test-add-user -c unit
cp -rf ./build/lcov/ ./lcov_results/unittest/
./tools/iceoryx_build_test.sh build-strict build-all test-add-user -c unit-timing clean
cp -rf ./build/lcov/ ./lcov_results/unittest_timing/

msg "docs"
./tools/iceoryx_build_test.sh clean doc
