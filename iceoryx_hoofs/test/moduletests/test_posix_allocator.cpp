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

#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object/allocator.hpp"
#include "iceoryx_hoofs/log/logging.hpp"
#include "test.hpp"

namespace
{
using namespace testing;

class Allocator_Test : public Test
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

    static constexpr uint64_t MEMORY_ALIGNMENT{iox::posix::Allocator::MEMORY_ALIGNMENT};

    void* memory{nullptr};
    size_t memorySize = 10016;
};

TEST_F(Allocator_Test, allocateOneSmallElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "f689e95c-5743-4370-93f0-8a23b909c75a");
    iox::posix::Allocator sut(memory, memorySize);
    int* bla = static_cast<int*>(sut.allocate(sizeof(int), MEMORY_ALIGNMENT));
    *bla = 123;
    EXPECT_THAT(*bla, Eq(123));
}

TEST_F(Allocator_Test, allocateEverythingWithSingleElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "f2e1085b-08fe-4b08-b022-0385b5a53fca");
    iox::posix::Allocator sut(memory, memorySize);
    int* bla = static_cast<int*>(sut.allocate(memorySize, 1));
    *bla = 123;
    EXPECT_THAT(*bla, Eq(123));
}

TEST_F(Allocator_Test, allocateEverythingWithMultipleElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "21d0fa61-54f9-41a0-8e53-e3448784497b");
    iox::posix::Allocator sut(memory, memorySize);
    for (size_t i = 0; i < memorySize; i += 32)
    {
        auto* bla = static_cast<size_t*>(sut.allocate(32, 1));
        *bla = i;
        EXPECT_THAT(*bla, Eq(i));
    }
}

TEST_F(Allocator_Test, allocateTooMuchSingleElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "9deed5c0-19d8-4469-a5c3-f185d4d881f1");
    iox::posix::Allocator sut(memory, memorySize);
    // @todo iox-#1613 remove EXPECT_DEATH
    // NOLINTBEGIN(hicpp-avoid-goto, cppcoreguidelines-avoid-goto, cert-err33-c, cppcoreguidelines-pro-type-vararg,
    // hiccpp-vararg)
    EXPECT_DEATH({ sut.allocate(memorySize + 1, MEMORY_ALIGNMENT); }, ".*");
    // NOLINTEND(hicpp-avoid-goto, cppcoreguidelines-avoid-goto, cert-err33-c, cppcoreguidelines-pro-type-vararg,
    // hiccpp-vararg)
}

TEST_F(Allocator_Test, allocateTooMuchMultipleElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "435151e8-cc34-41ce-8115-5c179716a60a");
    iox::posix::Allocator sut(memory, memorySize);
    for (size_t i = 0; i < memorySize; i += 32)
    {
        sut.allocate(32, 1);
    }

    // @todo iox-#1613 remove EXPECT_DEATH
    // NOLINTBEGIN(hicpp-avoid-goto, cppcoreguidelines-avoid-goto, cert-err33-c, cppcoreguidelines-pro-type-vararg,
    // hiccpp-vararg)
    EXPECT_DEATH({ sut.allocate(1, MEMORY_ALIGNMENT); }, ".*");
    // NOLINTEND(hicpp-avoid-goto, cppcoreguidelines-avoid-goto, cert-err33-c, cppcoreguidelines-pro-type-vararg,
    // hiccpp-vararg)
}

TEST_F(Allocator_Test, allocateAndAlignment)
{
    ::testing::Test::RecordProperty("TEST_ID", "4252ddcc-05d4-499f-ad7c-30bffb420e08");
    iox::posix::Allocator sut(memory, memorySize);
    auto* bla = static_cast<uint8_t*>(sut.allocate(5, MEMORY_ALIGNMENT));
    auto* bla2 = static_cast<uint8_t*>(sut.allocate(5, MEMORY_ALIGNMENT));
    EXPECT_THAT(bla2 - bla, Eq(8U));
}

TEST_F(Allocator_Test, allocateElementOfSizeZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "17caa50c-94bf-4a1d-a1ec-dfda563caa0b");
    iox::posix::Allocator sut(memory, memorySize);

    // @todo iox-#1613 remove EXPECT_DEATH
    // NOLINTBEGIN(hicpp-avoid-goto, cppcoreguidelines-avoid-goto, cert-err33-c, cppcoreguidelines-pro-type-vararg,
    // hiccpp-vararg)
    EXPECT_DEATH(sut.allocate(0, MEMORY_ALIGNMENT), ".*");
    // NOLINTEND(hicpp-avoid-goto, cppcoreguidelines-avoid-goto, cert-err33-c, cppcoreguidelines-pro-type-vararg,
    // hiccpp-vararg)
}

TEST_F(Allocator_Test, allocateAfterFinalizeAllocation)
{
    ::testing::Test::RecordProperty("TEST_ID", "323fc1af-481f-4732-b7d3-fa32da389cef");
    class AllocatorAccess : iox::posix::Allocator
    {
      public:
        AllocatorAccess(void* const f_startAddress, const uint64_t f_length)
            : iox::posix::Allocator(f_startAddress, f_length)
        {
        }
        using iox::posix::Allocator::allocate;
        using iox::posix::Allocator::finalizeAllocation;
    };
    AllocatorAccess sut(memory, memorySize);
    sut.allocate(5, MEMORY_ALIGNMENT);
    sut.finalizeAllocation();

    // @todo iox-#1613 remove EXPECT_DEATH
    // NOLINTBEGIN(hicpp-avoid-goto, cppcoreguidelines-avoid-goto, cert-err33-c, cppcoreguidelines-pro-type-vararg,
    // hiccpp-vararg)
    EXPECT_DEATH({ sut.allocate(5, MEMORY_ALIGNMENT); }, ".*");
    // NOLINTEND(hicpp-avoid-goto, cppcoreguidelines-avoid-goto, cert-err33-c, cppcoreguidelines-pro-type-vararg,
    // hiccpp-vararg)
}
} // namespace
