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
    printf "\033[1;32m%s: %s\033[0m\n" ${FUNCNAME[1]} "$1"
}

WORKSPACE=$(git rev-parse --show-toplevel)
cd ${WORKSPACE}

msg "installing build dependencies and clang-tidy-10"
sudo apt-get update && sudo apt-get install -y libacl1-dev libncurses5-dev bison clang-tidy-10

msg "compiler versions:
$(clang-10 --version)"

msg "generating compile_commands.json"
CXX=clang++-10 CC=clang-10 cmake -Bbuild -Hiceoryx_meta

# NOTE: run only for one package and do not fail.
# In the future, add more packages here and ideally fail,
# when there are violations
msg "running clang-tidy"
run-clang-tidy-10.py -quiet -p build iceoryx_hoofs
