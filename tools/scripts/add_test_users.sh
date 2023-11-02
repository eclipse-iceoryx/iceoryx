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
COMMAND=$1 # When called with argument "check" the script only look if the users exist
USERS="iox_roudi_test1 iox_roudi_test2 iox_roudi_test3"

for USER in ${USERS}  ; do
    if [ "$(getent group ${USER})" ]; then
        echo "${USER} already exist, skipping"
    else
        if [ "$EUID" -ne 0 ] || [ "$COMMAND" == "check" ]; then
            echo "Warning: ${USER} was not found on system"
            echo "Warning: Tests are running with user accounts 'iox_roudi_testX', please make sure that add_test_users.sh has run as sudo before."
            exit 1
        fi
        echo "adding users" $USER
        useradd -M $USER # create user without home dir
        usermod -L $USER # prevent login
    fi
done




