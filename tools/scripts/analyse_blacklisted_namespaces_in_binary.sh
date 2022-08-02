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
#   ./analyse_allowed_namespaces_in_binary.sh objectfile
#=====================================================


BLACKLISTED_NAMESPACE=(foo)
OBJECTFILE=$1

MATCH_NAMESPACES="\(\([A-Za-z]\+::\)\+[A-Za-z]\+\)"

for NAMESPACE in ${BLACKLISTED_NAMESPACE[@]}; do
    nm -C $OBJECTFILE -f posix | \
    grep "::$NAMESPACE::" | \
    sed -n "s/$MATCH_NAMESPACES/\n\1\n/gp" | \
    grep "::$NAMESPACE::" | \
    sed -n "s/$MATCH_NAMESPACES.*/\1/gp" | \
    sort | \
    uniq
done
