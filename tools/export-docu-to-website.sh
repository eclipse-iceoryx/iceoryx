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
#
# SPDX-License-Identifier: Apache-2.0

# Necessary tools:
# mkdocs-awesome-pages-plugin, v2.5.0
# mkdocs-material, v6.2.3
# markdown-include, v0.6.0
# mkdocs, v1.1.2
# Doxygen, 1.8.17
# doxybook2, v1.3.1
# mike, v0.5.5

set -e

WORKSPACE=$(git rev-parse --show-toplevel)
WEBREPO="git@github.com:eclipse-iceoryx/iceoryx-web.git"
VERSION=$1
TYPE=${2:-local} #`local` starts a local webserver to inspect the results, `publish` pushes the generated doc to iceoryx_web

cd $WORKSPACE

# Generate doxygen
cmake -Bbuild -Hiceoryx_meta -DBUILD_DOC=ON
cd $WORKSPACE/build
make -j8 doxygen_iceoryx_posh doxygen_iceoryx_utils doxygen_iceoryx_binding_c doxygen_iceoryx_dds doxygen_iceoryx_introspection

# Generate markdown from doxygen
cd $WORKSPACE

mkdir -p $WORKSPACE/doc/website/API-reference/utils
doxybook2 --input $WORKSPACE/build/doc/iceoryx_utils/xml/ --output $WORKSPACE/doc/website/API-reference/utils

mkdir -p $WORKSPACE/doc/website/API-reference/posh
doxybook2 --input $WORKSPACE/build/doc/iceoryx_posh/xml/ --output $WORKSPACE/doc/website/API-reference/posh

mkdir -p $WORKSPACE/doc/website/API-reference/c-binding
doxybook2 --input $WORKSPACE/build/doc/iceoryx_binding_c/xml/ --output $WORKSPACE/doc/website/API-reference/c-binding

mkdir -p $WORKSPACE/doc/website/API-reference/DDS-gateway
doxybook2 --input $WORKSPACE/build/doc/iceoryx_dds/xml/ --output $WORKSPACE/doc/website/API-reference/DDS-gateway

mkdir -p $WORKSPACE/doc/website/API-reference/introspection
doxybook2 --input $WORKSPACE/build/doc/iceoryx_introspection/xml/ --output $WORKSPACE/doc/website/API-reference/introspection


if [ "$TYPE" == "local" ]; then
    echo "starting local webserver"
    mkdocs serve --config-file ../iceoryx/mkdocs.yml
fi

if [ "$TYPE" == "publish" ]; then
   # Generate HTML and push to GitHub pages
    if [ ! -d "$WORKSPACE/../iceoryx-web" ]; then
        cd $WORKSPACE/../
        git clone $WEBREPO
    fi
    cd $WORKSPACE/../iceoryx-web
    mkdocs gh-deploy --config-file ../iceoryx/mkdocs.yml --remote-branch $VERSION
fi

