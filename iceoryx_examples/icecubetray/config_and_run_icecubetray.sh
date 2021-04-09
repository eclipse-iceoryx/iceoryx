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

WORKSPACE=$(git rev-parse --show-toplevel)
CONFIG=${1:-config}
RUN=${2:-run}
SESSION=iceensemble
tmux="tmux -2 -q"

if [ "$CONFIG" == "config" ] ; then
    # Create groups
    sudo groupadd -f privileged
    sudo groupadd -f unprivileged
    sudo groupadd -f iceoryx

    # Create users w/o homedir
    sudo useradd -M perception
    sudo useradd -M infotainment
    sudo useradd -M roudi
    sudo useradd -M notallowed

    # Assign users to group and disable login
    sudo usermod -L perception -a -G privileged
    sudo usermod -L infotainment -a -G unprivileged
    sudo usermod -L perception -a -G iceoryx
    sudo usermod -L infotainment -a -G iceoryx
    sudo usermod -L roudi -a -G iceoryx
    sudo usermod -L notallowed -a -G iceoryx
fi
# If you're using e.g Yocto or QNX refer to the manual on how to set up groups, users and permissions


if [ "$RUN" == "run" ] ; then
    # Allow RouDi to send SIGKILL to other apps
    sudo setcap cap_kill=ep ./build/iceoryx_examples/icecubetray/iox-cpp-roudi-static-segments

    $tmux kill-server

    $tmux has-session -t $SESSION
    if [ $? -eq 0 ]; then
        echo "Session $SESSION already exists. Attaching to session."
        $tmux attach -t $SESSION
        exit 0;
    fi

    command -v tmux >/dev/null 2>&1 || { echo >&2 "tmux is not installed but required. Trying to install it..."; sudo apt-get install tmux; }

    $tmux new-session -d -s $SESSION
    # Start custom RouDi in 'iceoryx' group
    $tmux new-window -a -t $SESSION 'sudo -u roudi -g iceoryx -- $WORKSPACE/build/iceoryx_examples/icecubetray/iox-cpp-roudi-static-segments'

    # Start perception app as 'perception' user
    $tmux split-window -t 0 -h 'sudo -u perception -g iceoryx -- $WORKSPACE/build/iceoryx_examples/icecubetray/iox-cpp-radar'

    # Start display app as 'infotainment' user
    $tmux split-window -t 1 -h 'sudo -u infotainment -g iceoryx -- $WORKSPACE/build/iceoryx_examples/icecubetray/iox-cpp-display'

    # Start cheeky app as 'notallowed' user
    $tmux split-window -t 0 -h 'sudo -u notallowed -g iceoryx -- $WORKSPACE/build/iceoryx_examples/icecubetray/iox-cpp-cheeky'

    $tmux attach -t $SESSION
fi
