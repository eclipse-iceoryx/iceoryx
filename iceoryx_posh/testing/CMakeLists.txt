# Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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
cmake_minimum_required(VERSION 3.16)

find_package(iceoryx_hoofs_testing)
#
######### posh roudi environment ##########
#
iox_add_library(
    STATIC
    TARGET                  iceoryx_posh_testing
    NAMESPACE               iceoryx_posh_testing
    PROJECT_PREFIX          ${PREFIX}
    BUILD_INTERFACE         ${CMAKE_CURRENT_SOURCE_DIR}/include
    INSTALL_INTERFACE       include/${PREFIX}
    PRIVATE_LIBS            iceoryx_posh::iceoryx_posh
                            iceoryx_hoofs::iceoryx_hoofs
                            iceoryx_posh::iceoryx_posh_roudi
                            iceoryx_hoofs_testing::iceoryx_hoofs_testing
    FILES
        roudi_environment/runtime_test_interface.cpp
        roudi_environment/roudi_environment.cpp
)
