#!/usr/bin/env bash

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

ICEORYX_ROOT_PATH=$(git rev-parse --show-toplevel)
EXIT_CODE=0

performCmakeLinting()
{
    NUMBER_OF_FILES=$(find $ICEORYX_ROOT_PATH -type f -name "CMakeLists.txt" | grep -v ${ICEORYX_ROOT_PATH}/build | grep -v ${ICEORYX_ROOT_PATH}/.github | grep -v ${ICEORYX_ROOT_PATH}/.git | wc -l)

    CURRENT_FILE=0
    for FILE in $(find $ICEORYX_ROOT_PATH -type f -iname "CMakeLists.txt" | grep -v ${ICEORYX_ROOT_PATH}/build | grep -v ${ICEORYX_ROOT_PATH}/.github | grep -v ${ICEORYX_ROOT_PATH}/.git)
    do
        let CURRENT_FILE=$CURRENT_FILE+1
        echo -e "[$CURRENT_FILE/$NUMBER_OF_FILES] $FILE"

        if ! [[ $(cat $FILE | grep -i add_library | grep -v iox_add_library | wc -l) == "0" ]]
        then
            echo "  please do not use the cmake command \"add_library\", use \"iox_add_library\" instead"
            EXIT_CODE=1
        fi

        if ! [[ $(cat $FILE | grep -i add_executable | grep -v iox_add_executable | wc -l) == "0" ]]
        then
            echo "  please do not use the cmake command \"add_executable\", use \"iox_add_executable\" instead"
            EXIT_CODE=1
        fi
    done
}

performCmakeLinting

exit $EXIT_CODE
