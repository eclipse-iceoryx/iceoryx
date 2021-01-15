// Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "test.hpp"

#include "iceoryx_posh/roudi/memory/iceoryx_roudi_memory_manager.hpp"

using namespace ::testing;

using iox::roudi::IceOryxRouDiMemoryManager;


class IceoryxRoudiMemoryMananger_test : public Test
{
  public:
    IceOryxRouDiMemoryManager* m_roudiMemoryManagerTest{nullptr};

    void SetUp() override
    {
    	auto config = iox::RouDiConfig_t().setDefaults();
    	m_roudiMemoryManagerTest = new IceOryxRouDiMemoryManager(config);
    }

    void TearDown() override
    {
 	delete m_roudiMemoryManagerTest;
    }
};

TEST_F(IceoryxRoudiMemoryMananger_test, IntrospectionMemoryManagerNotInitialzed)
{
    auto result = m_roudiMemoryManagerTest->introspectionMemoryManager();
    EXPECT_THAT(result, Eq(iox::cxx::nullopt_t()));

    auto resultTest = m_roudiMemoryManagerTest->segmentManager();
    EXPECT_THAT(resultTest, Eq(iox::cxx::nullopt_t()));

    auto testResult = m_roudiMemoryManagerTest->portPool();
    EXPECT_THAT(testResult, Eq(iox::cxx::nullopt_t()));
}


