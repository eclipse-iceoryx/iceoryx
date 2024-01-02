// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iox/detail/mpmc_lockfree_queue/mpmc_index_queue.hpp"

namespace
{
using namespace ::testing;
using iox::concurrent::MpmcIndexQueue;

template <typename T>
class MpmcIndexQueueTest : public ::testing::Test
{
  public:
    using Queue = T;
    using index_t = typename Queue::value_t;

    Queue queue;
    Queue fullQueue{Queue::ConstructFull};
};

TEST(MpmcIndexQueueTest, capacityIsConsistent)
{
    ::testing::Test::RecordProperty("TEST_ID", "86d94598-7271-45a1-a6c9-afad0bc8cc8b");
    MpmcIndexQueue<37U> q;
    EXPECT_EQ(q.capacity(), 37U);
}

typedef ::testing::Types<MpmcIndexQueue<1>, MpmcIndexQueue<10>, MpmcIndexQueue<1000>> TestQueues;

TYPED_TEST_SUITE(MpmcIndexQueueTest, TestQueues, );

TYPED_TEST(MpmcIndexQueueTest, defaultConstructedQueueIsEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "7fde1929-62b9-4377-8d02-5bcdd18c246f");
    auto& q = this->queue;
    EXPECT_TRUE(q.empty());
}

TYPED_TEST(MpmcIndexQueueTest, constructedQueueIsEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "e38bb9be-720b-4aba-a17a-f7bdaa34164a");
    using Queue = typename TestFixture::Queue;

    Queue q(Queue::ConstructEmpty);
    EXPECT_TRUE(q.empty());
}


TYPED_TEST(MpmcIndexQueueTest, queueIsNotEmptyAfterPush)
{
    ::testing::Test::RecordProperty("TEST_ID", "e9150ab1-ca01-44bf-b0bb-eab242c3a73f");
    auto& q = this->queue;
    auto index = this->fullQueue.pop();

    ASSERT_TRUE(index.has_value());
    // NOLINTJUSTIFICATION false positive, we checked that index.has_value() and the fullQueue has initialized every
    // value with Queue::ConstructFull
    // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
    q.push(index.value());
    EXPECT_FALSE(q.empty());
}

TYPED_TEST(MpmcIndexQueueTest, queueIsEmptyAgainAfterPushFollowedByPop)
{
    ::testing::Test::RecordProperty("TEST_ID", "38c6eba6-4c21-4df1-9f47-f3bf9438b5b1");
    auto& q = this->queue;

    auto index = this->fullQueue.pop();

    ASSERT_TRUE(index.has_value());
    // NOLINTJUSTIFICATION false positive, we checked that index.has_value() and the fullQueue has initialized every
    // value with Queue::ConstructFull
    // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
    q.push(index.value());
    EXPECT_FALSE(q.empty());
    q.pop();
    EXPECT_TRUE(q.empty());
}

TYPED_TEST(MpmcIndexQueueTest, IndicesAreIncreasingWhenConstructedFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "b83d43f5-fa30-417e-aa2e-3a69408e7869");
    using Queue = typename TestFixture::Queue;
    using index_t = typename TestFixture::index_t;

    Queue& q = this->fullQueue;
    EXPECT_FALSE(q.empty());

    index_t expected{0U};
    auto index = q.pop();
    while (index.has_value())
    {
        EXPECT_EQ(index.value(), expected++);
        index = q.pop();
    }
}


TYPED_TEST(MpmcIndexQueueTest, queueIsNotEmptyWhenConstructedFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "6a4ee1da-e70b-49d8-bfbe-0ef6d4aacf8c");
    using Queue = typename TestFixture::Queue;

    Queue& q = this->fullQueue;

    EXPECT_FALSE(q.empty());
}

TYPED_TEST(MpmcIndexQueueTest, queueIsEmptyWhenPopFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "af8bc70d-b0d3-48ff-8322-17722e01a490");
    using Queue = typename TestFixture::Queue;

    Queue& q = this->fullQueue;

    EXPECT_FALSE(q.empty());

    auto index = q.pop();
    while (index.has_value())
    {
        index = q.pop();
    }

    EXPECT_TRUE(q.empty());
}

TYPED_TEST(MpmcIndexQueueTest, pushAndPopSingleElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "983b068f-2847-4813-bf19-3ae713261576");
    using index_t = typename TestFixture::index_t;
    auto& q = this->queue;
    auto index = this->fullQueue.pop();

    ASSERT_TRUE(index.has_value());
    // NOLINTJUSTIFICATION false positive, we checked that index.has_value() and the fullQueue has initialized every
    // value with Queue::ConstructFull
    // NOLINTNEXTLINE(clang-analyzer-core.uninitialized.Assign)
    index_t rawIndex = index.value();

    EXPECT_TRUE(index.has_value());

    q.push(index.value());

    auto popped = q.pop();

    ASSERT_TRUE(popped.has_value());
    EXPECT_EQ(popped.value(), rawIndex);
}

TYPED_TEST(MpmcIndexQueueTest, poppedElementsAreInFifoOrder)
{
    ::testing::Test::RecordProperty("TEST_ID", "7004d51a-9a2f-4ac9-90bc-bb755b393431");
    auto& q = this->queue;
    using index_t = typename TestFixture::index_t;

    auto capacity = q.capacity();
    index_t expected{0U};

    for (uint64_t i = 0U; i < capacity; ++i)
    {
        auto index = this->fullQueue.pop();
        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), expected++);
        q.push(index.value());
    }

    expected = 0;
    for (uint64_t i = 0U; i < capacity; ++i)
    {
        auto popped = q.pop();
        ASSERT_TRUE(popped.has_value());
        EXPECT_EQ(popped.value(), expected++);
    }
}

TYPED_TEST(MpmcIndexQueueTest, popReturnsNothingWhenQueueIsEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "20487320-1c70-4212-aa00-847105780a71");
    auto& q = this->queue;
    EXPECT_FALSE(q.pop().has_value());
}


TYPED_TEST(MpmcIndexQueueTest, popIfFullReturnsNothingWhenQueueIsEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "68773e9d-bc67-4929-ba4d-05197bbdfcac");
    auto& q = this->queue;
    EXPECT_FALSE(q.popIfFull().has_value());
}


TYPED_TEST(MpmcIndexQueueTest, popIfFullReturnsOldestElementWhenQueueIsFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "a4479ef7-a19d-4f4d-a1af-d2abb8f6fa0c");
    using Queue = typename TestFixture::Queue;
    Queue& q = this->fullQueue;

    auto index = q.popIfFull();
    EXPECT_TRUE(index.has_value());
    EXPECT_EQ(index.value(), 0U);
}

TYPED_TEST(MpmcIndexQueueTest, popIfFullReturnsNothingWhenQueueIsNotFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "932a6096-4b56-42fc-8f9d-06a9fcf4c7d3");
    using Queue = typename TestFixture::Queue;
    Queue& q = this->fullQueue;

    auto index = q.pop();
    EXPECT_TRUE(index.has_value());
    EXPECT_FALSE(q.popIfFull().has_value());
}

TYPED_TEST(MpmcIndexQueueTest, popIfSizeIsAtLeastReturnsNothingIfQueueIsEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "415c5f5c-fc0d-45e3-994d-4ce7af9a9e98");
    auto& q = this->queue;
    EXPECT_FALSE(q.popIfSizeIsAtLeast(1U).has_value());
}

TYPED_TEST(MpmcIndexQueueTest, popIfSizeIsAtLeastZeroReturnsIndexIfQueueIsFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "1cb99149-8a34-43bf-ab6b-3744cb18889e");
    auto& q = this->fullQueue;
    EXPECT_TRUE(q.popIfSizeIsAtLeast(0U).has_value());
}

TYPED_TEST(MpmcIndexQueueTest, popIfSizeIsAtLeastZeroReturnsNothingIfQueueIsEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "74ba9355-4ec6-49ce-b4fc-d01b820d9bd1");
    auto& q = this->queue;
    EXPECT_FALSE(q.popIfSizeIsAtLeast(0U).has_value());
}

TYPED_TEST(MpmcIndexQueueTest, popIfSizeIsAtLeastZeroReturnsIndexIfQueueContainsOneElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "fb58c76c-1bd0-4a5b-8732-63832f3a7fd9");
    auto& q = this->queue;
    auto index = this->fullQueue.pop();
    ASSERT_TRUE(index.has_value());
    // NOLINTJUSTIFICATION false positive, we checked that index.has_value() and the fullQueue has initialized every
    // value with Queue::ConstructFull
    // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
    q.push(*index);

    index = q.popIfSizeIsAtLeast(0U);
    ASSERT_TRUE(index.has_value());
}

TYPED_TEST(MpmcIndexQueueTest, popIfSizeIsAtLeastOneReturnsIndexIfQueueContainsOneElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "d97a588c-c847-401e-82b6-db58f791fbf4");
    auto& q = this->queue;
    using index_t = typename TestFixture::index_t;

    // we can only push indices up to capacity - 1
    const index_t expectedIndex{q.capacity() - 1};
    q.push(expectedIndex);

    const auto index = q.popIfSizeIsAtLeast(1U);
    ASSERT_TRUE(index.has_value());
    EXPECT_EQ(*index, expectedIndex);
}

TYPED_TEST(MpmcIndexQueueTest, popIfSizeIsAtLeastTwoReturnsNothingIfQueueContainsOneElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "6338b9d1-235f-4af3-9eca-3daeca25f824");
    auto& q = this->queue;
    q.push(0U);
    const auto index = q.popIfSizeIsAtLeast(2U);
    ASSERT_FALSE(index.has_value());
}

TYPED_TEST(MpmcIndexQueueTest, popIfSizeIsAtLeastCapacityReturnsIndexIfQueueIsFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "5582c206-7c42-4ce3-821d-9e01085e72c5");
    const auto c = this->fullQueue.capacity();
    const auto index = this->fullQueue.popIfSizeIsAtLeast(c);
    ASSERT_TRUE(index.has_value());
    EXPECT_EQ(*index, 0U);
}

TYPED_TEST(MpmcIndexQueueTest, popIfSizeIsAtLeastCapacityReturnsNothingIfQueueIsNotFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "d4c944bd-56d9-4dea-9d27-b346cf42e46c");
    const auto CAP = this->fullQueue.capacity();
    this->fullQueue.pop();
    const auto index = this->fullQueue.popIfSizeIsAtLeast(CAP);
    ASSERT_FALSE(index.has_value());
}

} // namespace
