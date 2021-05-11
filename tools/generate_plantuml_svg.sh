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

# This script exports svg files from all *.puml files in a user-defined folder
# example usage: ./tools/generate_plantuml_svg.sh

set -e

WORKSPACE="$(git rev-parse --show-toplevel)"
PUML_DIR="$WORKSPACE/doc/design/diagrams"
EXPORT_DIR=${1:-$PUML_DIR}
PLANTUML_DIR="/tmp/plantuml-jar-mit-1.2021.5"

cd $WORKSPACE

if [ ! -f $PLANTUML_DIR/plantuml.jar ]; then
    echo "Downloading Plantuml..."
    wget -P /tmp https://downloads.sourceforge.net/project/plantuml/1.2021.5/plantuml-jar-mit-1.2021.5.zip
    cd /tmp
    unzip plantuml-jar-mit-1.2021.5.zip -d plantuml-jar-mit-1.2021.5
    cd $WORKSPACE
fi


if ! java -jar $PLANTUML_DIR/plantuml.jar -v &> /dev/null
then
    echo "plantuml could not be found"
    exit
fi

java -jar $PLANTUML_DIR/plantuml.jar -nometadata -tsvg $EXPORT_DIR/*
