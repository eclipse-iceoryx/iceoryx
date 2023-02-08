#!/bin/bash

# Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
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
# SPDX-Licensedentifier: Apache-2.0

set -e

SCOPE=${1:-full} # Can be either `full` for all files or `hook` for lintting with git hooks


fail() {
    printf "\033[1;31merror: %s: %s\033[0m\n" ${FUNCNAME[1]} "${1:-"Unknown error"}"
    exit 1
}

hash git || fail "git not found"

# Check if we have at least a specific cmake-lint version installed
CMAKE_LINT_VERSION="0.6.13"
CMAKE_LINT_CMD="cmake-lint"
if ! command -v $CMAKE_LINT_CMD &> /dev/null
then
    CMAKE_LINT_MAJOR_VERSION=$(cmake-lint --version | sed -rn 's/.*([0-9][0-9])\.[0-9].*/\1/p')
    if [[ $CMAKE_LINT_MAJOR_VERSION -lt "$CMAKE_LINT_VERSION" ]]; then
        echo "Warning: cmake-lint version $CMAKE_LINT_VERSION or higher is not installed."
        echo "Code will not be lintted."
        exit 0
    else
        CMAKE_LINT_CMD="cmake-lint"
    fi
fi

cd "$(git rev-parse --show-toplevel)"

if [[ "$SCOPE" == "hook"* ]]; then
    cmake_files=$(git diff --cached --name-only --diff-filter=ACMRT | grep -E "CMakeLists.txt|\.cmake" | cat)
    if [[ -n ${cmake_files} ]]; then

        echo "Running cmake-lint on the following file(s):"
        echo "--------------------------------------------"
        echo "${cmake_files}" | sed 's/^/\ -\ /g'
        echo

        for file in $cmake_files ; do
            if [ -f "$file" ]; then
                $CMAKE_LINT_CMD  -l error "${file}"
                git add "${file}"
            fi
        done
    fi
elif [[ "$SCOPE" == "full"* ]]; then
  git ls-files | grep -E "CMakeLists.txt|\.cmake" | xargs cmake-lint  -l error
elif [[ "$SCOPE" == "check"* ]]; then
  git ls-files | grep -E "CMakeLists.txt|\.cmake" | xargs cmake-lint  -l error
fi
