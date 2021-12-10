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


# Runs the iceoryx docker.
# The docker will immediately start RouDi. It can then be interacted with by
# either binding a shell to it or connecting via screen.
#
# NOTE: The container must first be built. This can be done using build_and_run.sh
#

print_usage () {
        echo "Usage: $0 <shm-size> [<container_name>]"
}

SHM_SIZE=$1
if [ -z "$SHM_SIZE" ]; then
	SHM_SIZE=700M
fi

CONTAINER_NAME=$2
if [ -z "$2" ]; then
CONTAINER_NAME=RouDi
fi

docker run -it --rm --shm-size $SHM_SIZE --name $CONTAINER_NAME iceoryx:latest
