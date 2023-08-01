#!/bin/bash
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

msg() {
    printf "\033[1;32m%s: %s\033[0m\n" ${FUNCNAME[1]} "$1"
}

WORKSPACE=$(git rev-parse --show-toplevel)
cd ${WORKSPACE}

msg "installing build dependencies"
sudo apt-get update && sudo apt-get install -y libacl1-dev libncurses5-dev

msg "compiler versions:
$(gcc --version)
$(clang --version)
$(bazel --version)"

msg "Linting Bazel files"
bazel run //:buildifier_lint_check

# Build everything including stresstests
msg "Bazel build"
bazel build //...

# Evaluate test failure https://github.com/eclipse-iceoryx/iceoryx/issues/1547
BAZEL_GTEST_FILTER="-Thread_test*"

# We exclude the sofi stresstest in CI since we cannot set the CPU Affinity in the GitHub Action Runners
msg "running tests"
bazel test //... --deleted_packages=iceoryx_hoofs/test/stresstests  --test_output=all --test_arg=--gtest_filter="$BAZEL_GTEST_FILTER"

# Build with clang
msg "Bazel build with clang"
bazel build --config=clang //...

msg "running tests with clang build"
bazel test --config=clang //... --deleted_packages=iceoryx_hoofs/test/stresstests  --test_output=all --test_arg=--gtest_filter="$BAZEL_GTEST_FILTER"
