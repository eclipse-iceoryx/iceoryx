// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2023 by Apex.AI Inc. All rights reserved.
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

#include "iox/bump_allocator.hpp"
#include "test.hpp"

#include <limits>

namespace
{
using namespace testing;

class BumpAllocator_Test : public Test
{
  public:
    void SetUp() override
    {
        // NOLINTNEXTLINE(hicpp-no-malloc, cppcoreguidelines-no-malloc) required to test allocation
        memory = malloc(MEMORY_SIZE);
        ASSERT_THAT(memory, Ne(nullptr));
    }

    void TearDown() override
    {
        // NOLINTNEXTLINE(hicpp-no-malloc, cppcoreguidelines-no-malloc) required to test allocation
        free(memory);
    }

    static constexpr uint64_t MEMORY_ALIGNMENT{8};
    static constexpr uint64_t MEMORY_SIZE{10016};

    void* memory{nullptr};
};

TEST_F(BumpAllocator_Test, AllocateFailsWithZeroSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "17caa50c-94bf-4a1d-a1ec-dfda563caa0b");
    iox::BumpAllocator sut(memory, MEMORY_SIZE);

    auto allocationResult = sut.allocate(0, MEMORY_ALIGNMENT);
    ASSERT_TRUE(allocationResult.has_error());
    EXPECT_THAT(allocationResult.error(), Eq(iox::BumpAllocatorError::REQUESTED_ZERO_SIZED_MEMORY));
}

TEST_F(BumpAllocator_Test, OverallocationFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "9deed5c0-19d8-4469-a5c3-f185d4d881f1");
    iox::BumpAllocator sut(memory, MEMORY_SIZE);

    auto allocationResult = sut.allocate(MEMORY_SIZE + 1, MEMORY_ALIGNMENT);
    ASSERT_TRUE(allocationResult.has_error());
    EXPECT_THAT(allocationResult.error(), Eq(iox::BumpAllocatorError::OUT_OF_MEMORY));
}

TEST_F(BumpAllocator_Test, OverallocationAfterMultipleCallsFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "435151e8-cc34-41ce-8115-5c179716a60a");
    constexpr uint64_t MEMORY_CHUNK_SIZE{32};
    iox::BumpAllocator sut(memory, MEMORY_SIZE);
    for (uint64_t i{0}; i < MEMORY_SIZE; i += MEMORY_CHUNK_SIZE)
    {
        ASSERT_FALSE(sut.allocate(MEMORY_CHUNK_SIZE, MEMORY_ALIGNMENT).has_error());
    }

    auto allocationResult = sut.allocate(1, MEMORY_ALIGNMENT);
    ASSERT_TRUE(allocationResult.has_error());
    EXPECT_THAT(allocationResult.error(), Eq(iox::BumpAllocatorError::OUT_OF_MEMORY));
}

TEST_F(BumpAllocator_Test, AllocationIsCorrectlyAligned)
{
    ::testing::Test::RecordProperty("TEST_ID", "4252ddcc-05d4-499f-ad7c-30bffb420e08");
    constexpr uint64_t MEMORY_CHUNK_SIZE{sizeof(int)};
    constexpr uint64_t MEMORY_CHUNK_ALIGNMENT{alignof(int)};
    iox::BumpAllocator sut(memory, MEMORY_SIZE);

    auto allocationResult = sut.allocate(MEMORY_CHUNK_SIZE, MEMORY_CHUNK_ALIGNMENT);
    ASSERT_FALSE(allocationResult.has_error());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) required for testing
    auto p = reinterpret_cast<uintptr_t>(allocationResult.value());
    EXPECT_THAT(p % MEMORY_CHUNK_ALIGNMENT, Eq(0));

    allocationResult = sut.allocate(2 * MEMORY_CHUNK_SIZE, 2 * MEMORY_CHUNK_ALIGNMENT);
    ASSERT_FALSE(allocationResult.has_error());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) required for testing
    p = reinterpret_cast<uintptr_t>(allocationResult.value());
    EXPECT_THAT(p % (2 * MEMORY_CHUNK_ALIGNMENT), Eq(0));
}

TEST_F(BumpAllocator_Test, AllocateSmallMemoryChunkAndStoreDataWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "f689e95c-5743-4370-93f0-8a23b909c75a");
    iox::BumpAllocator sut(memory, MEMORY_SIZE);
    auto allocationResult = sut.allocate(sizeof(int), alignof(int));
    ASSERT_FALSE(allocationResult.has_error());

    int* bla = static_cast<int*>(allocationResult.value());
    *bla = std::numeric_limits<int>::min();
    EXPECT_THAT(*bla, Eq(std::numeric_limits<int>::min()));
}

TEST_F(BumpAllocator_Test, AllocateCompleteMemoryAndStoreDataWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "f2e1085b-08fe-4b08-b022-0385b5a53fca");
    iox::BumpAllocator sut(memory, sizeof(int));
    auto allocationResult = sut.allocate(sizeof(int), alignof(int));
    ASSERT_FALSE(allocationResult.has_error());

    int* bla = static_cast<int*>(allocationResult.value());
    *bla = std::numeric_limits<int>::max();
    EXPECT_THAT(*bla, Eq(std::numeric_limits<int>::max()));
}

TEST_F(BumpAllocator_Test, AllocateCompleteMemoryWithEquallySizedChunksWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "21d0fa61-54f9-41a0-8e53-e3448784497b");
    constexpr uint64_t MEMORY_CHUNK_SIZE{32};
    iox::BumpAllocator sut(memory, MEMORY_SIZE);

    auto allocationResult = sut.allocate(MEMORY_CHUNK_SIZE, MEMORY_ALIGNMENT);
    ASSERT_FALSE(allocationResult.has_error());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) required for testing
    auto p0 = reinterpret_cast<uintptr_t>(allocationResult.value());

    for (uint64_t i{MEMORY_CHUNK_SIZE}; i < MEMORY_SIZE; i += MEMORY_CHUNK_SIZE)
    {
        allocationResult = sut.allocate(MEMORY_CHUNK_SIZE, MEMORY_ALIGNMENT);
        ASSERT_FALSE(allocationResult.has_error());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) required for testing
        auto p1 = reinterpret_cast<uintptr_t>(allocationResult.value());
        EXPECT_THAT(p1 - p0, Eq(MEMORY_CHUNK_SIZE));
        p0 = p1;
    }
}

TEST_F(BumpAllocator_Test, AllocateCompleteMemoryWithDifferentSizedChunksWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "c8079bb7-1de6-45a6-92af-a50aa59d7481");
    constexpr uint64_t MEMORY_CHUNK_SIZE{64};
    iox::BumpAllocator sut(memory, MEMORY_SIZE);

    auto allocationResult = sut.allocate(MEMORY_CHUNK_SIZE, MEMORY_ALIGNMENT);
    ASSERT_FALSE(allocationResult.has_error());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) required for testing
    auto p0 = reinterpret_cast<uintptr_t>(allocationResult.value());

    for (uint64_t i{MEMORY_CHUNK_SIZE}; i < MEMORY_SIZE - MEMORY_CHUNK_SIZE; i += MEMORY_CHUNK_SIZE)
    {
        allocationResult = sut.allocate(MEMORY_CHUNK_SIZE, MEMORY_ALIGNMENT);
        ASSERT_FALSE(allocationResult.has_error());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) required for testing
        auto p1 = reinterpret_cast<uintptr_t>(allocationResult.value());
        EXPECT_THAT(p1 - p0, Eq(MEMORY_CHUNK_SIZE));
        p0 = p1;
    }

    allocationResult = sut.allocate(MEMORY_CHUNK_SIZE / 2, MEMORY_ALIGNMENT);
    ASSERT_FALSE(allocationResult.has_error());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) required for testing
    auto p1 = reinterpret_cast<uintptr_t>(allocationResult.value());
    EXPECT_THAT(p1 - p0, Eq(MEMORY_CHUNK_SIZE));
}

TEST_F(BumpAllocator_Test, AllocateAfterDeallocateWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "323fc1af-481f-4732-b7d3-fa32da389cef");
    iox::BumpAllocator sut(memory, MEMORY_SIZE);
    auto allocationResult = sut.allocate(sizeof(int), alignof(int));
    ASSERT_FALSE(allocationResult.has_error());
    int* bla = static_cast<int*>(allocationResult.value());
    *bla = std::numeric_limits<int>::max();

    sut.deallocate();

    allocationResult = sut.allocate(sizeof(int), alignof(int));
    ASSERT_FALSE(allocationResult.has_error());
    bla = static_cast<int*>(allocationResult.value());
    EXPECT_THAT(bla, Eq(memory));

    *bla = 1990;
    EXPECT_THAT(*bla, Eq(1990));
}
} // namespace
