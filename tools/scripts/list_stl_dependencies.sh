#!/bin/bash

# Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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


#=====================================================
# usage:
#   ./list_stl_dependencies.sh
#=====================================================

SCOPE=${1:-list}
COMPONENTS=(iceoryx_hoofs iceoryx_posh)
POSH_SOURCE_DIR=(source include)
HOOFS_SOURCE_DIR=(source include posix memory time vocabulary container)
WORKSPACE=$(git rev-parse --show-toplevel)
QNX_PLATFORM_DIR=$WORKSPACE/iceoryx_platform/qnx/
USELIST=$WORKSPACE/tools/scripts/used-headers.txt
CURRENTLY_USED_HEADERS=$(mktemp)
GET_HEADER_NAME="\<\K[^<>]+(?=>)" # Matches the content between angle brackets

for COMPONENT in ${COMPONENTS[@]}; do
if [[ "$COMPONENT" == "iceoryx_posh" ]]; then
    for DIR in ${POSH_SOURCE_DIR[@]}; do
        GREP_PATH_HOOFS_POSH="${GREP_PATH_HOOFS_POSH} ${WORKSPACE}/${COMPONENT}/$DIR"
    done
fi
if [[ "$COMPONENT" == "iceoryx_hoofs" ]]; then
    for DIR in ${HOOFS_SOURCE_DIR[@]}; do
        GREP_PATH_HOOFS_POSH="${GREP_PATH_HOOFS_POSH} ${WORKSPACE}/${COMPONENT}/$DIR"
    done
fi
done

echo "# QNX platform / libc headers" | tee -a $CURRENTLY_USED_HEADERS

# GCC can't preprocess .inl so we grep them in plain text
QNX_CPP_HPP_FILES=$(find $QNX_PLATFORM_DIR -type f -iname *.cpp -o -iname *.hpp)
QNX_INL_FILES=$(find $QNX_PLATFORM_DIR -type f -iname *.inl)

{ gcc -w -fpreprocessed -dD -E $QNX_CPP_HPP_FILES; if [[ -n $QNX_INL_FILES ]]; then cat $QNX_INL_FILES; fi; } \
 | grep -e "#include <" \
 | grep -oP $GET_HEADER_NAME \
 | sort \
 | uniq \
 | tee -a $CURRENTLY_USED_HEADERS \
 | cat

echo

echo "# iceoryx_posh / iceoryx_hoofs headers" | tee -a $CURRENTLY_USED_HEADERS

# GCC can't preprocess .inl so we grep them in plain text
HOOFS_POSH_CPP_HPP_FILES=$(find $GREP_PATH_HOOFS_POSH -type f -iname *.cpp -o -iname *.hpp)
HOOFS_POSH_INL_FILES=$(find $GREP_PATH_HOOFS_POSH -type f -iname *.inl)

{ gcc -w -fpreprocessed -dD -E $HOOFS_POSH_CPP_HPP_FILES; if [[ -n $HOOFS_POSH_INL_FILES ]]; then cat $HOOFS_POSH_INL_FILES; fi; } \
 | grep -e "#include <" \
 | grep -oP $GET_HEADER_NAME \
 | sort \
 | uniq \
 | tee -a $CURRENTLY_USED_HEADERS \
 | cat

if [[ "$SCOPE" == "check" ]]; then
    echo
    echo "Comparing the used system headers against the list.."
    diff $CURRENTLY_USED_HEADERS $USELIST
    if [ $? -eq 1 ]; then
        echo "Mismatch of expected and found headers. Please check the diff above and remove/add the header in 'tools/scripts/used-headers.txt'!"
        exit 1
    fi
    echo "No header divergence found!"
fi

echo
echo "# usage of std components"
{ gcc -w -fpreprocessed -dD -E $(find $GREP_PATH_HOOFS_POSH -type f -iname *.cpp -o -iname *.hpp); cat $(find $GREP_PATH_HOOFS_POSH -type f -iname *.inl); } \
 | grep -HIne "std::" \
 | sed -n  "s/\([^:]*\:[0-9]*\)\:.*\(std::[a-zA-Z_]*\).*/\ \  \2/p" \
 | sort \
 | uniq

echo
echo "# files with stl dependency"
grep -RIne "std::" $GREP_PATH_HOOFS_POSH | sed -n  "s/\([^:]*\)\:[0-9]*\:.*std::[a-zA-Z_]*.*/\1/p" | xargs -I{} basename {} | sort | uniq

echo
echo "# using namespace with std component"
grep -RIne ".*using\s*namespace.*std" $GREP_PATH_HOOFS_POSH | sed -n "s/\(.*\)/\ \ \1/p"

exit 0
