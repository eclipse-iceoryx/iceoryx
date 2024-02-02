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

#
######### posh testing ##########
#
find_package(GTest CONFIG REQUIRED)
find_package(iceoryx_hoofs_testing REQUIRED)
iox_add_library(
    STATIC
    TARGET                  iceoryx_posh_testing
    NAMESPACE               iceoryx_posh_testing
    PROJECT_PREFIX          ${PREFIX}
    BUILD_INTERFACE         ${CMAKE_CURRENT_SOURCE_DIR}/include
    INSTALL_INTERFACE       include/${PREFIX}
    EXPORT_INCLUDE_DIRS     include/
    PUBLIC_LIBS             GTest::gtest
                            GTest::gmock
                            iceoryx_posh::iceoryx_posh_roudi_env
    PRIVATE_LIBS            iceoryx_posh::iceoryx_posh
                            iceoryx_hoofs::iceoryx_hoofs
                            iceoryx_hoofs_testing::iceoryx_hoofs_testing
    FILES
        source/roudi_gtest.cpp
)

if(TEST_WITH_HUGE_PAYLOAD)
    target_compile_definitions(iceoryx_posh_testing PUBLIC -DTEST_WITH_HUGE_PAYLOAD)
endif(TEST_WITH_HUGE_PAYLOAD)