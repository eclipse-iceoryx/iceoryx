// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 by Latitude AI. All rights reserved.
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

#include "iox/detail/mpmc_lockfree_queue.hpp"
#include "iox/detail/mpmc_resizeable_lockfree_queue.hpp"

// We test the common functionality of LockFreeQueue and ResizableLockFreeQueue here
// in typed tests to reduce code duplication.

namespace
{
using namespace ::testing;

// use a non-POD type for testing (just a boxed version of int).
// We use implicit conversions of int to Integer to be able use the same test structure
// for Integer and int (primarily for tryPush).
// This allows testing PODs and Custom Types with the same test structure.
struct Integer
{
    // NOLINTNEXTLINE(hicpp-explicit-conversions) required for typed tests
    Integer(int value = 0)
        : value(value)
    {
    }

    int value{0};

    // so that it behaves like an int for comparison purposes
    // NOLINTNEXTLINE(hicpp-explicit-conversions) required for typed tests
    operator int() const
    {
        return value;
    }
};

// non-POD type used to ensure that the queue supports move-only types.
struct MoveOnlyInteger : public Integer
{
    // NOLINTNEXTLINE(hicpp-explicit-conversions) required for typed tests
    MoveOnlyInteger(int value = 0)
        : Integer(value)
    {
    }

    MoveOnlyInteger(const MoveOnlyInteger&) = delete;
    MoveOnlyInteger& operator=(const MoveOnlyInteger&) = delete;
    MoveOnlyInteger(MoveOnlyInteger&&) = default;
    MoveOnlyInteger& operator=(MoveOnlyInteger&&) = default;

    ~MoveOnlyInteger() = default;
};

template <typename Config>
class MpmcLockFreeQueueTest : public ::testing::Test
{
  public:
    void SetUp() override
    {
        // reduce capacity before running the tests if required by config
        setCapacity<Config>();
    }

    void fillQueue(int start = 0)
    {
        int data{start};
        for (uint64_t i = 0; i < queue.capacity(); ++i)
        {
            queue.tryPush(data);
            data++;
        }
    }

    // only set Capacity if DynamicCapacity is smaller
    // since some queue types may not even provide the option to call setCapacity
    // this must be done at compile time
    // (the additional template indirection via Config_ is required for SFINAE)
    template <typename Config_>
    typename std::enable_if<Config_::SetCapacityInitially, void>::type setCapacity()
    {
        queue.setCapacity(Config::DynamicCapacity);
    }

    template <typename Config_>
    typename std::enable_if<!Config_::SetCapacityInitially, void>::type setCapacity()
    {
    }

    using Queue = typename Config::QueueType;
    Queue queue;
};

template <size_t Capacity>
using IntegerQueue = iox::concurrent::MpmcLockFreeQueue<Integer, Capacity>;

// define the test configurations with varying types, capacities and dynamically reduced capacities

template <template <typename, uint64_t> class QueueType_,
          typename ElementType_,
          uint64_t Capacity_,
          uint64_t DynamicCapacity_ = Capacity_>
struct Config
{
    static constexpr uint64_t Capacity = Capacity_;
    static constexpr uint64_t DynamicCapacity = DynamicCapacity_;
    static_assert(DynamicCapacity <= Capacity, "DynamicCapacity can be at most Capacity");

    using QueueType = QueueType_<ElementType_, Capacity>;
    static constexpr bool SetCapacityInitially = DynamicCapacity < Capacity;
};

template <typename T, uint64_t C>
using LFQueue = iox::concurrent::MpmcLockFreeQueue<T, C>;

template <typename T, uint64_t C>
using RLFQueue = iox::concurrent::MpmcResizeableLockFreeQueue<T, C>;


template <template <typename, uint64_t> class QueueType, typename ElementType, uint64_t Capacity>
using Full = Config<QueueType, ElementType, Capacity>;

template <template <typename, uint64_t> class QueueType, typename ElementType, uint64_t Capacity>
using AlmostFull = Config<QueueType, ElementType, Capacity, (Capacity > 1) ? (Capacity - 1) : Capacity>;

template <template <typename, uint64_t> class QueueType, typename ElementType, uint64_t Capacity>
using HalfFull = Config<QueueType, ElementType, Capacity, (Capacity > 1) ? (Capacity / 2) : Capacity>;

template <template <typename, uint64_t> class QueueType, typename ElementType, uint64_t Capacity>
using AlmostEmpty = Config<QueueType, ElementType, Capacity, 1>;

// configs of the lockfree queue without resize
using LFFull1 = Full<LFQueue, int, 1>;
using LFFull2 = Full<LFQueue, int, 1000>;
using LFFull3 = Full<LFQueue, Integer, 100>;
using LFFull4 = Full<LFQueue, MoveOnlyInteger, 10>;

// configs of the resizeable lockfree queue
using Full1 = Full<RLFQueue, Integer, 1>;
using Full2 = Full<RLFQueue, Integer, 10>;
using Full3 = Full<RLFQueue, int, 1000>;
using Full4 = Full<RLFQueue, MoveOnlyInteger, 100>;

using AlmostFull1 = AlmostFull<RLFQueue, Integer, 10>;
using AlmostFull2 = AlmostFull<RLFQueue, int, 1000>;
using AlmostFull3 = AlmostFull<RLFQueue, MoveOnlyInteger, 100>;

using HalfFull1 = HalfFull<RLFQueue, Integer, 10>;
using HalfFull2 = HalfFull<RLFQueue, int, 1000>;
using HalfFull3 = HalfFull<RLFQueue, MoveOnlyInteger, 100>;

using AlmostEmpty1 = AlmostEmpty<RLFQueue, Integer, 10>;
using AlmostEmpty2 = AlmostEmpty<RLFQueue, int, 1000>;
using AlmostEmpty3 = AlmostEmpty<RLFQueue, MoveOnlyInteger, 100>;

typedef ::testing::Types<LFFull1,
                         LFFull2,
                         LFFull3,
                         LFFull4,
                         Full1,
                         Full2,
                         Full3,
                         Full4,
                         AlmostFull1,
                         AlmostFull2,
                         AlmostFull3,
                         HalfFull1,
                         HalfFull2,
                         HalfFull3,
                         AlmostEmpty1,
                         AlmostEmpty2,
                         AlmostEmpty3>
    TestConfigs;

TYPED_TEST_SUITE(MpmcLockFreeQueueTest, TestConfigs, );

TEST(MpmcLockFreeQueueTest, capacityIsConsistent)
{
    ::testing::Test::RecordProperty("TEST_ID", "0b56ef76-2eac-4174-9999-e26495758e6a");
    constexpr uint64_t CAPACITY{37};
    IntegerQueue<CAPACITY> q;
    EXPECT_EQ(q.capacity(), CAPACITY);
}

TYPED_TEST(MpmcLockFreeQueueTest, constructedQueueIsEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "9bb8a86e-c3d0-44ef-9fb7-999f50f0c4ac");
    auto& q = this->queue;
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0);
}

TYPED_TEST(MpmcLockFreeQueueTest, pushAndPopSingleElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "381e13c3-d5dc-40c1-b4fd-65634578bdc8");
    auto& q = this->queue;

    int data = 42;
    EXPECT_TRUE(q.tryPush(data));
    EXPECT_EQ(q.size(), 1);

    auto x = q.pop();
    ASSERT_TRUE(x.has_value());
    EXPECT_EQ(x.value(), 42);
    EXPECT_EQ(q.size(), 0);
}

TYPED_TEST(MpmcLockFreeQueueTest, popFromEmptyQueueReturnsNothing)
{
    ::testing::Test::RecordProperty("TEST_ID", "f9d4f232-e9e7-4089-bf61-2696a4bdc8f3");
    auto& q = this->queue;

    int data = 24;
    q.tryPush(data);
    q.pop();
    EXPECT_FALSE(q.pop().has_value());
    EXPECT_EQ(q.size(), 0);
}

TYPED_TEST(MpmcLockFreeQueueTest, tryPushUntilFullCapacityIsUsed)
{
    ::testing::Test::RecordProperty("TEST_ID", "6c6bb533-aab5-46bc-8368-977fc2503a74");
    auto& q = this->queue;
    auto capacity = q.capacity();

    int data{0};
    for (uint64_t i = 0; i < capacity; ++i)
    {
        EXPECT_EQ(q.size(), i);
        EXPECT_TRUE(q.tryPush(data));
        data++;
    }

    EXPECT_EQ(q.size(), capacity);
}

TYPED_TEST(MpmcLockFreeQueueTest, tryPushInFullQueueFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "0b793a3e-1e47-46d9-91c2-4967569b508b");
    auto& q = this->queue;
    this->fillQueue(38);
    int data{37};
    EXPECT_FALSE(q.tryPush(data));
}

TYPED_TEST(MpmcLockFreeQueueTest, poppedElementsAreInFifoOrder)
{
    ::testing::Test::RecordProperty("TEST_ID", "fecc997a-ae8e-49d3-a1ea-7ec07b5a93ed");
    auto& q = this->queue;
    auto capacity = q.capacity();

    // scramble the start value to avoid false positives
    // due to memory values of previous tests on the stack ...
    int value = 73;
    this->fillQueue(value);

    for (uint64_t i = capacity; i > 0; --i)
    {
        EXPECT_EQ(q.size(), i);
        auto x = q.pop();
        ASSERT_TRUE(x.has_value());
        EXPECT_EQ(x.value(), value);
        ++value;
    }
    EXPECT_FALSE(q.pop().has_value());
    EXPECT_EQ(q.size(), 0);
}

TYPED_TEST(MpmcLockFreeQueueTest, pushDoesNotOverflowIfQueueIsNotFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "2096033c-5631-480e-8b9c-a8b472721cdb");
    auto& q = this->queue;
    auto capacity = q.capacity();

    int start{66};
    int data{start};
    for (uint64_t i = 0; i < capacity; ++i)
    {
        auto x = q.push(data);
        EXPECT_FALSE(x.has_value());
        data++;
    }
}

TYPED_TEST(MpmcLockFreeQueueTest, pushReturnsOldestElementOnOverflow)
{
    ::testing::Test::RecordProperty("TEST_ID", "df40eac8-11ba-4352-aef0-c1c4785a43f8");
    auto& q = this->queue;
    auto capacity = q.capacity();

    int start{666};
    this->fillQueue(start);

    int data{-start};
    for (uint64_t i = 0; i < capacity; ++i)
    {
        auto x = q.push(data);
        ASSERT_TRUE(x.has_value());
        EXPECT_EQ(x.value(), start);
        data--;
        start++;
    }
}

TYPED_TEST(MpmcLockFreeQueueTest, pushInsertsInFifoOrder)
{
    ::testing::Test::RecordProperty("TEST_ID", "18698a63-de51-407a-a2f0-dce591c92223");
    auto& q = this->queue;
    auto capacity = q.capacity();

    int start{69};
    this->fillQueue(start);

    int value{-start};
    for (uint64_t i = 0; i < capacity; ++i)
    {
        q.push(value);
        value--;
    }

    value = -start;
    for (uint64_t i = 0; i < capacity; ++i)
    {
        auto x = q.pop();
        ASSERT_TRUE(x.has_value());
        EXPECT_EQ(x.value(), value);
        value--;
    }
}

TYPED_TEST(MpmcLockFreeQueueTest, checkEmptynessAfterOneElementWasPushedandPopped)
{
    ::testing::Test::RecordProperty("TEST_ID", "ef49cb74-9631-4804-b040-36cc28c2abfc");
    auto& q = this->queue;

    q.tryPush(37);
    auto x = q.pop();

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0);
}

TYPED_TEST(MpmcLockFreeQueueTest, checkEmptynessAfterFullQueueWasEmptied)
{
    ::testing::Test::RecordProperty("TEST_ID", "fb0d2c27-25e6-41c6-a51a-909a01ec0052");
    auto& q = this->queue;
    auto capacity = q.capacity();

    int start{73};
    this->fillQueue(start);

    for (uint64_t i = 0; i < capacity; ++i)
    {
        auto x = q.pop();
    }

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0);
}

} // namespace
