#!/bin/bash

# Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Apache Software License 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0, or the MIT license
# which is available at https://opensource.org/licenses/MIT.
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT


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
