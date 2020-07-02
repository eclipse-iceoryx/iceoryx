#!/bin/bash

# Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

# This file runs all tests for Ice0ryx

component_folder="utils posh dds_gateway"

GTEST_FILTER="*"

for arg in "$@"
do 
    case "$arg" in
        "disable-timing-tests")
            GTEST_FILTER="-*.TimingTest_*"
            ;;
        "only-timing-tests")
            GTEST_FILTER="*.TimingTest_*"
            ;;
        *)
            echo ""
            echo "Test script for iceoryx."
            echo "By default all module-, integration- and componenttests are executed."
            echo ""
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "      disable-timing-tests        Disables all timing tests"
            echo "      only-timing-tests           Runs only timing tests"
            echo ""
            exit -1
            ;;
    esac
done 

#check if this script is sourced by another script,
# if yes then exit properly, so the other script can use this
# scripts definitions
[[ "${#BASH_SOURCE[@]}" -gt "1" ]] && { return 0; }

if [ -z "$TEST_RESULT_FOLDER" ]
then
    TEST_RESULT_FOLDER="$(pwd)/testresults"
fi

mkdir -p "$TEST_RESULT_FOLDER"

echo ">>>>>> Running Ice0ryx Tests <<<<<<"

set -e

BASE_DIRECTORY=$PWD

for folder in $component_folder; do
    echo ""
    echo "######################## processing moduletests & componenttests in $folder ########################"
    echo $PWD

    cd $BASE_DIRECTORY/$folder/test

    ./"$folder"_moduletests --gtest_filter="${GTEST_FILTER}" --gtest_output="xml:$TEST_RESULT_FOLDER/"$folder"_ModuleTestResults.xml"
    ./"$folder"_componenttests --gtest_filter="${GTEST_FILTER}" --gtest_output="xml:$TEST_RESULT_FOLDER/"$folder"_ComponenttestTestResults.xml"
    ./"$folder"_integrationtests --gtest_filter="${GTEST_FILTER}" --gtest_output="xml:$TEST_RESULT_FOLDER/"$folder"_IntegrationTestResults.xml"

done

# do not start RouDi while the module and componenttests are running;
# they might do things which hurts RouDi, like in the roudi_shm test where named semaphores are opened and closed

echo ">>>>>> Finished Running Iceoryx Tests <<<<<<"
