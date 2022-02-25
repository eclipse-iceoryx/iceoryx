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

#include "iceoryx_posh/mepoo/memory_info.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::mepoo;

TEST(MemoryInfo_test, ComparisonOperatorReturnsTrueWhenEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "55ffa041-9e19-4fc0-9536-f1a2a2afc44c");
    MemoryInfo info1;
    MemoryInfo info2;

    EXPECT_TRUE(info1 == info1);
    EXPECT_TRUE(info1 == info2);
    EXPECT_TRUE(info2 == info1);
}

TEST(MemoryInfo_test, ComparisonOperatorReturnsFalseWhenDeviceIdDoesNotMatch)
{
    ::testing::Test::RecordProperty("TEST_ID", "1f97ec6a-ecce-45ae-a88d-14455b463076");
    MemoryInfo info1;
    info1.deviceId = 42;
    MemoryInfo info2;
    info2.deviceId = 73;

    EXPECT_FALSE(info1 == info2);
    EXPECT_FALSE(info2 == info1);
}

TEST(MemoryInfo_test, ComparisonOperatorReturnsFalseWhenMemoryTypeDoesNotMatch)
{
    ::testing::Test::RecordProperty("TEST_ID", "e6a12a78-fd39-457e-bd3f-4fdc94275fab");
    MemoryInfo info1;
    info1.memoryType = 13;
    MemoryInfo info2;
    info2.memoryType = 37;

    EXPECT_FALSE(info1 == info2);
    EXPECT_FALSE(info2 == info1);
}
} // namespace
