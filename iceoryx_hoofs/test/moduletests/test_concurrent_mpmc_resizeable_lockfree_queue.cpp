// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iox/detail/mpmc_resizeable_lockfree_queue.hpp"

/// Test the added functionality of ResizeableLockFreeQueue
/// to change the capacity (setCapacity).
/// The remaining functionality is identical to LockFreeQueue
/// and tested in test_lockfree_queue.cpp (as a typed test).
namespace
{
using namespace ::testing;

// use a non-POD type for testing (just a boxed version of int)
struct Integer
{
    // NOLINTNEXTLINE(hicpp-explicit-conversions) required for typed tests
    Integer(uint64_t value = 0)
        : value(value)
    {
    }

    uint64_t value{0};

    // so that it behaves like an int for comparison purposes
    // NOLINTNEXTLINE(hicpp-explicit-conversions) required for typed tests
    operator uint64_t() const
    {
        return value;
    }
};

template <typename T>
class MpmcResizeableLockFreeQueueTest : public ::testing::Test
{
  public:
    void fillQueue(uint64_t start = 0)
    {
        uint64_t element{start};
        for (uint64_t i = 0; i < queue.capacity(); ++i)
        {
            queue.tryPush(element);
            element++;
        }
    }

    using Queue = T;
    Queue queue;
};

template <size_t Capacity>
using IntegerQueue = iox::concurrent::MpmcResizeableLockFreeQueue<Integer, Capacity>;

template <size_t Capacity>
using IntQueue = iox::concurrent::MpmcResizeableLockFreeQueue<uint64_t, Capacity>;

typedef ::testing::Types<IntegerQueue<1>, IntegerQueue<11>, IntQueue<10>> TestQueues;

TYPED_TEST_SUITE(MpmcResizeableLockFreeQueueTest, TestQueues, );

TEST(MpmcResizeableLockFreeQueueTest, maxCapacityIsConsistent)
{
    ::testing::Test::RecordProperty("TEST_ID", "9ca4e449-aa07-4180-aab5-29aeffdaa544");
    using Queue = IntegerQueue<37U>;
    EXPECT_EQ(Queue::maxCapacity(), 37U);
}

TYPED_TEST(MpmcResizeableLockFreeQueueTest, initialCapacityIsMaximalbyDefault)
{
    ::testing::Test::RecordProperty("TEST_ID", "475e3359-2b84-482b-ab60-f00baa4544af");
    using Queue = typename TestFixture::Queue;
    auto& q = this->queue;
    EXPECT_EQ(q.capacity(), q.maxCapacity());
    EXPECT_EQ(q.capacity(), Queue::maxCapacity());
}

TYPED_TEST(MpmcResizeableLockFreeQueueTest, constructWithMaxCapacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "616cc6b8-9b57-44c5-be12-e675b0ccb60e");
    constexpr auto MAX_CAP = TestFixture::Queue::MAX_CAPACITY;
    typename TestFixture::Queue q(MAX_CAP);
    EXPECT_EQ(q.capacity(), q.maxCapacity());
}

TYPED_TEST(MpmcResizeableLockFreeQueueTest, constructWithMoreThanMaxCapacitySaturatesAtMaxCapacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "1e02b0bc-d7e0-4a66-9381-2f3d8b3b868d");
    constexpr auto MAX_CAP = TestFixture::Queue::MAX_CAPACITY;
    typename TestFixture::Queue q(MAX_CAP + 1U);
    EXPECT_EQ(q.capacity(), q.maxCapacity());
}

TYPED_TEST(MpmcResizeableLockFreeQueueTest, constructWithNoCapacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "9b3f8fee-1045-4f4a-b241-7cd3c8e134af");
    typename TestFixture::Queue q(0U);
    EXPECT_EQ(q.capacity(), 0U);
}

TYPED_TEST(MpmcResizeableLockFreeQueueTest, constructWithHalfOfMaxCapacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "fdcd764d-75d3-4cb7-9ed1-330de98bbd61");
    constexpr auto cap = TestFixture::Queue::MAX_CAPACITY / 2U;
    typename TestFixture::Queue q(cap);
    EXPECT_EQ(q.capacity(), cap);
}

TYPED_TEST(MpmcResizeableLockFreeQueueTest, decreaseCapacityToZeroOneByOne)
{
    ::testing::Test::RecordProperty("TEST_ID", "5f0c0319-bf83-409d-b3b9-8e021afaf091");
    auto& q = this->queue;
    constexpr auto MAX_CAP = TestFixture::Queue::MAX_CAPACITY;

    uint64_t element = 0U;
    while (q.tryPush(element))
    {
        ++element;
    }

    auto i = MAX_CAP;
    while (i > 0U)
    {
        EXPECT_TRUE(q.setCapacity(--i));
        ASSERT_EQ(q.capacity(), i);
        ASSERT_EQ(q.size(), i);
    }
}

TYPED_TEST(MpmcResizeableLockFreeQueueTest, decreaseCapacityToZeroOneByOneWithHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "d99088a8-725c-451f-8f4a-ae0aafba368d");
    constexpr auto MAX_CAP = TestFixture::Queue::MAX_CAPACITY;
    using element_t = typename TestFixture::Queue::element_t;
    iox::vector<element_t, MAX_CAP> removedElements;
    auto removeHandler = [&](const element_t& value) { removedElements.emplace_back(std::move(value)); };

    auto& q = this->queue;

    uint64_t element = 0U;
    while (q.tryPush(element))
    {
        ++element;
    }

    auto i = MAX_CAP;
    while (i > 0U)
    {
        EXPECT_TRUE(q.setCapacity(--i, removeHandler));
        ASSERT_EQ(q.capacity(), i);
        ASSERT_EQ(q.size(), i);
        EXPECT_EQ(removedElements.size(), MAX_CAP - i);
    }
}

TYPED_TEST(MpmcResizeableLockFreeQueueTest, increaseToMaxCapacityOneByOne)
{
    ::testing::Test::RecordProperty("TEST_ID", "ff282db3-b6dd-4f01-a9b1-e4f788172c49");
    typename TestFixture::Queue q(0U);
    constexpr auto MAX_CAP = TestFixture::Queue::MAX_CAPACITY;
    EXPECT_EQ(q.capacity(), 0U);

    for (uint64_t i = 0U; i < MAX_CAP;)
    {
        EXPECT_TRUE(q.setCapacity(++i));
        ASSERT_EQ(q.capacity(), i);
    }
}

TYPED_TEST(MpmcResizeableLockFreeQueueTest, increaseToMaxCapacityOneByOneWithHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "cf4dcb07-b9be-4d81-8e9d-d170274bcc1d");
    constexpr auto MAX_CAP = TestFixture::Queue::MAX_CAPACITY;
    using element_t = typename TestFixture::Queue::element_t;
    iox::vector<element_t, MAX_CAP> removedElements;
    auto removeHandler = [&](const element_t& value) { removedElements.emplace_back(std::move(value)); };

    typename TestFixture::Queue q(0U);
    EXPECT_EQ(q.capacity(), 0U);

    for (uint64_t i = 0U; i < MAX_CAP;)
    {
        EXPECT_TRUE(q.setCapacity(++i, removeHandler));
        ASSERT_EQ(q.capacity(), i);
        EXPECT_EQ(removedElements.size(), 0U);
    }
}

TYPED_TEST(MpmcResizeableLockFreeQueueTest, setCapacityToZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "74906aed-bd84-4f63-9d9b-c829e97971f6");
    auto& q = this->queue;
    EXPECT_TRUE(q.setCapacity(0U));
    EXPECT_EQ(q.capacity(), 0U);
}

TYPED_TEST(MpmcResizeableLockFreeQueueTest, setCapacityToZeroWithHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "7af169f3-32a3-43b7-8e48-cc22728e13f0");
    constexpr auto MAX_CAP = TestFixture::Queue::MAX_CAPACITY;
    using element_t = typename TestFixture::Queue::element_t;
    iox::vector<element_t, MAX_CAP> removedElements;
    auto removeHandler = [&](const element_t& value) { removedElements.emplace_back(std::move(value)); };

    auto& q = this->queue;
    EXPECT_TRUE(q.setCapacity(0U, removeHandler));
    EXPECT_EQ(q.capacity(), 0U);
    EXPECT_EQ(removedElements.size(), 0U);
}

TYPED_TEST(MpmcResizeableLockFreeQueueTest, setCapacityToOne)
{
    ::testing::Test::RecordProperty("TEST_ID", "3bf17fb8-f688-43c4-a9cc-91fcafd43012");
    auto& q = this->queue;
    EXPECT_TRUE(q.setCapacity(1U));
    EXPECT_EQ(q.capacity(), 1U);
}

TYPED_TEST(MpmcResizeableLockFreeQueueTest, setCapacityToOneWithHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "1f640a03-37f4-4cb8-9985-1c27a9a5480f");
    constexpr auto MAX_CAP = TestFixture::Queue::MAX_CAPACITY;
    using element_t = typename TestFixture::Queue::element_t;
    iox::vector<element_t, MAX_CAP> removedElements;
    auto removeHandler = [&](const element_t& value) { removedElements.emplace_back(std::move(value)); };

    auto& q = this->queue;
    EXPECT_TRUE(q.setCapacity(1U, removeHandler));
    EXPECT_EQ(q.capacity(), 1U);
    EXPECT_EQ(removedElements.size(), 0U);
}

TYPED_TEST(MpmcResizeableLockFreeQueueTest, setCapacityToMaxCapacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "76bcd176-e3e2-4acc-a846-596e75be5869");
    typename TestFixture::Queue q(0U);
    constexpr auto MAX_CAP = TestFixture::Queue::MAX_CAPACITY;
    EXPECT_TRUE(q.setCapacity(MAX_CAP));
    EXPECT_EQ(q.capacity(), MAX_CAP);
}

TYPED_TEST(MpmcResizeableLockFreeQueueTest, setCapacityToMaxCapacityWithHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "2e485d0f-6dc1-46af-86d0-cec0840d0821");
    typename TestFixture::Queue q(0U);
    constexpr auto MAX_CAP = TestFixture::Queue::MAX_CAPACITY;

    using element_t = typename TestFixture::Queue::element_t;
    iox::vector<element_t, MAX_CAP> removedElements;
    auto removeHandler = [&](const element_t& value) { removedElements.emplace_back(std::move(value)); };

    EXPECT_TRUE(q.setCapacity(MAX_CAP, removeHandler));
    EXPECT_EQ(q.capacity(), MAX_CAP);
    EXPECT_EQ(removedElements.size(), 0U);
}

TYPED_TEST(MpmcResizeableLockFreeQueueTest, setCapacityToHalfOfMaxCapacityAndFillIt)
{
    ::testing::Test::RecordProperty("TEST_ID", "054875b1-a0ab-4c65-a3ce-d1bb4113e94e");
    auto& q = this->queue;
    constexpr auto MAX_CAP = TestFixture::Queue::MAX_CAPACITY;
    constexpr auto NEW_CAP = MAX_CAP / 2U;
    EXPECT_EQ(q.setCapacity(NEW_CAP), true);
    EXPECT_EQ(q.capacity(), NEW_CAP);

    uint64_t element = 0U;
    while (q.tryPush(element))
    {
        ++element;
    }

    EXPECT_EQ(q.capacity(), NEW_CAP);
    EXPECT_EQ(q.size(), NEW_CAP);
    EXPECT_EQ(element, NEW_CAP);
}

TYPED_TEST(MpmcResizeableLockFreeQueueTest, setCapacityToHalfOfMaxCapacityAndFillItWithHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "192221e7-c883-42a1-b7b2-c123d442b512");
    auto& q = this->queue;
    constexpr auto MAX_CAP = TestFixture::Queue::MAX_CAPACITY;
    constexpr auto NEW_CAP = MAX_CAP / 2U;

    using element_t = typename TestFixture::Queue::element_t;
    iox::vector<element_t, MAX_CAP> removedElements;
    auto removeHandler = [&](const element_t& value) { removedElements.emplace_back(std::move(value)); };

    EXPECT_EQ(q.setCapacity(NEW_CAP, removeHandler), true);
    EXPECT_EQ(q.capacity(), NEW_CAP);
    EXPECT_EQ(removedElements.size(), 0U);

    uint64_t element = 0U;
    while (q.tryPush(element))
    {
        ++element;
    }

    EXPECT_EQ(q.capacity(), NEW_CAP);
    EXPECT_EQ(q.size(), NEW_CAP);
    EXPECT_EQ(element, NEW_CAP);
}

TYPED_TEST(MpmcResizeableLockFreeQueueTest, setCapacityFromHalfOfMaxCapacityToMaxCapacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "d01bd6a0-c22d-4995-87ca-115c13269339");
    auto& q = this->queue;
    constexpr auto MAX_CAP = TestFixture::Queue::MAX_CAPACITY;
    constexpr auto NEW_CAP = MAX_CAP / 2U;
    q.setCapacity(NEW_CAP);

    uint64_t element = 0U;
    while (q.tryPush(element))
    {
        ++element;
    }

    EXPECT_EQ(q.setCapacity(MAX_CAP), true);
    EXPECT_EQ(q.capacity(), MAX_CAP);
    EXPECT_EQ(q.size(), NEW_CAP);

    while (q.tryPush(element))
    {
        ++element;
    }

    // we want to find all elements we pushed
    for (element = 0U; element < MAX_CAP; ++element)
    {
        const auto result = q.pop();
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value(), element);
    }
}

TYPED_TEST(MpmcResizeableLockFreeQueueTest, setCapacityFromHalfOfMaxCapacityToMaxCapacityWithHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "04bcc542-c6aa-4865-9b60-4058ccee2904");
    auto& q = this->queue;
    constexpr auto MAX_CAP = TestFixture::Queue::MAX_CAPACITY;
    constexpr auto NEW_CAP = MAX_CAP / 2U;

    using element_t = typename TestFixture::Queue::element_t;
    iox::vector<element_t, MAX_CAP> removedElements;
    auto removeHandler = [&](const element_t& value) { removedElements.emplace_back(std::move(value)); };

    q.setCapacity(NEW_CAP);

    uint64_t element = 0U;
    while (q.tryPush(element))
    {
        ++element;
    }

    EXPECT_EQ(q.setCapacity(MAX_CAP, removeHandler), true);
    EXPECT_EQ(q.capacity(), MAX_CAP);
    EXPECT_EQ(q.size(), NEW_CAP);
    EXPECT_EQ(removedElements.size(), 0U);

    while (q.tryPush(element))
    {
        ++element;
    }

    // we want to find all elements we pushed
    for (element = 0U; element < MAX_CAP; ++element)
    {
        const auto result = q.pop();
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value(), element);
    }
}

TYPED_TEST(MpmcResizeableLockFreeQueueTest, setCapacityOfFullQueueToHalfOfMaxCapacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "9d1eb828-33fa-4e98-98c6-88c342b7ab53");
    auto& q = this->queue;
    constexpr auto MAX_CAP = TestFixture::Queue::MAX_CAPACITY;
    constexpr auto NEW_CAP = MAX_CAP / 2U;

    uint64_t element = 0U;
    while (q.tryPush(element))
    {
        ++element;
    };

    EXPECT_TRUE(q.setCapacity(NEW_CAP));
    EXPECT_EQ(q.capacity(), NEW_CAP);
    EXPECT_EQ(q.size(), NEW_CAP);

    // the least recent values are removed due to the capacity being decreased
    // how man elements remain depends on whether MAX_CAP is divisable by 2
    for (element = NEW_CAP + MAX_CAP % 2U; element < MAX_CAP; ++element)
    {
        auto result = q.pop();
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value(), element);
    }
}

TYPED_TEST(MpmcResizeableLockFreeQueueTest, setCapacityOfFullQueueToHalfOfMaxCapacityWithHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "b06d2d45-e6a3-47b3-865c-8b940adfac9d");
    auto& q = this->queue;
    constexpr auto MAX_CAP = TestFixture::Queue::MAX_CAPACITY;
    constexpr auto NEW_CAP = MAX_CAP / 2U;

    using element_t = typename TestFixture::Queue::element_t;
    iox::vector<element_t, MAX_CAP> removedElements;
    auto removeHandler = [&](const element_t& value) { removedElements.emplace_back(std::move(value)); };

    uint64_t element = 0U;
    while (q.tryPush(element))
    {
        ++element;
    };

    EXPECT_TRUE(q.setCapacity(NEW_CAP, removeHandler));
    EXPECT_EQ(q.capacity(), NEW_CAP);
    EXPECT_EQ(q.size(), NEW_CAP);
    EXPECT_EQ(removedElements.size(), MAX_CAP / 2 + MAX_CAP % 2U);

    // the least recent values are removed due to the capacity being decreased
    // how many elements remain depends on whether MAX_CAP is divisable by 2
    for (element = NEW_CAP + MAX_CAP % 2U; element < MAX_CAP; ++element)
    {
        auto result = q.pop();
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value(), element);
    }
}

// note this is one of the most general cases and necessary to test:
// decreasing the capacity starting with a partially filled queue and checking whether the last values
// remain (and the others are removed)
TYPED_TEST(MpmcResizeableLockFreeQueueTest, DecreaseCapacityOfAPartiallyFilledQueue)
{
    ::testing::Test::RecordProperty("TEST_ID", "fa79f745-212a-433b-a1f3-b1abbd6f845d");
    auto& q = this->queue;
    constexpr auto MAX_CAP = TestFixture::Queue::MAX_CAPACITY;

    const auto CAP = MAX_CAP / 2U;

    q.setCapacity(CAP);

    uint64_t element = 0U;
    while (q.tryPush(element++))
    {
    }

    const auto CAP2 = CAP + MAX_CAP / 4U; // roughly 3 quarters of max (integer division)
    q.setCapacity(CAP2);

    // queue is now partially filled with elements (neither full nor empty)
    // decrease the capacity
    // verify that the test was set up correctly

    EXPECT_EQ(q.capacity(), CAP2);
    EXPECT_EQ(q.size(), CAP);

    // decrease the capacity of the partially filled queue again

    const auto CAP3 = CAP2 - CAP; // roughly a quarter of max

    EXPECT_TRUE(q.setCapacity(CAP3));
    EXPECT_EQ(q.capacity(), CAP3);
    EXPECT_EQ(q.size(), CAP3);

    // are the remaining elements correct? (i.e. we did not remove too many elements)
    for (element = CAP - CAP3; element < CAP; ++element)
    {
        auto result = q.pop();
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value(), element);
    }

    // refill to verify the capacity can really be used

    element = 0U;
    while (q.tryPush(element++))
    {
    }


    for (element = 0U; element < CAP3; ++element)
    {
        auto result = q.pop();
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value(), element);
    }

    EXPECT_EQ(q.size(), 0U);
}

TYPED_TEST(MpmcResizeableLockFreeQueueTest, DecreaseCapacityOfAPartiallyFilledQueueWithHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "3b0f917b-fd9b-45d7-84b9-c0468cc86d70");
    auto& q = this->queue;
    constexpr auto MAX_CAP = TestFixture::Queue::MAX_CAPACITY;

    using element_t = typename TestFixture::Queue::element_t;
    iox::vector<element_t, MAX_CAP> removedElements;
    auto removeHandler = [&](const element_t& value) { removedElements.emplace_back(std::move(value)); };

    const auto CAP = MAX_CAP / 2U;

    q.setCapacity(CAP);

    uint64_t element = 0U;
    while (q.tryPush(element++))
    {
    }

    const auto CAP2 = CAP + MAX_CAP / 4U; // roughly 3 quarters of max (integer division)
    q.setCapacity(CAP2);

    // queue is now partially filled with elements (neither full nor empty)
    // decrease the capacity
    // verify that the test was set up correctly

    EXPECT_EQ(q.capacity(), CAP2);
    EXPECT_EQ(q.size(), CAP);

    // decrease the capacity of the partially filled queue again

    const auto CAP3 = CAP2 - CAP; // roughly a quarter of max

    EXPECT_TRUE(q.setCapacity(CAP3, removeHandler));
    EXPECT_EQ(q.capacity(), CAP3);
    EXPECT_EQ(q.size(), CAP3);

    // CAP3 elements remain, the first CAP - CAP3 elements are removed

    // were the least recent elements removed?
    EXPECT_EQ(removedElements.size(), CAP - CAP3);
    element = 0U;
    for (auto& removedElement : removedElements)
    {
        EXPECT_EQ(removedElement, element++);
    }

    // are the remaining elements correct? (i.e. we did not remove too many elements)
    for (element = CAP - CAP3; element < CAP; ++element)
    {
        auto result = q.pop();
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value(), element);
    }

    // refill to verify the capacity can really be used

    element = 0U;
    while (q.tryPush(element++))
    {
    }


    for (element = 0U; element < CAP3; ++element)
    {
        auto result = q.pop();
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value(), element);
    }

    EXPECT_EQ(q.size(), 0U);
}

} // namespace
