#!/bin/bash

# Copyright (c) 2020 by ApexAI Inc. All rights reserved.
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

set -e

WORKSPACE=$(git rev-parse --show-toplevel)
WEBREPO="git@github.com:eclipse-iceoryx/iceoryx-web.git"
VERSION=$1

cd $WORKSPACE

# Generate doxygen, replace with CMake commands later
cd  $WORKSPACE/iceoryx_utils/doc/
doxygen doxyfile-utils
cd  $WORKSPACE/iceoryx_posh/doc/
doxygen doxyfile-posh

# Generate markdown from doxygen
cd $WORKSPACE
mkdir -p $WORKSPACE/doc/website/API-reference/utils
doxybook2 --input $WORKSPACE/iceoryx_utils/doc/xml/ --output $WORKSPACE/doc/website/API-reference/utils
mkdir -p $WORKSPACE/doc/website/API-reference/posh
doxybook2 --input $WORKSPACE/iceoryx_posh/doc/xml/ --output $WORKSPACE/doc/website/API-reference/posh

# Generate HTML and push to GitHub pages
if [ ! -d "$WORKSPACE/../iceoryx-web" ]; then
    cd $WORKSPACE/../
    git clone $WEBREPO
fi
cd $WORKSPACE/../iceoryx-web
mkdocs gh-deploy --config-file ../iceoryx/mkdocs.yml --remote-branch $VERSION
