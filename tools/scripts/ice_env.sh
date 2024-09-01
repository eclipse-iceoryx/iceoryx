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

CONTAINER_NAME_PREFIX="ice_env_"
CONTAINER_MEMORY_SIZE="8g"
CONTAINER_SHM_MEMORY_SIZE="2g"
DEFAULT_OS_VERSION="ubuntu:22.04"
CMAKE_VERSION="cmake-3.23.1-linux-x86_64"
CLANG_VERSION="15"
ICEORYX_PATH=$(git rev-parse --show-toplevel)

COLOR_RESET='\033[0m'
COLOR_GREEN='\033[1;32m'
COLOR_CYAN='\033[1;34m'
FONT_BOLD='\033[1m'
COLOR_RED='\033[1;31m'

install_cmake() {
    cd /
    wget https://github.com/Kitware/CMake/releases/download/v3.23.1/${CMAKE_VERSION}.tar.gz
    tar xf ${CMAKE_VERSION}.tar.gz
}

set_new_default_clang_binary() {
    local CLANG_BINARY_PATH=$1
    rm -rf ${CLANG_BINARY_PATH}
    ln -s ${CLANG_BINARY_PATH}-${CLANG_VERSION} ${CLANG_BINARY_PATH}
}

setup_docker_image() {
    echo "Europe/Berlin" > /etc/timezone
    ln -sf /usr/share/zoneinfo/Europe/Berlin /etc/localtime

    # ubuntu/debian and derivatives
    if command -v apt &>/dev/null; then
        dpkg --add-architecture i386
        apt update
        apt -y install g++ gcc sudo cmake git fish gdb lldb llvm clang clang-format wget libncurses5-dev libacl1-dev wget lsb-release software-properties-common vim
        apt -y install libacl1-dev:i386 libc6-dev-i386 libc6-dev-i386-cross libstdc++6-i386-cross gcc-multilib g++-multilib
        install_cmake

        # install newest clang
        wget https://apt.llvm.org/llvm.sh
        chmod +x llvm.sh
        # patch so that no interaction is required
        sed -i 's/^add-apt-repository/add-apt-repository -y/g' llvm.sh
        ./llvm.sh ${CLANG_VERSION} all

        # set newest clang as default
        set_new_default_clang_binary /usr/bin/clang
        set_new_default_clang_binary /usr/bin/clang++
        set_new_default_clang_binary /usr/bin/clang-tidy
        set_new_default_clang_binary /usr/bin/clang-format

    # archlinux based ones
    elif command -v pacman &>/dev/null; then
        echo "[multilib]" >> /etc/pacman.conf
        echo "Include = /etc/pacman.d/mirrorlist" >> /etc/pacman.conf
        pacman -Syu --noconfirm base base-devel clang cmake git fish gdb lldb llvm wget ncurses vim
        pacman -Syu --noconfirm lib32-acl lib32-gcc-libs lib32-ncurses
        install_cmake
    else
        echo Please install the following packages to have a working iceoryx environment:
        echo g++ gcc sudo cmake git fish gdb lldb llvm clang clang-format ncurses
    fi

    git config --global --add safe.directory /iceoryx

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
    echo -e "${FONT_BOLD}iceoryx development environment help${COLOR_RESET}"
    echo
    echo -e "  $0 ${COLOR_CYAN}[ACTION] ${COLOR_RESET}(optional)${COLOR_CYAN}[OS_VERSION]"
    echo
    echo -e "${COLOR_CYAN}ACTION:${COLOR_RESET}"
    echo -e "  ${FONT_BOLD}start${COLOR_RESET}          - start iceoryx development environment"
    echo -e "  ${FONT_BOLD}stop${COLOR_RESET}           - stops the iceoryx development environment"
    echo -e "  ${FONT_BOLD}stop_all${COLOR_RESET}       - stop all iceoryx development environments"
    echo -e "  ${FONT_BOLD}enter${COLOR_RESET}          - enters the iceoryx development environment, if it is"
    echo -e "                   not running it will be started first"
    echo -e "  ${FONT_BOLD}drop${COLOR_RESET}           - remove the iceoryx environment docker container with the"
    echo -e "                   specified OS_VERSION"
    echo -e "  ${FONT_BOLD}drop_all${COLOR_RESET}       - remove all iceoryx environment docker containers"
    echo -e "  ${FONT_BOLD}list${COLOR_RESET}           - list all locally iceoryx environment docker containers"
    echo -e "  ${FONT_BOLD}list_running${COLOR_RESET}   - list all running iceoryx environment docker containers"
    echo
    echo -e "${COLOR_CYAN}OS_VERSION:${COLOR_RESET}"
    echo "  A string which will be forwarded to \"-t\" in the docker command."
    echo "  The version of operating system to load. Default value is ${DEFAULT_OS_VERSION}."
    echo "  Other possibilities (not all) are:"
    echo "    ros:rolling"
    echo "    ubuntu:18.04"
    echo "    archlinux"
    echo
    echo -e "${COLOR_CYAN}Example:${COLOR_RESET}"
    echo "  $0 start archlinux     # starts iceoryx environment with archlinux docker container"
    echo "  $0 enter ubuntu:18.04  # enters (and starts if not running) iceoryx environment with ubuntu"
    echo
    exit
}

create_docker() {
    echo -e "  ${COLOR_CYAN}create iceoryx development environment docker container${COLOR_RESET} [${FONT_BOLD}$CONTAINER_NAME${COLOR_RESET}]"
    docker run --name $CONTAINER_NAME \
               --mount type=bind,source=${ICEORYX_PATH},target=/iceoryx \
               --hostname ${OS_VERSION} \
               -dt --memory $CONTAINER_MEMORY_SIZE \
               --shm-size $CONTAINER_SHM_MEMORY_SIZE ${OS_VERSION}
    echo -e "  ${COLOR_CYAN}setting up iceoryx development environment${COLOR_RESET} [${FONT_BOLD}$CONTAINER_NAME${COLOR_RESET}]"

    docker exec -it $CONTAINER_NAME /iceoryx/$(realpath $0 --relative-to=$ICEORYX_PATH) setup $OS_VERSION
}

startup_docker() {
    echo -en "         start iceoryx development environment docker container [${FONT_BOLD}$CONTAINER_NAME${COLOR_RESET}]"
    docker start $CONTAINER_NAME > /dev/null
    echo -e "\r  [${COLOR_GREEN}done${COLOR_RESET}]"
}

list_docker() {
    docker container ls -a | sed -n "s/.*\(ice_env_.*\)/  \1/p"
}

list_running_docker() {
    docker container ls | sed -n "s/.*\(ice_env_.*\)/  \1/p"
}

start_docker() {
    if [[ $(docker container inspect -f '{{.State.Running}}' $CONTAINER_NAME 2> /dev/null) == "true" ]]; then
        return
    fi

    if [[ $(list_docker | grep ${CONTAINER_NAME} | wc -l) == "0" ]]; then
        create_docker
    else
        startup_docker
    fi

    echo
    echo -e "  ${COLOR_CYAN}iceoryx development environment setup and started${COLOR_RESET}"
    echo -e "  #################################################"
    echo
    echo -e "    container name..........: ${FONT_BOLD}${CONTAINER_NAME}${COLOR_RESET}"
    echo -e "    OS-Version..............: ${FONT_BOLD}${OS_VERSION}${COLOR_RESET}"
    echo -e "    memory..................: ${FONT_BOLD}${CONTAINER_MEMORY_SIZE}${COLOR_RESET}"
    echo -e "    shared memory...........: ${FONT_BOLD}${CONTAINER_SHM_MEMORY_SIZE}${COLOR_RESET}"
    echo -e "    iceoryx-path............: ${FONT_BOLD}${ICEORYX_PATH}${COLOR_RESET}"
    echo
    echo -e "  A custom cmake version was installed in ${FONT_BOLD}${CMAKE_VERSION}/bin/cmake${COLOR_RESET}."
    echo "  This can be used when the cmake version in the image is out-of-date."
    echo
}

stop_docker() {
    if [[ $(docker container inspect -f '{{.State.Running}}' $CONTAINER_NAME) == "true" ]]; then
        echo -en "         stopping iceoryx development environment docker [${FONT_BOLD}${CONTAINER_NAME}${COLOR_RESET}] container"
        docker container stop $CONTAINER_NAME > /dev/null
        echo -e "\r  [${COLOR_GREEN}done${COLOR_RESET}]"
    fi
}

stop_all_docker() {
    echo -e "${COLOR_CYAN}stopping all iceoryx environment docker containers${COLOR_RESET}"
    for DOCKER in $(list_running_docker); do
        CONTAINER_NAME=$DOCKER
        stop_docker
    done
}

drop_docker() {
    stop_docker
    echo -en "         removing iceoryx development environment docker [${FONT_BOLD}${CONTAINER_NAME}${COLOR_RESET}] container"
    docker rm $CONTAINER_NAME > /dev/null
    echo -e "\r  [${COLOR_GREEN}done${COLOR_RESET}]"
}

drop_all_docker() {
    echo -e "${COLOR_RED}removing all iceoryx environment docker containers${COLOR_RESET}"
    for DOCKER in $(list_docker); do
        CONTAINER_NAME=$DOCKER
        drop_docker
    done
}

enter_docker() {
    start_docker

    # we use eval here since we would like to evaluate the expression inside of the docker
    # container and not right away in this script
    docker exec -it $CONTAINER_NAME fish -c "
    echo
    eval 'echo -e \"  gcc version..............: \"\\033\[1\;37m(gcc --version | head -1 )\\033\[0m'
    eval 'echo -e \"  g++ version..............: \"\\033\[1\;37m(g++ --version | head -1 )\\033\[0m'
    eval 'echo -e \"  clang version............: \"\\033\[1\;37m(clang --version | head -1 )\\033\[0m'
    eval 'echo -e \"  clang++ version..........: \"\\033\[1\;37m(clang++ --version | head -1 )\\033\[0m'
    eval 'echo -e \"  cmake version............: \"\\033\[1\;37m(cmake --version | head -1 )\\033\[0m'
    echo
    cd /iceoryx
    fish"

    # we use eval here since we would like to evaluate the expression inside of the docker
    # container and not right away in this script
    if [[ $? -ne 0 ]]; then
        docker exec -it $CONTAINER_NAME bash -c "
        echo
        eval 'echo \"  gcc version..............: \"\\033\[1\;37m(gcc --version | head -1 )\\033\[0m'
        eval 'echo \"  g++ version..............: \"\\033\[1\;37m(g++ --version | head -1 )\\033\[0m'
        eval 'echo \"  clang version............: \"\\033\[1\;37m(clang --version | head -1 )\\033\[0m'
        eval 'echo \"  clang++ version..........: \"\\033\[1\;37m(clang++ --version | head -1 )\\033\[0m'
        eval 'echo \"  cmake version............: \"\\033\[1\;37m(cmake --version | head -1 )\\033\[0m'
        echo
        cd /iceoryx
        bash
        "
    fi
}

ACTION=$1
OS_VERSION=$2

if [[ -z $OS_VERSION ]]; then
    OS_VERSION=$DEFAULT_OS_VERSION
fi

CONTAINER_NAME=${CONTAINER_NAME_PREFIX}$(echo ${OS_VERSION} | tr : .)

if [[ $ACTION == "start" ]]; then
    start_docker
elif [[ $ACTION == "stop" ]]; then
    stop_docker
elif [[ $ACTION == "stop_all" ]]; then
    stop_all_docker
elif [[ $ACTION == "drop" ]]; then
    drop_docker
elif [[ $ACTION == "drop_all" ]]; then
    drop_all_docker
elif [[ $ACTION == "enter" ]]; then
    enter_docker
elif [[ $ACTION == "setup" ]]; then
    setup_docker_image
elif [[ $ACTION == "list" ]]; then
    list_docker
elif [[ $ACTION == "list_running" ]]; then
    list_running_docker
else
    help
fi
