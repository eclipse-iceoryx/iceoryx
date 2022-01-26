#!/bin/bash

# Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

CONTAINER_NAME="ice_env"

setup_docker_image() {
    apt update
    echo "Europe/Berlin" > /etc/timezone
    ln -sf /usr/share/zoneinfo/Europe/Berlin /etc/localtime
    apt -y install libbison-dev g++ gcc sudo cmake git fish gdb
    echo "PATH=$PATH:/iceoryx/tools/ci:/iceoryx/scripts/" >> /etc/profile
    exit
}

start_docker_session() {
    bash
    exit
}

help() {
    echo
    echo "iceoryx development environment help"
    echo
    echo "  $0 [ACTION] (optional)[OS_VERSION]"
    echo
    echo "ACTION:"
    echo "  start          - start iceoryx development environment"
    echo "  stop           - stops the iceoryx development environment"
    echo "  enter          - enters the iceoryx development environment, if it is"
    echo "                   not running it will be started first"
    echo
    echo "OS_VERSION:"
    echo "  A string which will be forwarded to \"-t\" in the docker command."
    echo "  The version of operating system to load. Default value is ubuntu:20.04."
    echo "  Other possibilities (not all) are:"
    echo "    ubuntu:18.04"
    echo "    archlinux"
    echo
    exit
}

start_docker() {
    if [[ $(docker container inspect -f '{{.State.Running}}' $CONTAINER_NAME) == "true" ]]; then
        echo iceoryx development environment already running \(docker container: $CONTAINER_NAME\)
        exit
    fi

    docker run --name $CONTAINER_NAME --mount type=bind,source="/dev",target=/dev \
               --mount type=bind,source=${ICEORYX_PATH},target=/iceoryx \
               --mount type=bind,source=/tmp,target=/tmp \
               -dt ${OS_VERSION}
    echo iceoryx development environment started

    docker exec -it $CONTAINER_NAME /iceoryx/$0 setup $OS_VERSION 

    echo
    echo "  iceoryx development environment setup and started"
    echo "  #################################################"
    echo
    echo "    OS-Version..............: ${OS_VERSION}"
    echo "    iceoryx-path............: ${ICEORYX_PATH}"
    echo
}

stop_docker() {
    docker container stop $CONTAINER_NAME
    docker rm $CONTAINER_NAME
    echo iceoryx development environment stopped
}

enter_docker() {
    if [[ $(docker container inspect -f '{{.State.Running}}' $CONTAINER_NAME) != "true" ]]; then
        start_docker
    fi

    docker exec -it $CONTAINER_NAME fish
}

ACTION=$1
OS_VERSION=$2
ICEORYX_PATH=$(pwd)

if [[ -z $OS_VERSION ]]; then
    OS_VERSION="ubuntu:20.04"
fi

if [[ $ACTION == "start" ]]; then
    start_docker
elif [[ $ACTION == "stop" ]]; then
    stop_docker
elif [[ $ACTION == "enter" ]]; then
    enter_docker
elif [[ $ACTION == "setup" ]]; then
    setup_docker_image
else 
    help
fi
