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

#include "test.hpp"

#include "iceoryx_hoofs/data_structures/typed_allocator.hpp"

#include <algorithm>
#include <vector>

namespace
{
using namespace ::testing;

using namespace iox::cxx;

// count #ctor and #dtor calls
template <typename T>
class Counter
{
  public:
    static uint64_t numCreated;
    static uint64_t numCopied;
    static uint64_t numMoved;
    static uint64_t numDestroyed;

    Counter()
    {
        ++numCreated;
    }

    ~Counter()
    {
        ++numDestroyed;
    }

    static void resetCounts()
    {
        numCreated = 0U;
        numDestroyed = 0U;
    }
};

template <typename T>
uint64_t Counter<T>::numCreated = 0U;

template <typename T>
uint64_t Counter<T>::numDestroyed = 0U;

// test with non-primitive comparable types
struct Integer : Counter<Integer>
{
    Integer(uint32_t value)
        : value(value)
    {
    }
    uint32_t value;
};

// bool operator==(const Integer& lhs, const Integer& rhs)
// {
//     return lhs.value == rhs.value;
// }

static constexpr uint32_t TEST_CAPACITY = 4;
using TestValue = Integer;


// todo: typed test
using TestAlloator = TypedAllocator<Integer, TEST_CAPACITY>;

class TypedAllocator_test : public ::testing::Test
{
  protected:
    TypedAllocator_test()
    {
    }

    ~TypedAllocator_test()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

    TestAlloator sut;
};

TEST_F(TypedAllocator_test, CanAllocateExactlyCapacityBlocks)
{
    for (uint32_t i = 0; i < TEST_CAPACITY; ++i)
    {
        auto p = sut.allocate();
        ASSERT_NE(p, nullptr);
    }
    auto p = sut.allocate();
    EXPECT_EQ(p, nullptr);
    // calling deallocate is not necessary, the allocator just winks out
    // this is a valid use case for (zone) allocators for efficiency
}

TEST_F(TypedAllocator_test, AllocateAndDeallocateWorks)
{
    std::vector<TestValue*> allocations;
    for (uint32_t i = 0; i < TEST_CAPACITY; ++i)
    {
        auto p = sut.allocate();
        ASSERT_NE(p, nullptr);
        allocations.push_back(p);
    }

    // there is no deallocate feedback but it should not crash or anything
    // another test checks whether the memory can be reused
    for (auto p : allocations)
    {
        sut.deallocate(p);
    }
}

// check aligned allocation, different pointers

TEST_F(TypedAllocator_test, DeallocationFreesCapacityForNewAllocation)
{
    std::vector<TestValue*> allocations;
    for (uint32_t i = 0; i < TEST_CAPACITY; ++i)
    {
        auto p = sut.allocate();
        ASSERT_NE(p, nullptr);
        allocations.push_back(p);
    }

    constexpr uint32_t NUM_TO_DEALLOCATE = TEST_CAPACITY;

    for (uint32_t i = 0; i < NUM_TO_DEALLOCATE; ++i)
    {
        auto p = allocations[i];
        sut.deallocate(p);
    }

    // blocks should be free now for reuse

    constexpr uint32_t NUM_TO_ALLOCATE = NUM_TO_DEALLOCATE;


    for (uint32_t i = 0; i < NUM_TO_ALLOCATE; ++i)
    {
        auto p = sut.allocate();
        ASSERT_NE(p, nullptr);
    }

    auto p = sut.allocate();
    ASSERT_EQ(p, nullptr);
}

TEST_F(TypedAllocator_test, CanCreateExactlyCapacityElements)
{
    TestValue::resetCounts();
    constexpr uint32_t OFFSET = 73U;
    constexpr uint32_t NUM_TO_CREATE = TEST_CAPACITY;

    std::vector<TestValue*> allocations;
    for (uint32_t i = 0; i < NUM_TO_CREATE; ++i)
    {
        auto element = sut.create(i + OFFSET);
        ASSERT_NE(element, nullptr);
        // note that if the test fails here the dtor of
        // the element is not called but this is no problem
        // as it does not rely on RAII

        EXPECT_EQ(element->value, i + OFFSET);
        allocations.push_back(element);
    }

    // verify #ctor calls
    EXPECT_EQ(TestValue::numCreated, NUM_TO_CREATE);

    auto element = sut.create(OFFSET + NUM_TO_CREATE);
    EXPECT_EQ(element, nullptr);

    for (auto element : allocations)
    {
        sut.destroy(element);
    }

    // verify #dtor calls
    EXPECT_EQ(TestValue::numDestroyed, NUM_TO_CREATE);
}

TEST_F(TypedAllocator_test, DestroyFreesCapacityForCreationOfNewElements)
{
    TestValue::resetCounts();
    constexpr uint32_t OFFSET = 37U;
    constexpr uint32_t NUM_TO_CREATE = TEST_CAPACITY;

    std::vector<TestValue*> elements;
    for (uint32_t i = 0; i < NUM_TO_CREATE; ++i)
    {
        auto element = sut.create(i + OFFSET);
        ASSERT_NE(element, nullptr);
        elements.push_back(element);
    }

    for (auto element : elements)
    {
        sut.destroy(element);
    }

    EXPECT_EQ(TestValue::numDestroyed, NUM_TO_CREATE);

    // blocks should be free now for reuse

    elements.clear();
    auto offset = OFFSET + NUM_TO_CREATE;
    for (uint32_t i = 0; i < NUM_TO_CREATE; ++i)
    {
        auto value = i + offset;
        auto element = sut.create(value);
        ASSERT_NE(element, nullptr);
        // did we really create a new element with the expected value?
        EXPECT_EQ(element->value, value);
        elements.push_back(element);
    }

    for (auto element : elements)
    {
        sut.destroy(element);
    }

    EXPECT_EQ(TestValue::numCreated, 2 * NUM_TO_CREATE);
    EXPECT_EQ(TestValue::numDestroyed, 2 * NUM_TO_CREATE);
}

TEST_F(TypedAllocator_test, AllocationsAreAlignedAsElementType)
{
    constexpr auto ALIGN = alignof(TestValue);
    for (uint32_t i = 0; i < TEST_CAPACITY; ++i)
    {
        // we check the alignment of all allocations (i.e. not just one)
        auto p = sut.allocate();
        ASSERT_NE(p, nullptr);
        auto adr = reinterpret_cast<uint64_t>(p);
        EXPECT_EQ(adr % ALIGN, 0U);
    }
}

TEST_F(TypedAllocator_test, AllocationsHaveUniqueAdresses)
{
    std::vector<TestValue*> allocations;
    for (uint32_t i = 0; i < TEST_CAPACITY; ++i)
    {
        auto p = sut.allocate();
        ASSERT_NE(p, nullptr);
        allocations.push_back(p);
    }

    // filter unique allocations
    std::sort(allocations.begin(), allocations.end());
    auto lastUnique = std::unique(allocations.begin(), allocations.end());
    allocations.erase(lastUnique, allocations.end());

    EXPECT_EQ(allocations.size(), TEST_CAPACITY);
}

TEST_F(TypedAllocator_test, CreatedElementsHaveUniqueAdresses)
{
    std::vector<TestValue*> elements;
    for (uint32_t i = 0; i < TEST_CAPACITY; ++i)
    {
        auto element = sut.create(i);
        ASSERT_NE(element, nullptr);
        elements.push_back(element);
    }

    // filter unique element adresses
    std::sort(elements.begin(), elements.end());
    auto lastUnique = std::unique(elements.begin(), elements.end());
    elements.erase(lastUnique, elements.end());

    EXPECT_EQ(elements.size(), TEST_CAPACITY);

    for (auto element : elements)
    {
        sut.destroy(element);
    }
}


} // namespace
