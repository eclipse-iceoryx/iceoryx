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
# SPDX-License-Identifier: Apache-2.0

set -e

SCOPE=${1:-full} # Can be either `full` for all files or `hook` for formatting with git hooks


fail() {
    printf "\033[1;31merror: %s: %s\033[0m\n" ${FUNCNAME[1]} "${1:-"Unknown error"}"
    exit 1
}

hash git || fail "git not found"

# Check if we have at least a specific markdownlint version installed
MARKDOWNLINT_VERSION=0.33.0
MARKDOWNLINT_CMD="markdownlint"
if ! command -v $MARKDOWNLINT_CMD &> /dev/null
then
    MARKDOWNLINT_MAJOR_VERSION=$(markdownlint --version | sed -rn 's/.*([0-9][0-9])\.[0-9].*/\1/p')
    if [[ $MARKDOWNLINT_MAJOR_VERSION -lt "$MARKDOWNLINT_VERSION" ]]; then
        echo "Warning: markdownlint version $MARKDOWNLINT_VERSION or higher is not installed."
        echo "Code will not be formatted."
        exit 0
    else
        MARKDOWNLINT_CMD="markdownlint"
    fi
fi

cd "$(git rev-parse --show-toplevel)"

if [[ "$SCOPE" == "hook"* ]]; then
    md_files=$(git diff --cached --name-only --diff-filter=ACMRT | grep -E "\.md"| cat)
    if [[ -n ${md_files} ]]; then

        echo "Running markdownlint on the following file(s):"
        echo "--------------------------------------------"
        echo "${md_files}" | sed 's/^/\ -\ /g'
        echo

        git ls-files | grep -E "\.md"| xargs markdownlint --output ./markdownlint.log --config ./.markdownlint.yaml --fix

        for file in $md_files ; do
            if [ -f "$file" ]; then
                git add "${file}"
            fi
        done
    fi
elif [[ "$SCOPE" == "full"* ]]; then
    git ls-files | grep -E "\.md"| xargs markdownlint --output ./markdownlint.log --config ./.markdownlint.yaml --fix
elif [[ "$SCOPE" == "check"* ]]; then
    git ls-files | grep -E "\.md"| xargs markdownlint --output ./markdownlint.log --config ./.markdownlint.yaml --fix
fi
