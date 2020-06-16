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

#include "test.hpp"

#include "iceoryx_utils/concurrent/lockfree_queue.hpp"
using namespace ::testing;

namespace
{
// use a non-POD type for testing (just a boxed version of int)
struct Integer
{
    Integer(int value = 0)
        : value(value)
    {
    }

    int value{0};

    // so that it behaves like an int for comparison purposes
    operator int() const
    {
        return value;
    }
};

template <typename T>
class LockFreeQueueTest : public ::testing::Test
{
  protected:
    LockFreeQueueTest()
    {
    }

    ~LockFreeQueueTest()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
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

    using Queue = T;
    Queue queue;
};

template <size_t Capacity>
using IntegerQueue = iox::concurrent::LockFreeQueue<Integer, Capacity>;

template <size_t Capacity>
using IntQueue = iox::concurrent::LockFreeQueue<int, Capacity>;

TEST(LockFreeQueueTest, capacityIsConsistent)
{
    IntegerQueue<37> q;
    EXPECT_EQ(q.capacity(), 37);
}

// note that we use implicit conversions of int to Integer to be able use the same test structure
// for Integer and int (primarily for tryPush)
typedef ::testing::Types<IntegerQueue<1>, IntegerQueue<10>, IntQueue<10>> TestQueues;

TYPED_TEST_CASE(LockFreeQueueTest, TestQueues);

TYPED_TEST(LockFreeQueueTest, constructedQueueIsEmpty)
{
    auto& q = this->queue;
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0);
}

TYPED_TEST(LockFreeQueueTest, pushAndPopSingleElement)
{
    auto& q = this->queue;

    int data = 42;
    EXPECT_TRUE(q.tryPush(data));
    EXPECT_EQ(q.size(), 1);

    auto x = q.pop();
    ASSERT_TRUE(x.has_value());
    EXPECT_EQ(x.value(), 42);
    EXPECT_EQ(q.size(), 0);
}

TYPED_TEST(LockFreeQueueTest, popFromEmptyQueueReturnsNothing)
{
    auto& q = this->queue;

    int data = 24;
    q.tryPush(data);
    q.pop();
    EXPECT_FALSE(q.pop().has_value());
    EXPECT_EQ(q.size(), 0);
}

TYPED_TEST(LockFreeQueueTest, tryPushUntilFullCapacityIsUsed)
{
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

TYPED_TEST(LockFreeQueueTest, tryPushInFullQueueFails)
{
    auto& q = this->queue;
    this->fillQueue(38);
    int data{37};
    EXPECT_FALSE(q.tryPush(data));
}

TYPED_TEST(LockFreeQueueTest, poppedElementsAreInFifoOrder)
{
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

TYPED_TEST(LockFreeQueueTest, pushDoesNotOverflowIfQueueIsNotFull)
{
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

TYPED_TEST(LockFreeQueueTest, pushReturnsOldestElementOnOverflow)
{
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

TYPED_TEST(LockFreeQueueTest, pushInsertsInFifoOrder)
{
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

TYPED_TEST(LockFreeQueueTest, checkEmptynessAfterOneElementWasPushedandPopped)
{
    auto& q = this->queue;

    q.tryPush(37);
    auto x = q.pop();

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0);
}

TYPED_TEST(LockFreeQueueTest, checkEmptynessAfterFullQueueWasEmptied)
{
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