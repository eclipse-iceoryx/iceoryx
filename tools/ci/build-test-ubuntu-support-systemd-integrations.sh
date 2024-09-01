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

# This script builds iceoryx_hoofs und iceoryx_posh and executes all tests

set -e

if [ "$USER" != "root" ]; then
    echo "Please run this as root or with sudo"
    exit 1
fi

msg() {
    printf "\033[1;32m%s: %s\033[0m\n" ${FUNCNAME[1]} "$1"
}

WORKSPACE=$(git rev-parse --show-toplevel)
DIR_BUILD="build"

msg "Create unit file"
echo -e "[Unit]\nDescription=Test application roudi\n\n[Service]\nType=notify\nRestartSec=10\nRestart=always\nExecStart=${WORKSPACE}/${DIR_BUILD}/iox-roudi\nTimeoutStartSec=10\nWatchdogSec=5\n\n[Install]\nWantedBy=multi-user.target" | tee /usr/lib/systemd/system/test_iox.service > /dev/null

msg "Show unit"
cat /usr/lib/systemd/system/test_iox.service

msg "Daemon reload"
systemctl daemon-reload

msg "Check status"
systemctl status test_iox || true

msg "Start roudi"
systemctl start test_iox || (echo "Failed to start service"; sudo journalctl -u test_iox -n 50; exit 1)

msg "Wait for 30 seconds"
sleep 30

msg "Check roudi"
systemctl status test_iox || (echo "Failed to start service"; sudo journalctl -u test_iox -n 50; exit 1)

msg "Stop roudi"
systemctl stop test_iox

msg "Show journal"
journalctl -u test_iox -n 100