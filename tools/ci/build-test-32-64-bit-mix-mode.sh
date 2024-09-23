#!/bin/bash
# Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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

# This script builds iceoryx_hoofs und iceoryx_posh and executes all tests

set -e

COLOR_OFF='\033[0m'
COLOR_RED='\033[1;31m'
COLOR_GREEN='\033[1;32m'
COLOR_YELLOW='\033[1;33m'
COLOR_BLUE='\033[1;34m'

WORKSPACE=$(git rev-parse --show-toplevel)
cd ${WORKSPACE}

echo -e "${COLOR_BLUE}#${COLOR_OFF}"
echo -e "${COLOR_BLUE}# Building 32 bit binaries${COLOR_OFF}"
echo -e "${COLOR_BLUE}#${COLOR_OFF}"
tools/iceoryx_build_test.sh build-strict build-all examples experimental-32-64-bit-mix-mode 32-bit-x86 --build-dir build-32

echo -e "${COLOR_BLUE}#${COLOR_OFF}"
echo -e "${COLOR_BLUE}# Building 64 bit binaries${COLOR_OFF}"
echo -e "${COLOR_BLUE}#${COLOR_OFF}"
tools/iceoryx_build_test.sh build-strict build-all examples experimental-32-64-bit-mix-mode --build-dir build-64

echo -e "${COLOR_BLUE}#${COLOR_OFF}"
echo -e "${COLOR_BLUE}# Running 32 bit posh tests${COLOR_OFF}"
echo -e "${COLOR_BLUE}#${COLOR_OFF}"
build-32/posh/test/posh_moduletests
build-32/posh/test/posh_integrationtests

echo -e "${COLOR_BLUE}#${COLOR_OFF}"
echo -e "${COLOR_BLUE}# Running 64 bit posh tests${COLOR_OFF}"
echo -e "${COLOR_BLUE}#${COLOR_OFF}"
build-64/posh/test/posh_moduletests
build-64/posh/test/posh_integrationtests

ROUDI_APP=iox-roudi
ROUDI_CMD_32_BIT=build-32/${ROUDI_APP}
ROUDI_CMD_64_BIT=build-64/${ROUDI_APP}
ROUDI_LOG=/tmp/iceoryx-32-64-bit-test-roudi-log

PUBLISHER_APP=iox-cpp-publisher-with-options
PUBLISHER_CMD_32_BIT=build-32/iceoryx_examples/iceoptions/${PUBLISHER_APP}
PUBLISHER_CMD_64_BIT=build-64/iceoryx_examples/iceoptions/${PUBLISHER_APP}
PUBLISHER_LOG=/tmp/iceoryx-32-64-bit-test-publisher-log

SUBSCRIBER_APP=iox-cpp-subscriber-with-options
SUBSCRIBER_CMD_32_BIT=build-32/iceoryx_examples/iceoptions/${SUBSCRIBER_APP}
SUBSCRIBER_CMD_64_BIT=build-64/iceoryx_examples/iceoptions/${SUBSCRIBER_APP}
SUBSCRIBER_LOG=/tmp/iceoryx-32-64-bit-test-subscriber-log

function print_log()
{
    echo -e "${COLOR_BLUE}### ${ROUDI_APP} log${COLOR_OFF}"
    cat ${ROUDI_LOG}
    echo -e "${COLOR_BLUE}### log end"

    echo -e "${COLOR_BLUE}### ${PUBLISHER_APP} log${COLOR_OFF}"
    cat ${PUBLISHER_LOG}
    echo -e "${COLOR_BLUE}### log end"

    echo -e "${COLOR_BLUE}### ${SUBSCRIBER_APP} log${COLOR_OFF}"
    cat ${SUBSCRIBER_LOG}
    echo -e "${COLOR_BLUE}### log end"
}

function run_test()
{
    TEST_TIME_IN_SECONDS=10

    ROUDI_CMD=$1
    PUBLISHER_CMD=$2
    SUBSCRIBER_CMD=$3

    ${ROUDI_CMD} > ${ROUDI_LOG} &
    ROUDI_PID=$!

    ${PUBLISHER_CMD} > ${PUBLISHER_LOG} &
    PUBLISHER_PID=$!

    ${SUBSCRIBER_CMD} > ${SUBSCRIBER_LOG} &
    SUBSCRIBER_PID=$!

    echo -e "${COLOR_YELLOW}Running applications for ${TEST_TIME_IN_SECONDS} seconds ... ${COLOR_OFF}"
    sleep ${TEST_TIME_IN_SECONDS}

    kill ${SUBSCRIBER_PID}
    kill ${PUBLISHER_PID}
    kill ${ROUDI_PID}
    wait ${ROUDI_PID}

    echo -e "${COLOR_BLUE}## Check publisher result${COLOR_OFF}"
    EXPECTED_PUBLISHER_OUTPUT="sent value: 3"
    if cat ${PUBLISHER_LOG} | grep --fixed-string --quiet "${EXPECTED_PUBLISHER_OUTPUT}"; then
        echo -e "${COLOR_GREEN}Found '${EXPECTED_PUBLISHER_OUTPUT}' in publisher log!${COLOR_OFF}"
    else
        print_log

        echo -e "${COLOR_RED}Error! Could not find '${EXPECTED_PUBLISHER_OUTPUT}' in publisher log!${COLOR_OFF}"
        exit 1
    fi

    echo -e "${COLOR_BLUE}## Check subscriber result${COLOR_OFF}"
    EXPECTED_SUBSCRIBER_OUTPUT="got value: 3"
    if cat ${SUBSCRIBER_LOG} | grep --fixed-string --quiet "${EXPECTED_SUBSCRIBER_OUTPUT}"; then
        echo -e "${COLOR_GREEN}Found '${EXPECTED_SUBSCRIBER_OUTPUT}' in subscriber log!${COLOR_OFF}"
    else
        print_log

        echo -e "${COLOR_RED}Error! Could not find '${EXPECTED_SUBSCRIBER_OUTPUT}' in subscriber log!${COLOR_OFF}"
        exit 1
    fi
}

echo -e "${COLOR_BLUE}#${COLOR_OFF}"
echo -e "${COLOR_BLUE}# Running 32 bit iox-roudi with 32 bit publisher example and 64 bit subscriber example${COLOR_OFF}"
echo -e "${COLOR_BLUE}#${COLOR_OFF}"
run_test ${ROUDI_CMD_32_BIT} ${PUBLISHER_CMD_32_BIT} ${SUBSCRIBER_CMD_64_BIT}

echo -e "${COLOR_BLUE}#${COLOR_OFF}"
echo -e "${COLOR_BLUE}# Running 32 bit iox-roudi with 64 bit publisher example and 32 bit subscriber example${COLOR_OFF}"
echo -e "${COLOR_BLUE}#${COLOR_OFF}"
run_test ${ROUDI_CMD_32_BIT} ${PUBLISHER_CMD_64_BIT} ${SUBSCRIBER_CMD_32_BIT}

echo -e "${COLOR_BLUE}#${COLOR_OFF}"
echo -e "${COLOR_BLUE}# Running 64 bit iox-roudi with 32 bit publisher example and 64 bit subscriber example${COLOR_OFF}"
echo -e "${COLOR_BLUE}#${COLOR_OFF}"
run_test ${ROUDI_CMD_64_BIT} ${PUBLISHER_CMD_32_BIT} ${SUBSCRIBER_CMD_64_BIT}

echo -e "${COLOR_BLUE}#${COLOR_OFF}"
echo -e "${COLOR_BLUE}# Running 64 bit iox-roudi with 64 bit publisher example and 32 bit subscriber example${COLOR_OFF}"
echo -e "${COLOR_BLUE}#${COLOR_OFF}"
run_test ${ROUDI_CMD_64_BIT} ${PUBLISHER_CMD_64_BIT} ${SUBSCRIBER_CMD_32_BIT}
