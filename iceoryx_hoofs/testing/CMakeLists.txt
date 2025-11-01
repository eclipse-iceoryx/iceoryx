# Copyright (c) 2019 - 2021 by Robert Bosch GmbH. All rights reserved.
# Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
#
# This program and the accompanying materials are made available under the
# terms of the Apache Software License 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0, or the MIT license
# which is available at https://opensource.org/licenses/MIT.
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#

iox_add_library(
    STATIC
    TARGET                  iceoryx_hoofs_testing
    NAMESPACE               iceoryx_hoofs_testing
    PROJECT_PREFIX          ${PREFIX}
    PUBLIC_LIBS             GTest::gtest
                            GTest::gmock
    PRIVATE_LIBS            ${CODE_COVERAGE_LIBS}
                            iceoryx_hoofs
                            ${CMAKE_DL_LIBS}
    BUILD_INTERFACE         ${CMAKE_CURRENT_SOURCE_DIR}/include
    INSTALL_INTERFACE       include/${PREFIX}
    EXPORT_INCLUDE_DIRS     include/
    FILES
        testing_logger.cpp
        timing_test.cpp
        error_reporting/testing_error_handler.cpp
        error_reporting/testing_support.cpp
)

# deactivate _FORTIFY_SOURCE since it causes issues with longjmp in the testing logger and error handler
target_compile_options(iceoryx_hoofs_testing PRIVATE -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0)

if(TEST_WITH_ADDITIONAL_USER)
    target_compile_definitions(iceoryx_hoofs_testing PUBLIC -DTEST_WITH_ADDITIONAL_USER)
endif(TEST_WITH_ADDITIONAL_USER)
