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

#include "iceoryx_utils/internal/communication_channel/receiver.hpp"
#include "iceoryx_utils/communication_channel/protocol/fifo_protocol.hpp"

#include <atomic>
#include <gmock/gmock.h>
#include <thread>

using namespace testing;
using namespace iox;
using namespace iox::units;


template <typename Receiver_t>
class CommunicationChannelReceiver_Test : public Test
{
  public:
    CommunicationChannelReceiver_Test()
        : sut(&transportLayer)
    {
    }

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    typename Receiver_t::TransportLayer_t transportLayer;
    Receiver_t sut;
};

template <typename DataType>
using FiFoTestProtocol = FiFoProtocol<DataType, 100>;

using Implementations = Types<Receiver<int, FiFoTestProtocol>>;
TYPED_TEST_CASE(CommunicationChannelReceiver_Test, Implementations);

TYPED_TEST(CommunicationChannelReceiver_Test, TryReceiveWithoutSample)
{
    auto result = this->sut.TryReceive();
    EXPECT_THAT(result.has_value(), Eq(false));
}

TYPED_TEST(CommunicationChannelReceiver_Test, TryReceiveWithSample)
{
    this->transportLayer.Send(123);
    auto result = this->sut.TryReceive();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(123));
}

TYPED_TEST(CommunicationChannelReceiver_Test, TryReceiveMultipleSampleCorrectOrdering)
{
    int limit = 10;
    for (int i = 0; i < limit; ++i)
        this->transportLayer.Send(i);

    for (int i = 0; i < limit; ++i)
    {
        auto result = this->sut.TryReceive();
        ASSERT_THAT(result.has_value(), Eq(true));
        EXPECT_THAT(*result, Eq(i));
    }
}

TYPED_TEST(CommunicationChannelReceiver_Test, BlockingReceiveWithSample)
{
    this->transportLayer.Send(912);
    auto result = this->sut.BlockingReceive();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(912));
}

TYPED_TEST(CommunicationChannelReceiver_Test, BlockingReceiveMultipleSampleCorrectOrdering)
{
    int limit = 10;
    for (int i = 0; i < limit; ++i)
        this->transportLayer.Send(i);

    for (int i = 0; i < limit; ++i)
    {
        auto result = this->sut.BlockingReceive();
        ASSERT_THAT(result.has_value(), Eq(true));
        EXPECT_THAT(*result, Eq(i));
    }
}

TYPED_TEST(CommunicationChannelReceiver_Test, TimedReceiveWithoutSample)
{
    auto result = this->sut.timedReceive(1_ms);
    EXPECT_THAT(result.has_value(), Eq(false));
}

TYPED_TEST(CommunicationChannelReceiver_Test, TimedReceiveWithSample)
{
    this->transportLayer.Send(123);
    auto result = this->sut.timedReceive(1_ms);
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(123));
}

TYPED_TEST(CommunicationChannelReceiver_Test, TimedReceiveMultipleSampleCorrectOrdering)
{
    int limit = 10;
    for (int i = 0; i < limit; ++i)
        this->transportLayer.Send(i);

    for (int i = 0; i < limit; ++i)
    {
        auto result = this->sut.timedReceive(1_ms);
        ASSERT_THAT(result.has_value(), Eq(true));
        EXPECT_THAT(*result, Eq(i));
    }
}

TYPED_TEST(CommunicationChannelReceiver_Test, BlockingReceiveIsBlocking)
{
    std::atomic_bool hasReceivedData{false};

    std::thread t([&] {
        auto result = this->sut.BlockingReceive();
        ASSERT_THAT(result.has_value(), Eq(true));
        EXPECT_THAT(*result, Eq(9192));
        hasReceivedData.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_THAT(hasReceivedData.load(), Eq(false));

    this->transportLayer.Send(9192);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_THAT(hasReceivedData.load(), Eq(true));
    t.join();
}

TYPED_TEST(CommunicationChannelReceiver_Test, TimedReceiveIsBlocking)
{
    std::atomic_bool hasReceivedData{false};

    std::thread t([&] {
        auto result = this->sut.timedReceive(1_d);
        ASSERT_THAT(result.has_value(), Eq(true));
        EXPECT_THAT(*result, Eq(9112));
        hasReceivedData.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_THAT(hasReceivedData.load(), Eq(false));

    this->transportLayer.Send(9112);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_THAT(hasReceivedData.load(), Eq(true));
    t.join();
}

TYPED_TEST(CommunicationChannelReceiver_Test, TimedReceiveHasTimeout)
{
    std::atomic_bool hasTimeoutOccurred{false};

    std::thread t([&] {
        auto result = this->sut.timedReceive(100_ms);
        ASSERT_THAT(result.has_value(), Eq(false));
        hasTimeoutOccurred.store(true);
    });

    ASSERT_THAT(hasTimeoutOccurred.load(), Eq(false));

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    ASSERT_THAT(hasTimeoutOccurred.load(), Eq(true));
    t.join();
}
