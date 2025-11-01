# Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
# Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

cmake_minimum_required(VERSION 3.16)
project(benchmark_optional_and_expected)

include(GNUInstallDirs)

find_package(iceoryx_platform REQUIRED)
find_package(iceoryx_hoofs CONFIG REQUIRED)
find_package(Threads REQUIRED)

include(IceoryxPlatform)
include(IceoryxPlatformSettings)

iox_add_executable(
    TARGET      iox-bm-optional-and-expected
    FILES       ./benchmark_optional_and_expected.cpp
    LIBS        iceoryx_hoofs::iceoryx_hoofs iceoryx_platform::iceoryx_platform Threads::Threads
)
