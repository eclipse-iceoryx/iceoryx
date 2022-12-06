// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

namespace
{
using namespace testing;

class BumpAllocator_Test : public Test
{
  public:
    void SetUp() override
    {
        // NOLINTNEXTLINE(hicpp-no-malloc, cppcoreguidelines-no-malloc) required to test allocation
        memory = malloc(memorySize);
    }

    void TearDown() override
    {
        // NOLINTNEXTLINE(hicpp-no-malloc, cppcoreguidelines-no-malloc) required to test allocation
        free(memory);
    }

    static constexpr uint64_t MEMORY_ALIGNMENT{8};

    void* memory{nullptr};
    size_t memorySize = 10016;
};

TEST_F(BumpAllocator_Test, allocateOneSmallElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "f689e95c-5743-4370-93f0-8a23b909c75a");
    iox::BumpAllocator sut(memory, memorySize);
    auto allocationResult = sut.allocate(sizeof(int), MEMORY_ALIGNMENT);
    ASSERT_FALSE(allocationResult.has_error());
    int* bla = static_cast<int*>(allocationResult.value());
    *bla = 123;
    EXPECT_THAT(*bla, Eq(123));
}

TEST_F(BumpAllocator_Test, allocateEverythingWithSingleElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "f2e1085b-08fe-4b08-b022-0385b5a53fca");
    iox::BumpAllocator sut(memory, memorySize);
    auto allocationResult = sut.allocate(memorySize, 1);
    ASSERT_FALSE(allocationResult.has_error());
    int* bla = static_cast<int*>(allocationResult.value());
    *bla = 123;
    EXPECT_THAT(*bla, Eq(123));
}

TEST_F(BumpAllocator_Test, allocateEverythingWithMultipleElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "21d0fa61-54f9-41a0-8e53-e3448784497b");
    iox::BumpAllocator sut(memory, memorySize);
    for (size_t i = 0; i < memorySize; i += 32)
    {
        auto allocationResult = sut.allocate(32, 1);
        ASSERT_FALSE(allocationResult.has_error());
        auto* bla = static_cast<size_t*>(allocationResult.value());
        *bla = i;
        EXPECT_THAT(*bla, Eq(i));
    }
}

TEST_F(BumpAllocator_Test, allocateTooMuchSingleElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "9deed5c0-19d8-4469-a5c3-f185d4d881f1");
    iox::BumpAllocator sut(memory, memorySize);
    auto allocationResult = sut.allocate(memorySize + 1, MEMORY_ALIGNMENT);
    ASSERT_TRUE(allocationResult.has_error());
    EXPECT_THAT(allocationResult.get_error(), Eq(iox::BumpAllocatorError::OUT_OF_MEMORY));
}

TEST_F(BumpAllocator_Test, allocateTooMuchMultipleElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "435151e8-cc34-41ce-8115-5c179716a60a");
    iox::BumpAllocator sut(memory, memorySize);
    for (size_t i = 0; i < memorySize; i += 32)
    {
        ASSERT_FALSE(sut.allocate(32, 1).has_error());
    }

    auto allocationResult = sut.allocate(1, MEMORY_ALIGNMENT);
    ASSERT_TRUE(allocationResult.has_error());
    EXPECT_THAT(allocationResult.get_error(), Eq(iox::BumpAllocatorError::OUT_OF_MEMORY));
}

TEST_F(BumpAllocator_Test, allocateAndAlignment)
{
    ::testing::Test::RecordProperty("TEST_ID", "4252ddcc-05d4-499f-ad7c-30bffb420e08");
    iox::BumpAllocator sut(memory, memorySize);
    auto allocationResult = sut.allocate(5, MEMORY_ALIGNMENT);
    ASSERT_FALSE(allocationResult.has_error());
    auto* bla = static_cast<uint8_t*>(allocationResult.value());

    allocationResult = sut.allocate(5, MEMORY_ALIGNMENT);
    ASSERT_FALSE(allocationResult.has_error());
    auto* bla2 = static_cast<uint8_t*>(allocationResult.value());
    EXPECT_THAT(bla2 - bla, Eq(8U));
}

TEST_F(BumpAllocator_Test, allocateElementOfSizeZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "17caa50c-94bf-4a1d-a1ec-dfda563caa0b");
    iox::BumpAllocator sut(memory, memorySize);

    // @todo iox-#1613 remove EXPECT_DEATH
    // NOLINTBEGIN(hicpp-avoid-goto, cppcoreguidelines-avoid-goto, cert-err33-c, cppcoreguidelines-pro-type-vararg,
    // hiccpp-vararg)
    EXPECT_DEATH(IOX_DISCARD_RESULT(sut.allocate(0, MEMORY_ALIGNMENT)), ".*");
    // NOLINTEND(hicpp-avoid-goto, cppcoreguidelines-avoid-goto, cert-err33-c, cppcoreguidelines-pro-type-vararg,
    // hiccpp-vararg)
}

TEST_F(BumpAllocator_Test, allocateAfterDeallocateWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "323fc1af-481f-4732-b7d3-fa32da389cef");
    iox::BumpAllocator sut(memory, memorySize);
    ASSERT_FALSE(sut.allocate(memorySize, 1).has_error());

    sut.deallocate();

    auto allocationResult = sut.allocate(memorySize, 1);
    ASSERT_FALSE(allocationResult.has_error());
    int* bla = static_cast<int*>(allocationResult.value());
    *bla = 1990;
    EXPECT_THAT(*bla, Eq(1990));
}
} // namespace
