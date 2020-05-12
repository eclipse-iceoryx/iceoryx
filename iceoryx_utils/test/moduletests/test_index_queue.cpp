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

#include "iceoryx_utils/internal/lockfree_queue/index_queue.hpp"
using namespace ::testing;

namespace
{
using iox::IndexQueue;

template <typename T>
class IndexQueueTest : public ::testing::Test
{
  public:
    using Queue = T;
    using index_t = typename Queue::value_t;

  protected:
    IndexQueueTest()
    {
    }

    ~IndexQueueTest()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

    Queue queue;
    Queue fullQueue{Queue::ConstructFull};
};

TEST(LockFreeQueueTest, capacityIsConsistent)
{
    IndexQueue<37> q;
    EXPECT_EQ(q.capacity(), 37);
}

typedef ::testing::Types<IndexQueue<1>, IndexQueue<10>, IndexQueue<1000>> TestQueues;
// typedef ::testing::Types<IndexQueue<10>> TestQueues;


TYPED_TEST_CASE(IndexQueueTest, TestQueues);


TYPED_TEST(IndexQueueTest, defaultConstructedQueueIsEmpty)
{
    auto& q = this->queue;
    EXPECT_TRUE(q.empty());
}

TYPED_TEST(IndexQueueTest, constructedQueueIsEmpty)
{
    using Queue = typename TestFixture::Queue;
    using index_t = typename TestFixture::index_t;

    Queue q(Queue::ConstructEmpty);
    EXPECT_TRUE(q.empty());
}


TYPED_TEST(IndexQueueTest, queueIsNotEmptyAfterPush)
{
    using index_t = typename TestFixture::index_t;
    auto& q = this->queue;
    auto index = this->fullQueue.pop();

    q.push(index);
    EXPECT_FALSE(q.empty());
}

TYPED_TEST(IndexQueueTest, queueIsEmptyAgainAfterPushFollowedByPop)
{
    using index_t = typename TestFixture::index_t;
    auto& q = this->queue;

    auto index = this->fullQueue.pop();

    q.push(index);
    EXPECT_FALSE(q.empty());
    q.pop();
    EXPECT_TRUE(q.empty());
}

TYPED_TEST(IndexQueueTest, IndicesAreIncreasingWhenConstructedFull)
{
    using Queue = typename TestFixture::Queue;
    using index_t = typename TestFixture::index_t;

    Queue& q = this->fullQueue;
    EXPECT_FALSE(q.empty());

    index_t expected{0};
    auto index = q.pop();
    while (index.is_valid())
    {
        EXPECT_EQ(index, expected++);
        index = q.pop();
    }
}


TYPED_TEST(IndexQueueTest, queueIsNotEmptyWhenConstructedFull)
{
    using Queue = typename TestFixture::Queue;

    Queue& q = this->fullQueue;

    EXPECT_FALSE(q.empty());
}

TYPED_TEST(IndexQueueTest, queueIsEmptyWhenPopFails)
{
    using Queue = typename TestFixture::Queue;

    Queue& q = this->fullQueue;

    EXPECT_FALSE(q.empty());

    auto index = q.pop();
    while (index.is_valid())
    {
        index = q.pop();
    }

    EXPECT_TRUE(q.empty());
}

TYPED_TEST(IndexQueueTest, pushAndPopSingleElement)
{
    auto& q = this->queue;
    auto index = this->fullQueue.pop();

    q.push(index);
    auto popped = q.pop();

    EXPECT_TRUE(popped.is_valid());
    EXPECT_EQ(popped, index);
}

TYPED_TEST(IndexQueueTest, poppedElementsAreInFifoOrder)
{
    auto& q = this->queue;
    using index_t = typename TestFixture::index_t;

    auto capacity = q.capacity();
    index_t expected{0};

    for (uint64_t i = 0; i < capacity; ++i)
    {
        auto index = this->fullQueue.pop();
        EXPECT_EQ(index, expected++);
        q.push(index);
    }

    expected = 0;
    for (uint64_t i = 0; i < capacity; ++i)
    {
        auto popped = q.pop();
        ASSERT_TRUE(popped.is_valid());
        EXPECT_EQ(popped, expected++);
    }
}

TYPED_TEST(IndexQueueTest, popReturnsNothingWhenQueueIsEmpty)
{
    auto& q = this->queue;
    EXPECT_FALSE(q.pop().is_valid());
}


TYPED_TEST(IndexQueueTest, popIfFullReturnsNothingWhenQueueIsEmpty)
{
    auto& q = this->queue;
    EXPECT_FALSE(q.popIfFull().is_valid());
}


TYPED_TEST(IndexQueueTest, popIfFullReturnsOldestElementWhenQueueIsFull)
{
    using Queue = typename TestFixture::Queue;
    Queue& q = this->fullQueue;

    auto index = q.popIfFull();
    EXPECT_TRUE(index.is_valid());
    EXPECT_EQ(index, 0);
}

TYPED_TEST(IndexQueueTest, popIfFullReturnsNothingWhenQueueIsNotFull)
{
    using Queue = typename TestFixture::Queue;
    Queue& q = this->fullQueue;

    auto index = q.pop();
    EXPECT_TRUE(index.is_valid());
    EXPECT_FALSE(q.popIfFull().is_valid());
}

}