// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/concurrent/lockfree_queue.hpp"
#include "iceoryx_hoofs/concurrent/resizeable_lockfree_queue.hpp"
#include "iceoryx_hoofs/internal/concurrent/fifo.hpp"
#include "iceoryx_hoofs/internal/concurrent/trigger_queue.hpp"
#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;

using namespace iox;
using namespace iox::concurrent;

template <typename QueueType>
class TriggerQueue_test : public Test
{
  public:
    void SetUp() override
    {
        m_watchdog.watchAndActOnFailure([] { std::terminate(); });
    }

    void TearDown() override
    {
    }

    bool fillQueue(const uint64_t offset = 0U)
    {
        for (uint64_t i = 0U; i < this->m_sut.capacity(); ++i)
        {
            EXPECT_THAT(this->m_sut.push(i + offset), Eq(true));
            EXPECT_THAT(this->m_sut.empty(), Eq(false));
            EXPECT_THAT(this->m_sut.size(), Eq(i + 1U));
        }
        return this->m_sut.size() == this->m_sut.capacity();
    }

    QueueType m_sut;
    Watchdog m_watchdog{units::Duration::fromSeconds(2)};
};

using TriggerQueueTestSubjects = Types<TriggerQueue<uint64_t, 1, FiFo>,
                                       TriggerQueue<uint64_t, 10, FiFo>,
                                       TriggerQueue<uint64_t, 100, FiFo>,
                                       TriggerQueue<uint64_t, 1, LockFreeQueue>,
                                       TriggerQueue<uint64_t, 10, LockFreeQueue>,
                                       TriggerQueue<uint64_t, 100, LockFreeQueue>,
                                       TriggerQueue<uint64_t, 1, ResizeableLockFreeQueue>,
                                       TriggerQueue<uint64_t, 10, ResizeableLockFreeQueue>,
                                       TriggerQueue<uint64_t, 100, ResizeableLockFreeQueue>>;

TYPED_TEST_SUITE(TriggerQueue_test, TriggerQueueTestSubjects);


TYPED_TEST(TriggerQueue_test, EmptyOnConstruction)
{
    ::testing::Test::RecordProperty("TEST_ID", "e318311d-88fb-4014-8f4e-35fe539bd0e8");
    EXPECT_THAT(this->m_sut.empty(), Eq(true));
    EXPECT_THAT(this->m_sut.size(), Eq(0U));
}

TYPED_TEST(TriggerQueue_test, PushOneElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "6c7f57fa-6656-479c-b7e2-dfe2d68b114a");
    EXPECT_THAT(this->m_sut.push(5U), Eq(true));
    EXPECT_THAT(this->m_sut.empty(), Eq(false));
    EXPECT_THAT(this->m_sut.size(), Eq(1U));
}

TYPED_TEST(TriggerQueue_test, PushTillFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "25dfabda-d873-4681-89e8-8d0741c30ab4");
    EXPECT_TRUE(this->fillQueue());
}

TYPED_TEST(TriggerQueue_test, PopOnEmptyReturnsNullopt)
{
    ::testing::Test::RecordProperty("TEST_ID", "6e58cfb9-9271-4dbb-bf02-7177d55c8a26");
    EXPECT_THAT(this->m_sut.pop(), Eq(cxx::nullopt));
}

TYPED_TEST(TriggerQueue_test, PopOneElement)
{
    ::testing::Test::RecordProperty("TEST_ID", "49187773-f92d-4b11-9bbc-8e0ebf63a22a");
    this->m_sut.push(123U);
    auto result = this->m_sut.pop();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(123U));
}

TYPED_TEST(TriggerQueue_test, PopFullQueue)
{
    ::testing::Test::RecordProperty("TEST_ID", "c7fd6ec6-c8fc-4143-aee6-653797265531");
    constexpr uint64_t OFFSET = 231;
    this->fillQueue(OFFSET);

    for (uint64_t i = 0U; i < this->m_sut.capacity(); ++i)
    {
        auto result = this->m_sut.pop();
        ASSERT_THAT(result.has_value(), Eq(true));
        EXPECT_THAT(*result, Eq(i + OFFSET));
    }
}

TYPED_TEST(TriggerQueue_test, PushBlocksUntilPopWhenFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "519eaae3-b2da-4a7b-86a7-3bdf7523dc03");
    constexpr int64_t TIMEOUT_IN_MS = 100;
    this->fillQueue();

    std::atomic<uint64_t> counter{0U};

    std::thread t([&] {
        this->m_sut.push(0U);
        ++counter;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(TIMEOUT_IN_MS));
    EXPECT_THAT(counter.load(), Eq(0));

    auto result = this->m_sut.pop();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(0U));
    std::this_thread::sleep_for(std::chrono::milliseconds(TIMEOUT_IN_MS));
    EXPECT_THAT(counter.load(), Eq(1));
    t.join();
}

TYPED_TEST(TriggerQueue_test, PushBlocksUntilDestroyWasCalled)
{
    ::testing::Test::RecordProperty("TEST_ID", "aaf70037-dd47-4a01-bc41-f948248bf05a");
    constexpr int64_t TIMEOUT_IN_MS = 100;
    this->fillQueue();

    std::atomic<uint64_t> counter{0U};

    std::thread t([&] {
        this->m_sut.push(1U);
        this->m_sut.push(2U);
        this->m_sut.push(3U);
        this->m_sut.push(4U);
        ++counter;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(TIMEOUT_IN_MS));
    EXPECT_THAT(counter.load(), Eq(0));

    this->m_sut.destroy();
    std::this_thread::sleep_for(std::chrono::milliseconds(TIMEOUT_IN_MS));
    EXPECT_THAT(counter.load(), Eq(1));
    t.join();
}

TYPED_TEST(TriggerQueue_test, AfterDestroyPushAddsNoElements)
{
    ::testing::Test::RecordProperty("TEST_ID", "79ea6a9c-c53a-49d0-8618-53b37718e8c8");
    this->m_sut.destroy();
    this->m_sut.push(123U);

    EXPECT_THAT(this->m_sut.size(), Eq(0U));
}

} // namespace
