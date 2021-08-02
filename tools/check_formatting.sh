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

# This script add useraccounts to the system for testing Access Control Lists in iceoryx
# Use at your own risk!

set -e

fail() {
    printf "\033[1;31merror: %s: %s\033[0m\n" ${FUNCNAME[1]} "${1:-"Unknown error"}"
    exit 1
}

hash git || fail "git not found"
hash clang-format-10 || fail "clang-format-10 not found"

cd $(git rev-parse --show-toplevel)

# format files
git ls-files | grep -E "\.(c|cpp|inl|h|hpp)$" | xargs clang-format -i -style=file

# check incorrectly formatted files
git diff-index --name-only --exit-code HEAD -- || \
fail "
Files above are incorrectly formatted, please either
- install git-hooks https://github.com/eclipse-iceoryx/iceoryx/blob/master/tools/git-hooks/Readme.md
- or format files manually with 'clang-format-10 -i style=file <filename>'
"
