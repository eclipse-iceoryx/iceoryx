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
#include "iceoryx_utils/internal/concurrent/trigger_queue.hpp"

using namespace ::testing;

template <typename QueueType>
class TriggerQueue_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    iox::cxx::optional<QueueType> m_sut = QueueType::CreateTriggerQueue();
};

namespace helper
{
template <typename T, uint64_t CAPACITY>
constexpr uint64_t GetCapacity(iox::concurrent::TriggerQueue<T, CAPACITY>&)
{
    return CAPACITY;
}
} // namespace helper

using TriggerQueueTestSubjects = Types<iox::concurrent::TriggerQueue<uint64_t, 1>,
                                       iox::concurrent::TriggerQueue<uint64_t, 10>,
                                       iox::concurrent::TriggerQueue<uint64_t, 100>>;
TYPED_TEST_CASE(TriggerQueue_test, TriggerQueueTestSubjects);

TYPED_TEST(TriggerQueue_test, EmptyWhenEmpty)
{
    EXPECT_THAT(this->m_sut->empty(), Eq(true));
}

TYPED_TEST(TriggerQueue_test, EmptyWhenNotEmpty)
{
    EXPECT_THAT(this->m_sut->push(5), Eq(true));
    EXPECT_THAT(this->m_sut->empty(), Eq(false));
}

TYPED_TEST(TriggerQueue_test, SinglePush)
{
    EXPECT_THAT(this->m_sut->push(512u), Eq(true));
    uint64_t value;
    EXPECT_THAT(this->m_sut->blocking_pop(value), Eq(true));
    EXPECT_THAT(value, Eq(512u));
}

TYPED_TEST(TriggerQueue_test, PushTillItsFull)
{
    for (uint64_t i = 0; i < this->m_sut->capacity(); ++i)
    {
        ASSERT_THAT(this->m_sut->push(i * 3), Eq(true));
    }

    for (uint64_t i = 0; i < this->m_sut->capacity(); ++i)
    {
        uint64_t value;
        EXPECT_THAT(this->m_sut->blocking_pop(value), Eq(true));
        EXPECT_THAT(value, Eq(value));
    }
}

TYPED_TEST(TriggerQueue_test, PushWhenItIsAlreadyFull)
{
    for (uint64_t i = 0; i < this->m_sut->capacity(); ++i)
    {
        ASSERT_THAT(this->m_sut->push(i * 3), Eq(true));
    }

    EXPECT_THAT(this->m_sut->push(551), Eq(false));
}

TYPED_TEST(TriggerQueue_test, TryPopOnEmpty)
{
    uint64_t value;
    EXPECT_THAT(this->m_sut->try_pop(value), Eq(false));
}

TYPED_TEST(TriggerQueue_test, TryPopSingleElement)
{
    uint64_t value;
    this->m_sut->push(73u);
    EXPECT_THAT(this->m_sut->try_pop(value), Eq(true));
    EXPECT_THAT(value, Eq(73u));
}

TYPED_TEST(TriggerQueue_test, TryPopFull)
{
    for (uint64_t i = 0; i < this->m_sut->capacity(); ++i)
    {
        this->m_sut->push(i * 3u);
    }

    for (uint64_t i = 0; i < this->m_sut->capacity(); ++i)
    {
        uint64_t value;
        EXPECT_THAT(this->m_sut->try_pop(value), Eq(true));
        EXPECT_THAT(value, Eq(i * 3u));
    }
}

TYPED_TEST(TriggerQueue_test, Size)
{
    for (uint64_t i = 0; i < this->m_sut->capacity(); ++i)
    {
        this->m_sut->push(i * 3);
        EXPECT_THAT(this->m_sut->size(), Eq(i + 1));
    }
}

TYPED_TEST(TriggerQueue_test, Capacity)
{
    EXPECT_THAT(this->m_sut->capacity(), helper::GetCapacity(this->m_sut.value()));
}
