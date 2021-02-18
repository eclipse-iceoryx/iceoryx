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

# This script add useraccounts to the system for testing Access Control Lists in iceoryx
# Use at your own risk!

set -e

#====================================================================================================
#==== Step : Create local test users and groups for testing access control  =========================
#====================================================================================================


if [ $(getent group roudi_test1) ] && [ $(getent group roudi_test2) ] && [ $(getent group roudi_test3) ]; then
    echo "users and groups for testing already exist"
else
    if [ "$EUID" -ne 0 ]; then
        echo "Please run the script as root with sudo"
        exit 1
    fi
    USERS="roudi_test1 roudi_test2 roudi_test3"
    for USER in $USERS; do
        echo "adding users" $USER
        sudo useradd -M $USER # create user without home dir
        sudo usermod -L $USER # prevent login
    done
fi
