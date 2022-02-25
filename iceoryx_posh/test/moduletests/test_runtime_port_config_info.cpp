// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "iceoryx_posh/runtime/port_config_info.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::runtime;

TEST(PortConfigInfo_test, ComparisonOperatorReturnsTrueWhenEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "b72010f4-d636-42f1-aff5-f85523e0a9df");
    PortConfigInfo info1;
    PortConfigInfo info2;

    EXPECT_TRUE(info1 == info1);
    EXPECT_TRUE(info1 == info2);
    EXPECT_TRUE(info2 == info1);
}

TEST(PortConfigInfo_test, ComparisonOperatorReturnsFalseWhenDeviceIdDoesNotMatch)
{
    ::testing::Test::RecordProperty("TEST_ID", "2c166fba-6eba-4cca-ba8d-d135e30e827c");
    PortConfigInfo info1;
    info1.portType = 42;
    PortConfigInfo info2;
    info2.portType = 73;

    EXPECT_FALSE(info1 == info2);
    EXPECT_FALSE(info2 == info1);
}

TEST(PortConfigInfo_test, ComparisonOperatorReturnsFalseWhenMemoryTypeDoesNotMatch)
{
    ::testing::Test::RecordProperty("TEST_ID", "b7b0e181-f241-4aeb-90f7-af5ee6747bf6");
    PortConfigInfo info1;
    info1.memoryInfo.deviceId = 13;
    PortConfigInfo info2;
    info2.memoryInfo.deviceId = 37;

    EXPECT_FALSE(info1 == info2);
    EXPECT_FALSE(info2 == info1);
}
} // namespace
