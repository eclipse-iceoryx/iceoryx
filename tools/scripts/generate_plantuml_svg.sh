#!/usr/bin/env bash

# Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

# This script exports svg files from all *.puml files in a user-defined folder
# example usage: ./tools/generate_plantuml_svg.sh

set -e

declare -a REQUIRED_PROGRAMS=("plantuml" "git")

STARTUP_CHECK_SUCCESS=true
verifyProgram() {
    if ! command -v $1 &>/dev/null
    then
        echo Please install \"$1\" to use this script
        STARTUP_CHECK_SUCCESS=false
    fi
}

checkRequiredPrograms() {
    for p in "${REQUIRED_PROGRAMS[@]}"
    do
        verifyProgram $p
    done

    if ! $STARTUP_CHECK_SUCCESS
    then
        echo Please install missing applications
        exit
    fi
}

checkRequiredPrograms

WORKSPACE="$(git rev-parse --show-toplevel)"
PUML_DIR="$WORKSPACE/doc/design/diagrams"
IMPORT_DIR=${1:-$PUML_DIR}
EXPORT_DIR="$WORKSPACE/doc/website/images/"
NUM_THREADS=1

cd "$WORKSPACE"

# set number of cores for building
if [[ "$OSTYPE" == "linux-gnu"* ]] || [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "msys" ]]; then
    NUM_THREADS=$(nproc)
elif [[ "$OSTYPE" == "darwin"* ]]; then
    NUM_THREADS=$(sysctl -n hw.ncpu)
fi
echo " [i] Generating with $NUM_THREADS threads"

plantuml \
     -config "$WORKSPACE/doc/iceoryx-plantuml-config.puml" \
     -nometadata \
     -nbthread "$NUM_THREADS" \
     -tsvg \
     -I "$IMPORT_DIR/**.puml" \
     -output "$EXPORT_DIR"

echo " [i] Finished"
