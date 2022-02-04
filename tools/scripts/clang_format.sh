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

set -e

SCOPE=${1:-full} # Can be either `full` for all files or `hook` for formatting with git hooks


fail() {
    printf "\033[1;31merror: %s: %s\033[0m\n" ${FUNCNAME[1]} "${1:-"Unknown error"}"
    exit 1
}

hash git || fail "git not found"

# Check if we have at least a specific clang-format version installed
CLANG_FORMAT_VERSION=12
CLANG_FORMAT_CMD="clang-format-$CLANG_FORMAT_VERSION"
if ! command -v $CLANG_FORMAT_CMD &> /dev/null
then
    CLANG_FORMAT_MAJOR_VERSION=$(clang-format --version | sed -rn 's/.*([0-9][0-9])\.[0-9].*/\1/p')
    if [[ $CLANG_FORMAT_MAJOR_VERSION -lt "$CLANG_FORMAT_VERSION" ]]; then
        echo "Warning: clang-format version $CLANG_FORMAT_VERSION or higher is not installed."
        echo "Code will not be formatted."
        exit 0
    else
        CLANG_FORMAT_CMD="clang-format"
    fi
fi

cd "$(git rev-parse --show-toplevel)"

if [[ "$SCOPE" == "hook"* ]]; then
    cpp_files=$(git diff --cached --name-only --diff-filter=ACMRT | grep -E "\.(c|cpp|inl|h|hpp)$" | cat)
    if [[ -n ${cpp_files} ]]; then

        echo "Running clang-format on the following file(s):"
        echo "--------------------------------------------"
        echo "${cpp_files}" | sed 's/^/\ -\ /g'
        echo

        for file in $cpp_files ; do
            if [ -f "$file" ]; then
                $CLANG_FORMAT_CMD -i -style=file "${file}"
                git add "${file}"
            fi
        done
    fi
elif [[ "$SCOPE" == "full"* ]]; then
    git ls-files | grep -E "\.(c|cpp|inl|h|hpp)$" | xargs clang-format -i -style=file --Werror
elif [[ "$SCOPE" == "check"* ]]; then
    git ls-files | grep -E "\.(c|cpp|inl|h|hpp)$" | xargs clang-format -i -style=file --Werror --dry-run
fi
