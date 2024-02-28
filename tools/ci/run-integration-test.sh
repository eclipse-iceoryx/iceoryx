#!/bin/bash
# Copyright (c) 2021, 2023 by Apex.AI Inc. All rights reserved.
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

# This script builds iceoryx_hoofs und iceoryx_posh and executes all tests

set -e

msg() {
    printf "\033[1;32m%s: %s\033[0m\n" "${FUNCNAME[1]}" "$1"
}

WORKSPACE=$(git rev-parse --show-toplevel)
cd "${WORKSPACE}"

msg "installing dependencies"
# NOTE: github action ros-tooling/setup-ros should be run before
sudo apt install -y apt-transport-https
sudo apt update && sudo apt install -y cmake libacl1-dev libncurses5-dev pkg-config

msg "sourcing ROS workspace"
# shellcheck source=/dev/null
source /opt/ros/humble/setup.bash

msg "checking copyrights"
sudo rm -rf /opt/ros/humble/lib/python3.10/site-packages/ament_copyright/template/apache2_header.txt
sudo cp -rf tools/apache2_header.txt /opt/ros/humble/lib/python3.10/site-packages/ament_copyright/template/.
# shellcheck disable=SC2026
sudo sed -i '41 c\"'c'", "'cc'", "'cpp'", "'cxx'", "'h'", "'hh'", "'hpp'", "'hxx'", "'inl'", "'sh'"' /opt/ros/humble/lib/python3.10/site-packages/ament_copyright/main.py
ament_copyright ./**/* tools/apache2_header.txt

msg "compiler versions:
$(gcc --version)
$(clang --version)"

msg "building"
rm -rf iceoryx_examples/COLCON_IGNORE iceoryx_integrationtest/COLCON_IGNORE
colcon build --packages-up-to iceoryx_integrationtest --meta=iceoryx_integrationtest/colcon.meta

msg "executing tests"
# shellcheck source=/dev/null
source ./install/setup.bash
colcon test --packages-select iceoryx_integrationtest --meta=iceoryx_integrationtest/colcon.meta
colcon test-result --all --verbose
