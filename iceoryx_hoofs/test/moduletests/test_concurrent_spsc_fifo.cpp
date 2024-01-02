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

#include "iox/detail/spsc_fifo.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>

namespace
{
using namespace testing;
using namespace iox::concurrent;

constexpr uint64_t FIFO_CAPACITY = 10;

class SpscFifo_Test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    SpscFifo<uint64_t, FIFO_CAPACITY> sut;
};

TEST_F(SpscFifo_Test, SinglePopSinglePush)
{
    ::testing::Test::RecordProperty("TEST_ID", "57059a17-ec89-42e3-a07c-4a53d0cdcb1d");
    EXPECT_THAT(sut.push(25), Eq(true));
    auto result = sut.pop();
    EXPECT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(result.value(), Eq(25U));
}

TEST_F(SpscFifo_Test, PopFailsWhenEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "0063d54a-e3cb-43f8-ac32-fd0ad94ba7e1");
    auto result = sut.pop();
    EXPECT_THAT(result.has_value(), Eq(false));
}

TEST_F(SpscFifo_Test, PushFailsWhenFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "8d492e83-c0c3-47bd-b745-9f56e20199e9");
    for (uint64_t k = 0; k < FIFO_CAPACITY; ++k)
    {
        EXPECT_THAT(sut.push(k), Eq(true));
    }
    EXPECT_THAT(sut.push(123), Eq(false));
}

TEST_F(SpscFifo_Test, IsEmptyWhenPopReturnsNullopt)
{
    ::testing::Test::RecordProperty("TEST_ID", "81a538c8-f366-4625-8aad-d83ab1d5ecf4");
    for (uint64_t k = 0; k < FIFO_CAPACITY; ++k)
    {
        EXPECT_THAT(sut.push(k), Eq(true));
    }
    for (uint64_t k = 0; k < FIFO_CAPACITY; ++k)
    {
        EXPECT_THAT(sut.pop().has_value(), Eq(true));
    }

    EXPECT_THAT(sut.pop().has_value(), Eq(false));
    EXPECT_THAT(sut.empty(), Eq(true));
}

TEST_F(SpscFifo_Test, OverflowTestWithPushPopAlternation)
{
    ::testing::Test::RecordProperty("TEST_ID", "6ea65156-ca3f-42fc-b199-1119696023c1");
    for (uint64_t k = 0; k < 100 * FIFO_CAPACITY; ++k)
    {
        EXPECT_THAT(sut.push(k), Eq(true));
        auto result = sut.pop();
        EXPECT_THAT(result.has_value(), Eq(true));
        EXPECT_THAT(result.value(), Eq(k));
    }
}

TEST_F(SpscFifo_Test, OverflowFromFullToEmptyRepetition)
{
    ::testing::Test::RecordProperty("TEST_ID", "33a8c03f-5538-46b4-846e-9dec4badab0b");
    uint64_t m = 0;

    for (uint64_t repetition = 0; repetition < 10; ++repetition)
    {
        for (uint64_t k = 0; k < FIFO_CAPACITY; ++k, ++m)
        {
            EXPECT_THAT(sut.push(m), Eq(true));
        }

        for (uint64_t k = 0; k < FIFO_CAPACITY; ++k)
        {
            auto result = sut.pop();
            EXPECT_THAT(result.has_value(), Eq(true));
            EXPECT_THAT(result.value(), Eq(m - FIFO_CAPACITY + k));
        }
        EXPECT_THAT(sut.empty(), Eq(true));
    }
}
} // namespace
