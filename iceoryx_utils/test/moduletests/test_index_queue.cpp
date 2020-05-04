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

#if 0
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

    index_t index{0};
    q.push(index);
    EXPECT_FALSE(q.empty());
}

TYPED_TEST(IndexQueueTest, queueIsEmptyAgainAfterPushFollowedByPop)
{
    using index_t = typename TestFixture::index_t;
    auto& q = this->queue;

    index_t index{0};
    q.push(index);
    EXPECT_FALSE(q.empty());
    q.pop(index);
    EXPECT_TRUE(q.empty());
}

TYPED_TEST(IndexQueueTest, IndicesAreIncreasingWhenConstructedFull)
{
    using Queue = typename TestFixture::Queue;
    using index_t = typename TestFixture::index_t;

    Queue q(Queue::ConstructFull);
    EXPECT_FALSE(q.empty());

    index_t index;
    index_t expected{0};

    while (q.pop(index))
    {
        EXPECT_EQ(index, expected++);
    }
    EXPECT_EQ(expected, q.capacity());
}

TYPED_TEST(IndexQueueTest, queueIsNotEmptyWhenConstructedFull)
{
    using Queue = typename TestFixture::Queue;
    using index_t = typename TestFixture::index_t;

    Queue q(Queue::ConstructFull);
    EXPECT_FALSE(q.empty());

    EXPECT_FALSE(q.empty());
}

TYPED_TEST(IndexQueueTest, queueIsEmptyWhenPopFails)
{
    using Queue = typename TestFixture::Queue;
    using index_t = typename TestFixture::index_t;

    Queue q(Queue::ConstructFull);
    EXPECT_FALSE(q.empty());

    index_t index;
    while (q.pop(index))
    {
    }

    EXPECT_TRUE(q.empty());
}

TYPED_TEST(IndexQueueTest, pushAndPopSingleElement)
{
    auto& q = this->queue;
    using index_t = typename TestFixture::index_t;

    index_t maxIndex{q.capacity() - 1};
    index_t index{maxIndex};
    q.push(index);

    index = 0;
    EXPECT_TRUE(q.pop(index));
    EXPECT_EQ(index, maxIndex);
}

TYPED_TEST(IndexQueueTest, poppedElementsAreInFifoOrder)
{
    auto& q = this->queue;
    using index_t = typename TestFixture::index_t;

    auto capacity = q.capacity();
    index_t maxIndex{capacity - 1};
    index_t index{maxIndex};

    for (uint64_t i = 0; i < capacity; ++i)
    {
        q.push(index--);
    }

    index_t expected{maxIndex};
    for (uint64_t i = 0; i < capacity; ++i)
    {
        ASSERT_TRUE(q.pop(index));
        EXPECT_EQ(index, expected);
        --expected;
    }
    EXPECT_FALSE(q.pop(index));
}

TYPED_TEST(IndexQueueTest, popReturnsNothingWhenQueueIsEmpty)
{
    auto& q = this->queue;
    using index_t = typename TestFixture::index_t;
    index_t index;

    EXPECT_FALSE(q.pop(index));
}

TYPED_TEST(IndexQueueTest, popIfFullReturnsNothingWhenQueueIsEmpty)
{
    auto& q = this->queue;
    using index_t = typename TestFixture::index_t;
    index_t index;

    EXPECT_FALSE(q.popIfFull(index));
}

TYPED_TEST(IndexQueueTest, popIfFullReturnsOldestElementWhenQueueIsFull)
{
    using Queue = typename TestFixture::Queue;
    using index_t = typename TestFixture::index_t;

    Queue q(Queue::ConstructFull);

    index_t index{73};

    EXPECT_TRUE(q.popIfFull(index));
    EXPECT_EQ(index, 0);
}

TYPED_TEST(IndexQueueTest, popIfFullReturnsNothingWhenQueueIsNotFull)
{
    using Queue = typename TestFixture::Queue;
    using index_t = typename TestFixture::index_t;

    Queue q(Queue::ConstructFull);

    index_t index;
    EXPECT_TRUE(q.pop(index));

    EXPECT_FALSE(q.popIfFull(index));
}

} // namespace

#endif