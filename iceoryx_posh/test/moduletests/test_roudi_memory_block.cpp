// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/roudi/memory/memory_block.hpp"

#include "mocks/roudi_memory_provider_mock.hpp"

#include "test.hpp"

using namespace ::testing;

using namespace iox::roudi;

class MemoryBlockTestImpl final : public MemoryBlock
{
  public:
    uint64_t size() const noexcept override
    {
        return MEMORY_SIZE;
    }

    uint64_t alignment() const noexcept override
    {
        return MEMORY_ALIGNMENT;
    }

    void destroy() noexcept override
    {
    }

    static constexpr uint64_t MEMORY_SIZE = 1;
    static constexpr uint64_t MEMORY_ALIGNMENT = 1;
};

class MemoryBlock_Test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    MemoryBlockTestImpl sut;
    MemoryProviderTestImpl memoryProvider;
};

TEST_F(MemoryBlock_Test, Initial)
{
    EXPECT_THAT(sut.memory().has_value(), Eq(false));
}

TEST_F(MemoryBlock_Test, MemoryAvailableAfterCreation)
{
    memoryProvider.addMemoryBlock(&sut);
    memoryProvider.create();
    EXPECT_THAT(memoryProvider.dummyMemory, Ne(nullptr));
    ASSERT_THAT(sut.memory().has_value(), Eq(true));
    EXPECT_THAT(sut.memory().value(), Eq(memoryProvider.dummyMemory));
}
