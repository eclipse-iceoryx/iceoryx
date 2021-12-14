#!/bin/bash

# Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#
# Binds a shell to a running iceoryx docker container.
#

print_usage () {
        echo "Usage: $0 [<container_name>]"
}

CONTAINER_NAME=$1
if [ -z "$1" ]; then
CONTAINER_NAME=RouDi
fi

if [ ! "$(docker inspect -f '{{.State.Running}}' $CONTAINER_NAME 2> /dev/null)" ]; then
    echo "iceoryx docker container not running. Did you forget to start it ?"
    exit
fi

echo "Binding to container: $CONTAINER_NAME"

docker exec -it $CONTAINER_NAME /bin/bash
