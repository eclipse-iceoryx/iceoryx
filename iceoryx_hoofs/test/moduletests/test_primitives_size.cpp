// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex AI Inc. All rights reserved.
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

#include "iox/size.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;

class Size_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(Size_test, ArrayCapacityReturnsCorrectValues)
{
    ::testing::Test::RecordProperty("TEST_ID", "8392b2ba-04ef-45e6-8b47-4c0c90d98f61");
    constexpr uint64_t CAPACITY{42};
    // NOLINTJUSTIFICATION Used only for test purposes
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
    constexpr uint32_t SUT[CAPACITY]{};

    EXPECT_EQ(iox::size(SUT), CAPACITY);
}

} // namespace
