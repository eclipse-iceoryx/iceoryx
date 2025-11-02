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

# Usage: Adapt first VERSION file and run the script afterwards.

set -e

WORKSPACE=$(git rev-parse --show-toplevel)

echo "Did you update the changelogs?"
select yn in "Yes" "No"; do
    case $yn in
        Yes ) break;;
        No ) exit 0;;
    esac
done

echo "Did you update the VERSION file with the new version number?"
select yn in "Yes" "No"; do
    case $yn in
        Yes ) break;;
        No ) exit 0;;
    esac
done

echo "Did you update the version number in MODULE.bazel?"
select yn in "Yes" "No"; do
    case $yn in
        Yes ) break;;
        No ) exit 0;;
    esac
done

target_version=$(head -n 1 "$WORKSPACE/VERSION")
echo "The new version number will be: $target_version"

find "$WORKSPACE" -name "*.xml" -type f -exec sed -i 's/<version>\(.*\)<\/version>/<version>'"$target_version"'<\/version>/g' {} \;

find "$WORKSPACE" -name "CMakeLists.txt" -type f -exec sed -i 's/set(IOX_VERSION_STRING "\(.*\)")/set\(IOX_VERSION_STRING "'"$target_version"'"\)/g' {} \;

find "$WORKSPACE" -name "*.cmake" -type f -exec sed -i 's/set(IOX_VERSION_STRING "\(.*\)")/set\(IOX_VERSION_STRING "'"$target_version"'"\)/g' {} \;

echo "The necessary files are modified with the new version number. Please commit and merge them."
echo "When this is done, you can set the git tag and create a new Github release."
