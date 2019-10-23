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

#include "iceoryx_utils/communication_channel/protocol/fifo_protocol.hpp"
#include "iceoryx_utils/communication_channel/unidirectional_communication_channel.hpp"

#include <atomic>
#include <gmock/gmock.h>
#include <thread>

using namespace testing;
using namespace iox;
using namespace iox::units;

template <typename UnidirectionalCommunicationChannel_t>
class UnidirectionalCommunicationChannel_Test : public Test
{
  public:
    UnidirectionalCommunicationChannel_Test()
        : sut()
        , transmitter(sut.getTransmitter())
        , receiver(sut.getReceiver())
    {
    }

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    UnidirectionalCommunicationChannel_t sut;
    typename UnidirectionalCommunicationChannel_t::Transmitter_t* transmitter;
    typename UnidirectionalCommunicationChannel_t::Receiver_t* receiver;
};

template <typename DataType>
using FiFoTestProtocol = FiFoProtocol<DataType, 100>;

using Implementations = Types<UnidirectionalCommunicationChannel<int, FiFoTestProtocol>>;
TYPED_TEST_CASE(UnidirectionalCommunicationChannel_Test, Implementations);

TYPED_TEST(UnidirectionalCommunicationChannel_Test, SendAndTryReceive)
{
    ASSERT_THAT(this->transmitter->Send(313), Eq(true));
    auto result = this->receiver->TryReceive();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(313));
}

TYPED_TEST(UnidirectionalCommunicationChannel_Test, TryReceiveWithoutSend)
{
    auto result = this->receiver->TryReceive();
    ASSERT_THAT(result.has_value(), Eq(false));
}

TYPED_TEST(UnidirectionalCommunicationChannel_Test, MultipleSendAndTryReceive)
{
    int limit = 12;
    for (int i = 0; i < limit; ++i)
        ASSERT_THAT(this->transmitter->Send(i), Eq(true));

    for (int i = 0; i < limit; ++i)
    {
        auto result = this->receiver->TryReceive();
        ASSERT_THAT(result.has_value(), Eq(true));
        EXPECT_THAT(*result, Eq(i));
    }
}

TYPED_TEST(UnidirectionalCommunicationChannel_Test, OneSendTwoTryReceive)
{
    ASSERT_THAT(this->transmitter->Send(8001), Eq(true));

    auto result = this->receiver->TryReceive();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(8001));

    auto result2 = this->receiver->TryReceive();
    ASSERT_THAT(result2.has_value(), Eq(false));
}

TYPED_TEST(UnidirectionalCommunicationChannel_Test, SendAndBlockingReceive)
{
    ASSERT_THAT(this->transmitter->Send(6313), Eq(true));
    auto result = this->receiver->BlockingReceive();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(6313));
}

TYPED_TEST(UnidirectionalCommunicationChannel_Test, MultipleSendAndBlockingReceive)
{
    int limit = 12;
    for (int i = 0; i < limit; ++i)
        ASSERT_THAT(this->transmitter->Send(i * 5), Eq(true));

    for (int i = 0; i < limit; ++i)
    {
        auto result = this->receiver->BlockingReceive();
        ASSERT_THAT(result.has_value(), Eq(true));
        EXPECT_THAT(*result, Eq(i * 5));
    }
}

TYPED_TEST(UnidirectionalCommunicationChannel_Test, SendAndTimedReceive)
{
    ASSERT_THAT(this->transmitter->Send(313), Eq(true));
    auto result = this->receiver->timedReceive(10_ms);
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(313));
}

TYPED_TEST(UnidirectionalCommunicationChannel_Test, TimedReceiveWithoutSend)
{
    auto result = this->receiver->timedReceive(10_ms);
    ASSERT_THAT(result.has_value(), Eq(false));
}

TYPED_TEST(UnidirectionalCommunicationChannel_Test, MultipleSendAndTimedReceive)
{
    int limit = 12;
    for (int i = 0; i < limit; ++i)
        ASSERT_THAT(this->transmitter->Send(i), Eq(true));

    for (int i = 0; i < limit; ++i)
    {
        auto result = this->receiver->timedReceive(10_ms);
        ASSERT_THAT(result.has_value(), Eq(true));
        EXPECT_THAT(*result, Eq(i));
    }
}

TYPED_TEST(UnidirectionalCommunicationChannel_Test, OneSendTwoTimedReceive)
{
    ASSERT_THAT(this->transmitter->Send(8001), Eq(true));

    auto result = this->receiver->timedReceive(10_ms);
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(8001));

    auto result2 = this->receiver->timedReceive(10_ms);
    ASSERT_THAT(result2.has_value(), Eq(false));
}

TYPED_TEST(UnidirectionalCommunicationChannel_Test, BlockingReceiveIsBlockingTillDataIsSend)
{
    std::atomic_bool hasReceivedData{false};
    std::thread t1([&] {
        this->receiver->BlockingReceive();
        hasReceivedData.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_THAT(hasReceivedData.load(), Eq(false));
    ASSERT_THAT(this->transmitter->Send(8001), Eq(true));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_THAT(hasReceivedData.load(), Eq(true));

    t1.join();
}

TYPED_TEST(UnidirectionalCommunicationChannel_Test, TimedReceiveIsBlockingTillDataIsSend)
{
    std::atomic_bool hasReceivedData{false};
    std::thread t1([&] {
        this->receiver->timedReceive(1000_s);
        hasReceivedData.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_THAT(hasReceivedData.load(), Eq(false));
    ASSERT_THAT(this->transmitter->Send(8001), Eq(true));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_THAT(hasReceivedData.load(), Eq(true));

    t1.join();
}

TYPED_TEST(UnidirectionalCommunicationChannel_Test, TimedReceiveHasTimeout)
{
    std::atomic_bool hasTimeout{false};
    std::thread t1([&] {
        this->receiver->timedReceive(100_ms);
        hasTimeout.store(true);
    });

    ASSERT_THAT(hasTimeout.load(), Eq(false));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    ASSERT_THAT(hasTimeout.load(), Eq(true));

    t1.join();
}

namespace UnidirectionalCommunicationChannelTestInternals
{
static std::string ctorTest;
template <typename T>
class TestProtocol
{
  public:
    TestProtocol(const std::string& testName)
    {
        ctorTest.append(testName);
    }
    bool Send(const T& f_message)
    {
        return true;
    }
    cxx::optional<T> TryReceive()
    {
        return cxx::nullopt_t();
    }
    cxx::optional<T> BlockingReceive()
    {
        return cxx::nullopt_t();
    }
    cxx::optional<T> timedReceive(const units::Duration&)
    {
        return cxx::nullopt_t();
    }
};
} // namespace UnidirectionalCommunicationChannelTestInternals

TEST(UnidirectionalCommunicationChannel_BaseTest, ConstructorArgumentForTransportLayer)
{
    UnidirectionalCommunicationChannel<int, UnidirectionalCommunicationChannelTestInternals::TestProtocol> sut(
        "ctorFuu");
    EXPECT_THAT(UnidirectionalCommunicationChannelTestInternals::ctorTest, Eq("ctorFuu"));
}
