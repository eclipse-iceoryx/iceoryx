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
numberOfTestCasesTotal=$(numberOfTestCases "iceoryx_*")
numberOfFalsePositivesTotal=$(numberOfFalsePositives "iceoryx_*")
numberOfTestCasesWithoutFalsePositivesTotal=$numberOfTestCasesTotal-$numberOfFalsePositivesTotal
numberOfTestIDsTotal=$(numberOfTestIDs "iceoryx_*")
if [[ "$numberOfTestCasesWithoutFalsePositivesTotal" -gt "$numberOfTestIDsTotal" ]]; then
    echo -e "\e[1;31mThe number of test IDs do not match the number of test cases!\e[m"
    echo "number of test cases: $numberOfTestCasesTotal"
    echo "number of false positives: $numberOfFalsePositivesTotal"
    echo "number of test IDs: $numberOfTestIDsTotal"
    echo ""
    echo "trying to find files with missing IDs ..."

    found=0
    # find file with missing IDs
    for file in $(find iceoryx_* -iname "*.cpp")
    do
        numberOfTestCasesInFile=$(numberOfTestCases $file)
        numberOfFalsePositivesFile=$(numberOfFalsePositives $file)
        numberOfTestCasesWithoutFalsePositivesFile=$numberOfTestCasesInFile-$numberOfFalsePositivesFile
        numberOfTestIDsFile=$(numberOfTestIDs $file)
        if [[ "$numberOfTestCasesWithoutFalsePositivesFile" -gt "$numberOfTestIDsFile" ]]; then
            echo -e "\e[1;31mThe file \"$file\" is missing test IDs for some test cases!\e[m"
            found=1
        fi
    done

    if [[ $found -gt 0 ]]; then
        echo "... done"
    else
        echo "... not found"
    fi
    exit 1
fi


## unique test IDs
notUniqueIds=$(grep -hr "::testing::Test::RecordProperty" | sort | uniq -d)
if [[ -n "$notUniqueIds" ]]; then
    echo -e "\e[1;31mThe following test IDs are not unique!\e[m"
    echo "$notUniqueIds"
    exit 1
fi
