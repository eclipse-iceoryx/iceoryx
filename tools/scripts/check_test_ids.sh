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

## sanity check for number of tests and test IDs
numberOfTestCases=$(grep -rn --include="*.cpp" -e "^\(TEST\|TYPED_TEST\|TIMING_TEST\)" iceoryx_* | wc -l)
numberOfFalsePositives=$(grep -rn --include="*.cpp" TYPED_TEST_SUITE iceoryx_* | wc -l)
numberOfTestCasesWithoutFalsePositives=$numberOfTestCases-$numberOfFalsePositives
numberOfTestIDs=$(grep -rn --include="*.cpp" "::testing::Test::RecordProperty" iceoryx_* | wc -l)
if [[ "$numberOfTestCasesWithoutFalsePositives" -gt "$numberOfTestIDs" ]]; then
    echo -e "\e[1;31mThe number of test IDs do not match the number of test cases!\e[m"
    echo "number of test cases: $numberOfTestCases"
    echo "number of false positives: $numberOfFalsePositives"
    echo "number of test IDs: $numberOfTestIDs"
    exit 1
fi

## unique test IDs
notUniqueIds=$(grep -hr "::testing::Test::RecordProperty" | sort | uniq -d)
if [[ -n "$notUniqueIds" ]]; then
    echo -e "\e[1;31mThe following test IDs are not unique!\e[m"
    echo "$notUniqueIds"
    exit 1
fi
