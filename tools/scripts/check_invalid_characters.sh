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

# This script does a sanity check for invalid characters in C++ source code

set -e

ICEORYX_PATH=$(git rev-parse --show-toplevel)
cd $ICEORYX_PATH

function findInFiles() {
    searchString=$1
    grep -rn --include=\*.{h,hpp,inl,c,cpp} "\`" iceoryx_*
}

BACKTICK_SEARCH_STRING="\`"
NUMBER_OF_BACKTICKS=$(findInFiles $BACKTICK_SEARCH_STRING | wc -l)
if [[ "$NUMBER_OF_BACKTICKS" -gt "0" ]]; then
    echo -e "\e[1;31mFound invalid backtick character in the following file(s)!\e[m"
    echo -e "\e[1;31mPlease replace with a single quote!\e[m"
    echo -e "\e[1;31m\` -> '\e[m"
    findInFiles $BACKTICK_SEARCH_STRING
    exit 1
fi
