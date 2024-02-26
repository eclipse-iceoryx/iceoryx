# Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

# NOTE
# This package must NOT depend in any way on gtest or any other testing framework since
# it will be used from other languages like Rust for integration testing.

#
######### posh roudi env ##########
#
iox_add_library(
    STATIC
    TARGET                  iceoryx_posh_roudi_env
    NAMESPACE               iceoryx_posh
    PROJECT_PREFIX          ${PREFIX}
    BUILD_INTERFACE         ${CMAKE_CURRENT_SOURCE_DIR}/include
    INSTALL_INTERFACE       include/${PREFIX}
    EXPORT_INCLUDE_DIRS     include/
    PRIVATE_LIBS            iceoryx_posh::iceoryx_posh
                            iceoryx_hoofs::iceoryx_hoofs
                            iceoryx_posh::iceoryx_posh_roudi
    FILES
        source/minimal_iceoryx_config.cpp
        source/roudi_env.cpp
        source/roudi_env_node_builder.cpp
        source/runtime_test_interface.cpp
)
