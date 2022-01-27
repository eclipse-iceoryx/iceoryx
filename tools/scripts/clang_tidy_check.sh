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

# This script checks code files with clang-tidy
# Example usage: ./tools/scripts/check_clang_tidy.sh hook|ci_pull_request_ci_full

set -e

MODE=$1

FILE_FILTER="\.(h|hpp|inl|c|cpp)$"
FILE_BLACKLIST='(test|testing|tools|iceoryx_posh|iceoryx_dds|iceoryx_binding_c|iceoryx_dds|doc|iceoryx_integrationtest|iceoryx_meta|iceoryx_examples)'

hash run-clang-tidy-12.py || fail "run-clang-tidy-12 not found, please install clang-tidy-12"

fail() {
    printf "\033[1;31merror: %s: %s\033[0m\n" ${FUNCNAME[1]} "${1:-"Unknown error"}"
    exit 1
}

WORKSPACE=$(git rev-parse --show-toplevel)
cd "${WORKSPACE}"

if ! [[ -f build/compile_commands.json ]]; then # or we just exit here with an error
    sudo apt-get update && sudo apt-get install -y libacl1-dev libncurses5-dev bison clang-tidy-12
    export CXX=clang++-12
    export CC=clang-12
    cmake -Bbuild -Hiceoryx_meta
fi

if [[ "$MODE" == "hook"* ]]; then
    FILES=$(git diff --cached --name-only --diff-filter=ACMRT | grep -E "$FILE_FILTER" | grep -Ev "$FILE_BLACKLIST" | cat)
    echo "Checking files with Clang-Tidy"
    echo " "
        if [ -z "$FILES" ]
        then
              echo "No files to check, skipping clang-tidy"
        else
              clang-tidy-12 -p build $FILES # Add `--warnings-as-errors=*` later
        fi
    exit
elif [[ "$MODE" == "full"* ]]; then
    FILES=$(git ls-files | grep -E "$FILE_FILTER" | grep -Ev "$FILE_BLACKLIST")
    echo "Checking all files with Clang-Tidy"
    echo " "
    echo $FILES
elif [[ "$MODE" == "ci_pull_request"* ]]; then
    # TODO: only process files change in PR
    # https://dev.to/scienta/get-changed-files-in-github-actions-1p36
    FILES=$(git diff --name-only --diff-filter=ACMRT ${{ github.event.pull_request.base.sha }} ${{ github.sha }}  | grep -E "$FILE_FILTER" | grep -Ev "$FILE_BLACKLIST")
    echo "Checking changed files with Clang-Tidy in Pull-Request"
    echo " "
    echo $FILES
fi

run-clang-tidy-12.py -p build $FILES
