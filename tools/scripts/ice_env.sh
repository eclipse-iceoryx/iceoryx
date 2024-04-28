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
CONTAINER_MEMORY_SIZE="4g"
CONTAINER_SHM_MEMORY_SIZE="2g"
DEFAULT_OS_VERSION="ubuntu:20.04"

setup_docker_image() {
    echo "Europe/Berlin" > /etc/timezone
    ln -sf /usr/share/zoneinfo/Europe/Berlin /etc/localtime

    local BASE_OS_VERSION=$(echo $OS_VERSION | sed -n "s/\([^\:]*\).*/\1/p")

    if [[ $BASE_OS_VERSION == "ubuntu" ]]; then
        apt update
        apt -y install libbison-dev g++ gcc sudo cmake git fish gdb lldb llvm clang clang-format
    elif [[ $BASE_OS_VERSION == "archlinux" ]]; then
        pacman -Syu --noconfirm base base-devel clang cmake git fish gdb lldb llvm
    else
        echo Please install the following packages to have a working iceoryx environment
        echo libbison-dev g++ gcc sudo cmake git fish gdb lldb llvm clang clang-format
    fi

    mkdir -p /root/.config/fish
    echo "set -gx PATH /iceoryx/tools/ci /iceoryx/scripts \$PATH" >> /root/.config/fish/config.fish
    echo "set -gx ASAN_OPTIONS 'symbolize=1,detect_leaks=1,abort_on_error=1,quarantine_size_mb=8192'" >> /root/.config/fish/config.fish
    echo "set -gx UBSAN_OPTIONS 'print_stacktrace=1'" >> /root/.config/fish/config.fish
    echo "set -gx ASAN_SYMBOLIZER_PATH '/usr/bin/llvm-symbolizer'" >> /root/.config/fish/config.fish

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
    echo "  The version of operating system to load. Default value is ${DEFAULT_OS_VERSION}."
    echo "  Other possibilities (not all) are:"
    echo "    ubuntu:18.04"
    echo "    archlinux"
    echo
    echo "Example:"
    echo "  $0 start archlinux     # starts iceoryx environment with archlinux docker container"
    echo "  $0 enter ubuntu:18.04  # enters (and starts if not running) iceoryx environment with ubuntu"
    echo
    exit
}

start_docker() {
    local ICEORYX_PATH=$(git rev-parse --show-toplevel)
    if [[ $(docker container inspect -f '{{.State.Running}}' $CONTAINER_NAME 2>/dev/null) == "true" ]]; then
        echo iceoryx development environment already running \(docker container: $CONTAINER_NAME\)
        exit
    fi

    docker run --name $CONTAINER_NAME \
               --mount type=bind,source=${ICEORYX_PATH},target=/iceoryx \
               --hostname ${OS_VERSION} \
               --network host \
               -dt --memory $CONTAINER_MEMORY_SIZE \
               --shm-size $CONTAINER_SHM_MEMORY_SIZE ${OS_VERSION}
    echo iceoryx development environment started

    docker exec -it $CONTAINER_NAME /iceoryx/$(git rev-parse --show-prefix)/$0 setup $OS_VERSION

    echo
    echo "  iceoryx development environment setup and started"
    echo "  #################################################"
    echo
    echo "    container name..........: ${CONTAINER_NAME}"
    echo "    OS-Version..............: ${OS_VERSION}"
    echo "    memory..................: ${CONTAINER_MEMORY_SIZE}"
    echo "    shared memory...........: ${CONTAINER_SHM_MEMORY_SIZE}"
    echo "    iceoryx-path............: ${ICEORYX_PATH}"
    echo
}

stop_docker() {
    docker container stop $CONTAINER_NAME > /dev/null
    docker rm $CONTAINER_NAME > /dev/null
    echo iceoryx development environment stopped
}

enter_docker() {
    if [[ $(docker container inspect -f '{{.State.Running}}' $CONTAINER_NAME) != "true" ]]; then
        start_docker
    fi

    docker exec -it $CONTAINER_NAME fish -c "
    echo
    # we use eval here since we would like to evaluate the expression inside of the docker
    # container and not right away in this script
    eval 'echo \"  gcc version..............: \"(gcc --version | head -1 )'
    eval 'echo \"  g++ version..............: \"(g++ --version | head -1 )'
    eval 'echo \"  clang version............: \"(clang --version | head -1 )'
    eval 'echo \"  clang++ version..........: \"(clang++ --version | head -1 )'
    eval 'echo \"  cmake version............: \"(cmake --version | head -1 )'
    echo
    cd /iceoryx
    fish"
}

ACTION=$1
OS_VERSION=$2

if [[ -z $OS_VERSION ]]; then
    OS_VERSION=$DEFAULT_OS_VERSION
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
