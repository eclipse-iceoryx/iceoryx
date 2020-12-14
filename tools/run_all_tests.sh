#!/bin/bash

# Copyright (c) 2019-2020 by Robert Bosch GmbH. All rights reserved.
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

COMPONENTS="utils posh binding_c"
GTEST_FILTER="*"
BASE_DIR=$PWD
GCOV_SCOPE="all"
CONTINUE_ON_ERROR=false

set_sanitizer_options () {
    # This script runs from build folder
    cd ..
    local PROJECT_ROOT=$PWD
    cd build

    echo "Project root is PROJECT_ROOT"

# new_delete_type_mismatch is disabled because of the below error
# ==112203==ERROR: AddressSanitizer: new-delete-type-mismatch on 0x622000021100 in thread T0:
#   object passed to delete has wrong type:
#   size of the allocated type:   5120 bytes;
#   size of the deallocated type: 496 bytes.
#     #0 0x7fd36deac9d8 in operator delete(void*, unsigned long) (/usr/lib/x86_64-linux-gnu/libasan.so.4+0xe19d8)
#     #1 0x55c8284bcc43 in ReceiverPort_test::~ReceiverPort_test() /home/pbt2kor/data/aos/repos/iceoryx_oss/iceoryx/iceoryx_posh/test/moduletests/test_posh_receiverport.cpp:49
#     #2 0x55c8284c15d1 in ReceiverPort_test_newdata_Test::~ReceiverPort_test_newdata_Test() /home/pbt2kor/data/aos/repos/iceoryx_oss/iceoryx/iceoryx_posh/test/moduletests/test_posh_receiverport.cpp:137
#     #3 0x55c8284c15ed in ReceiverPort_test_newdata_Test::~ReceiverPort_test_newdata_Test() /home/pbt2kor/data/aos/repos/iceoryx_oss/iceoryx/iceoryx_posh/test/moduletests/test_posh_receiverport.cpp:137
#     #4 0x55c82857b2fb in testing::Test::DeleteSelf_() (/home/pbt2kor/data/aos/repos/iceoryx_oss/iceoryx/build/posh/test/posh_moduletests+0x3432fb)
    echo "OSTYPE is $OSTYPE"
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        ASAN_OPTIONS=detect_leaks=1
    else
    # other OS (Mac here)
    # ==23449==AddressSanitizer: detect_leaks is not supported on this platform.
        ASAN_OPTIONS=detect_leaks=0
    fi
    ASAN_OPTIONS=$ASAN_OPTIONS:detect_stack_use_after_return=1:detect_stack_use_after_scope=1:check_initialization_order=true:strict_init_order=true:new_delete_type_mismatch=0:suppressions=$PROJECT_ROOT/iceoryx_meta/sanitizer_blacklist/asan_runtime.txt
    export ASAN_OPTIONS
    export LSAN_OPTIONS=suppressions=$PROJECT_ROOT/iceoryx_meta/sanitizer_blacklist/lsan_runtime.txt

    echo "ASAN_OPTIONS : $ASAN_OPTIONS"
    echo "LSAN_OPTIONS : $LSAN_OPTIONS"
}

for arg in "$@"
do 
    case "$arg" in
        "with-dds-gateway-tests")
            COMPONENTS="$COMPONENTS dds_gateway"
            ;;
        "disable-timing-tests")
            GTEST_FILTER="-*.TimingTest_*"
            ;;
        "only-timing-tests")
            GTEST_FILTER="*.TimingTest_*"
            ;;
        "continue-on-error")
            CONTINUE_ON_ERROR=true
            ;;
        "all" | "component" | "unit" | "integration")
            GCOV_SCOPE="$arg"
            ;;
        *)
            echo ""
            echo "Test script for iceoryx."
            echo "By default all module-, integration- and componenttests are executed."
            echo ""
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "      skip-dds-tests              Skips tests for iceoryx_dds"
            echo "      disable-timing-tests        Disables all timing tests"
            echo "      only-timing-tests           Runs only timing tests"
            echo "      continue-on-error           Continue execution upon error"
            echo ""
            exit -1
            ;;
    esac
done 

# check if this script is sourced by another script,
# if yes then exit properly, so the other script can use this
# scripts definitions
[[ "${#BASH_SOURCE[@]}" -gt "1" ]] && { return 0; }

if [ -z "$TEST_RESULTS_DIR" ]
then
    TEST_RESULTS_DIR="$(pwd)/testresults"
fi

mkdir -p "$TEST_RESULTS_DIR"

echo ">>>>>> Running Ice0ryx Tests <<<<<<"

if [ $CONTINUE_ON_ERROR == true ]
then
# Continue executing tests , when a test fails
    set +e
else
# Stop executing tests , when a test fails
    set -e
fi

failed_tests=0
execute_test () {
    local component=$1
    local test_scope=$2

    case $test_scope in
        "unit")
        test_binary="$component"_moduletests
        result_file="$component"_ModuleTestResults.xml
        ;;
        "component")
        test_binary="$component"_componenttests
        result_file="$component"_ComponenttestTestResults.xml
        ;;
        "integration")
        test_binary="$component"_integrationtests
        result_file="$component"_IntegrationTestResults.xml
        ;;
        *)
        echo "Wrong scope $test_scope!"
        ;;
    esac

    # Runs only tests available for the given component
    if [ -f ./$test_binary ]; then
        echo "Executing $test_binary"
        ./$test_binary --gtest_filter="${GTEST_FILTER}" --gtest_output="xml:$TEST_RESULTS_DIR/$result_file"
    fi

    # return code from test application is non-zero -> some test cases failed
    if [ $? != 0 ]; then
        ((failed_tests++))
        echo "$test_scope test for $component failed!"
    fi
}

set_sanitizer_options

execute_test () {
    local component=$1
    local test_scope=$2
    local test_binary=""

    case $test_scope in
        "unit")
        test_binary="$component"_moduletests
        result_file="$component"_ModuleTestResults.xml
        ;;
        "component")
        test_binary="$component"_componenttests
        result_file="$component"_ComponenttestTestResults.xml
        ;;
        "integration")
        test_binary="$component"_integrationtests
        result_file="$component"_IntegrationTestResults.xml
        ;;
        *)
        echo "Wrong scope $test_scope!"
        ;;
    esac

    # Runs only tests available for the given component
    if [ -f ./$test_binary ]; then
        echo "Executing $test_binary"
        ./$test_binary --gtest_filter="${GTEST_FILTER}" --gtest_output="xml:$TEST_RESULTS_DIR/$result_file"
    fi

    if [ $? != 0 ]; then
        echo "$test_scope test for $component failed!"
    fi
}

for COMPONENT in $COMPONENTS; do
    echo ""
    echo "######################## executing tests for $COMPONENT ########################"
    cd $BASE_DIR/$COMPONENT/test

    if [ $GCOV_SCOPE == "unit" ] || [ $GCOV_SCOPE == "all" ]; then
        execute_test $COMPONENT unit
    fi
    if [ $GCOV_SCOPE == "component" ] || [ $GCOV_SCOPE == "all" ]; then
        execute_test $COMPONENT component
    fi
    if [ $GCOV_SCOPE == "integration" ] || [ $GCOV_SCOPE == "all" ]; then
        execute_test $COMPONENT integration
    fi
done

# do not start RouDi while the module and componenttests are running;
# they might do things which hurts RouDi, like in the roudi_shm test where named semaphores are opened and closed

if [ $failed_tests != 0 ]
then
    echo "$failed_tests tests failed!"
fi
echo ">>>>>> Finished Running Iceoryx Tests <<<<<<"
# set return code to indicate test execution status (code = number of failed tests)
# this return code should not be interpreted as standard unix return code 
exit $failed_tests


