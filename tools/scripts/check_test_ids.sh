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

# This script does a sanity check for the number of tests and the number of test IDs and
# checks for unique test IDs

set -e

ICEORYX_PATH=$(git rev-parse --show-toplevel)
cd $ICEORYX_PATH

## sanity check for number of tests and test IDs
for file in $(find . -iname "*.cpp")
do
    echo "Check file: $file"
    numberOfTestCases=$(grep -rn --include="*.cpp" -e "^\(TEST\|TYPED_TEST\|TIMING_TEST\)" $file | wc -l)
    numberOfFalsePositives=$(grep -rn --include="*.cpp" TYPED_TEST_SUITE $file | wc -l)
    set +e
    numberOfTestCasesWithoutFalsePositives=$(expr $numberOfTestCases - $numberOfFalsePositives)
    set -e
    numberOfTestIDs=$(grep -rn --include="*.cpp" "::testing::Test::RecordProperty" $file | wc -l)
    if [[ "$numberOfTestCasesWithoutFalsePositives" -gt "$numberOfTestIDs" ]]; then
        echo -e "\e[1;31mThe file \"$file\" is missing test IDs for some test cases!\e[m"
        exit 1
    fi
done

## unique test IDs
notUniqueIds=$(grep -hr "::testing::Test::RecordProperty" | sort | uniq -d)
if [[ -n "$notUniqueIds" ]]; then
    echo -e "\e[1;31mThe following test IDs are not unique!\e[m"
    echo "$notUniqueIds"
    exit 1
fi
