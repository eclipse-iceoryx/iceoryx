// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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


#include "iceoryx_posh/testing/roudi_environment/minimal_roudi_config.hpp"
#include "iceoryx_posh/testing/roudi_environment/roudi_environment.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::roudi;
using namespace iox::testing;

class RouDiEnvironment_test : public Test
{
  public:
    void SetUp(){};
    void TearDown(){};
};

TEST_F(RouDiEnvironment_test, StartingRouDiTwiceLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "38075292-7897-4db5-b20e-f06ab324ad31");
    RouDiEnvironment m_sut{MinimalRouDiConfigBuilder().create()};

    GTEST_FLAG(death_test_style) = "threadsafe";
    EXPECT_DEATH({ RouDiEnvironment m_sut2{MinimalRouDiConfigBuilder().create()}; }, ".*");
}

} // namespace
