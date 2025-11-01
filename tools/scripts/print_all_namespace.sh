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


# Print all namespaces used in iceoryx

COMPONENTS=(iceoryx_posh iceoryx_hoofs iceoryx_binding_c)
SOURCE_DIR=(source include)
WORKSPACE=$(git rev-parse --show-toplevel)

for COMPONENT in ${COMPONENTS[@]}; do
    for DIR in ${SOURCE_DIR[@]}; do
        GREP_PATH="${GREP_PATH} ${WORKSPACE}/${COMPONENT}/$DIR"
    done
done

grep -RIn -P '(^|\W{2})namespace\s+(\w+)\s*|$\{' $GREP_PATH | sed -n s/.*\namespace*//p  | sort | uniq
