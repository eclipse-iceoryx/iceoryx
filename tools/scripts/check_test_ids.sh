#!/bin/bash

# Copyright (c) 2022 - 2023 by Apex.AI Inc. All rights reserved.
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

function numberOfTestCases() {
    grep -rn --include="*.cpp" -e "^\(TEST\|TYPED_TEST\|TIMING_TEST\)" $1 | wc -l
}
function numberOfFalsePositives() {
    grep -rn --include="*.cpp" TYPED_TEST_SUITE $1 | wc -l
}
function numberOfTestIDs() {
    grep -rn --include="*.cpp" "::testing::Test::RecordProperty" $1 | wc -l
}

## sanity check for number of tests and test IDs
## at first a quick check and then a slower check for the respective files
NUMBER_OF_TEST_CASES_TOTAL=$(numberOfTestCases "iceoryx_*")
NUMBER_OF_FALSE_POSITIVES_TOTAL=$(numberOfFalsePositives "iceoryx_*")
NUMBER_OF_TEST_CASES_WITHOUT_FALSE_POSITIVES_TOTAL=$NUMBER_OF_TEST_CASES_TOTAL-$NUMBER_OF_FALSE_POSITIVES_TOTAL
NUMBER_OF_TEST_IDS_TOTAL=$(numberOfTestIDs "iceoryx_*")
if [[ "$NUMBER_OF_TEST_CASES_WITHOUT_FALSE_POSITIVES_TOTAL" -gt "$NUMBER_OF_TEST_IDS_TOTAL" ]]; then
    echo -e "\e[1;31mThe number of test IDs do not match the number of test cases!\e[m"
    echo "number of test cases: $NUMBER_OF_TEST_CASES_TOTAL"
    echo "number of false positives: $NUMBER_OF_FALSE_POSITIVES_TOTAL"
    echo "number of test IDs: $NUMBER_OF_TEST_IDS_TOTAL"
    echo ""
    echo "trying to find files with missing IDs ..."

    # find file with missing IDs
    while IFS= read -r -d '' FILE
    do
        NUMBER_OF_TEST_CASES_IN_FILE=$(numberOfTestCases $FILE)
        NUMBER_OF_FALSE_POSITIVES_IN_FILE=$(numberOfFalsePositives $FILE)
        NUMBER_OF_TEST_CASES_WITHOUT_FALSE_POSITIVES_IN_FILE=$NUMBER_OF_TEST_CASES_IN_FILE-$NUMBER_OF_FALSE_POSITIVES_IN_FILE
        NUMBER_OF_TEST_IDS_IN_FILE=$(numberOfTestIDs $FILE)
        if [[ "$NUMBER_OF_TEST_CASES_WITHOUT_FALSE_POSITIVES_IN_FILE" -gt "$NUMBER_OF_TEST_IDS_IN_FILE" ]]; then
            echo -e "\e[1;31mThe file \"$FILE\" is missing test IDs for some test cases!\e[m"
            FOUND=1
        fi
    done < <(find iceoryx_* -iname "*.cpp" -print0)

    if [[ $FOUND ]]; then
        echo "... done"
    else
        echo "... not found"
    fi
    exit 1
fi


## unique test IDs
NOT_UNIQUE_TEST_IDS=$(grep -hr "::testing::Test::RecordProperty" | sort | uniq -d)
if [[ -n "$NOT_UNIQUE_TEST_IDS" ]]; then
    echo -e "\e[1;31mThe following test IDs are not unique!\e[m"
    echo "$NOT_UNIQUE_TEST_IDS"
    exit 1
fi
