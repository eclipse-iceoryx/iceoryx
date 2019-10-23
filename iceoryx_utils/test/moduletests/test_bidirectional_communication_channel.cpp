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

#include "iceoryx_utils/communication_channel/bidirectional_communication_channel.hpp"
#include "iceoryx_utils/communication_channel/protocol/fifo_protocol.hpp"

#include <atomic>
#include <gmock/gmock.h>
#include <thread>

using namespace testing;
using namespace iox;
using namespace iox::units;

template <typename BidirectionalCommunicationChannel_t>
class BidirectionalCommunicationChannel_Test : public Test
{
  public:
    BidirectionalCommunicationChannel_Test()
        : sut()
        , transceiverA2B(sut.getFirstTransceiver())
        , transceiverB2A(sut.getSecondTransceiver())
    {
    }

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    typename BidirectionalCommunicationChannel_t::TransportLayer_t transportLayerA2B;
    typename BidirectionalCommunicationChannel_t::TransportLayer_t transportLayerB2A;
    BidirectionalCommunicationChannel_t sut;
    typename BidirectionalCommunicationChannel_t::Transceiver_t* transceiverA2B;
    typename BidirectionalCommunicationChannel_t::Transceiver_t* transceiverB2A;
};

template <typename DataType>
using FiFoTestProtocol = FiFoProtocol<DataType, 100>;

using Implementations = Types<BidirectionalCommunicationChannel<int, FiFoTestProtocol>>;
TYPED_TEST_CASE(BidirectionalCommunicationChannel_Test, Implementations);

TYPED_TEST(BidirectionalCommunicationChannel_Test, SendAndTryReceiveA2B)
{
    ASSERT_THAT(this->transceiverA2B->Send(313), Eq(true));
    auto result = this->transceiverB2A->TryReceive();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(313));
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, SendAndTryReceiveB2A)
{
    ASSERT_THAT(this->transceiverB2A->Send(5313), Eq(true));
    auto result = this->transceiverA2B->TryReceive();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(5313));
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, TryReceiveWithoutSendA2B)
{
    auto result = this->transceiverA2B->TryReceive();
    ASSERT_THAT(result.has_value(), Eq(false));
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, TryReceiveWithoutSendB2A)
{
    auto result = this->transceiverB2A->TryReceive();
    ASSERT_THAT(result.has_value(), Eq(false));
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, MultiSendAndTryReceiveA2B)
{
    int limit = 14;
    for (int i = 0; i < limit; ++i)
        ASSERT_THAT(this->transceiverA2B->Send(i * 87), Eq(true));

    for (int i = 0; i < limit; ++i)
    {
        auto result = this->transceiverB2A->TryReceive();
        ASSERT_THAT(result.has_value(), Eq(true));
        EXPECT_THAT(*result, Eq(i * 87));
    }
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, MultiSendAndTryReceiveB2A)
{
    int limit = 15;
    for (int i = 0; i < limit; ++i)
        ASSERT_THAT(this->transceiverB2A->Send(i * 71), Eq(true));

    for (int i = 0; i < limit; ++i)
    {
        auto result = this->transceiverA2B->TryReceive();
        ASSERT_THAT(result.has_value(), Eq(true));
        EXPECT_THAT(*result, Eq(i * 71));
    }
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, OneSendAndTwoTryReceiveA2B)
{
    ASSERT_THAT(this->transceiverA2B->Send(781), Eq(true));

    auto result = this->transceiverB2A->TryReceive();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(781));

    auto result2 = this->transceiverB2A->TryReceive();
    ASSERT_THAT(result2.has_value(), Eq(false));
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, OneSendAndTwoTryReceiveB2A)
{
    ASSERT_THAT(this->transceiverB2A->Send(983), Eq(true));

    auto result = this->transceiverA2B->TryReceive();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(983));

    auto result2 = this->transceiverA2B->TryReceive();
    ASSERT_THAT(result2.has_value(), Eq(false));
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, SendAndTimedReceiveA2B)
{
    ASSERT_THAT(this->transceiverA2B->Send(313), Eq(true));
    auto result = this->transceiverB2A->timedReceive(1_ms);
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(313));
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, SendAndTimedReceiveB2A)
{
    ASSERT_THAT(this->transceiverB2A->Send(5313), Eq(true));
    auto result = this->transceiverA2B->timedReceive(1_ms);
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(5313));
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, TimedReceiveWithoutSendA2B)
{
    auto result = this->transceiverA2B->timedReceive(1_ms);
    ASSERT_THAT(result.has_value(), Eq(false));
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, TimedReceiveWithoutSendB2A)
{
    auto result = this->transceiverB2A->timedReceive(1_ms);
    ASSERT_THAT(result.has_value(), Eq(false));
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, MultiSendAndTimedReceiveA2B)
{
    int limit = 14;
    for (int i = 0; i < limit; ++i)
        ASSERT_THAT(this->transceiverA2B->Send(i * 87), Eq(true));

    for (int i = 0; i < limit; ++i)
    {
        auto result = this->transceiverB2A->timedReceive(1_ms);
        ASSERT_THAT(result.has_value(), Eq(true));
        EXPECT_THAT(*result, Eq(i * 87));
    }
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, MultiSendAndTimedReceiveB2A)
{
    int limit = 15;
    for (int i = 0; i < limit; ++i)
        ASSERT_THAT(this->transceiverB2A->Send(i * 71), Eq(true));

    for (int i = 0; i < limit; ++i)
    {
        auto result = this->transceiverA2B->timedReceive(1_ms);
        ASSERT_THAT(result.has_value(), Eq(true));
        EXPECT_THAT(*result, Eq(i * 71));
    }
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, OneSendAndTwoTimedReceiveA2B)
{
    ASSERT_THAT(this->transceiverA2B->Send(781), Eq(true));

    auto result = this->transceiverB2A->timedReceive(1_ms);
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(781));

    auto result2 = this->transceiverB2A->timedReceive(1_ms);
    ASSERT_THAT(result2.has_value(), Eq(false));
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, OneSendAndTwoTimedReceiveB2A)
{
    ASSERT_THAT(this->transceiverB2A->Send(983), Eq(true));

    auto result = this->transceiverA2B->timedReceive(1_ms);
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(983));

    auto result2 = this->transceiverA2B->timedReceive(1_ms);
    ASSERT_THAT(result2.has_value(), Eq(false));
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, SendAndBlockingReceiveA2B)
{
    ASSERT_THAT(this->transceiverA2B->Send(313), Eq(true));
    auto result = this->transceiverB2A->BlockingReceive();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(313));
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, SendAndBlockingReceiveB2A)
{
    ASSERT_THAT(this->transceiverB2A->Send(5313), Eq(true));
    auto result = this->transceiverA2B->BlockingReceive();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(5313));
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, MultiSendAndBlockingReceiveA2B)
{
    int limit = 14;
    for (int i = 0; i < limit; ++i)
        ASSERT_THAT(this->transceiverA2B->Send(i * 87), Eq(true));

    for (int i = 0; i < limit; ++i)
    {
        auto result = this->transceiverB2A->BlockingReceive();
        ASSERT_THAT(result.has_value(), Eq(true));
        EXPECT_THAT(*result, Eq(i * 87));
    }
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, MultiSendAndBlockingReceiveB2A)
{
    int limit = 15;
    for (int i = 0; i < limit; ++i)
        ASSERT_THAT(this->transceiverB2A->Send(i * 71), Eq(true));

    for (int i = 0; i < limit; ++i)
    {
        auto result = this->transceiverA2B->BlockingReceive();
        ASSERT_THAT(result.has_value(), Eq(true));
        EXPECT_THAT(*result, Eq(i * 71));
    }
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, BlockingReceiveIsBlockingTillDataIsSendA2B)
{
    std::atomic_bool hasReceivedData{false};
    std::thread t1([&] {
        this->transceiverB2A->BlockingReceive();
        hasReceivedData.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_THAT(hasReceivedData.load(), Eq(false));
    ASSERT_THAT(this->transceiverA2B->Send(8001), Eq(true));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_THAT(hasReceivedData.load(), Eq(true));

    t1.join();
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, BlockingReceiveIsBlockingTillDataIsSendB2A)
{
    std::atomic_bool hasReceivedData{false};
    std::thread t1([&] {
        this->transceiverA2B->BlockingReceive();
        hasReceivedData.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_THAT(hasReceivedData.load(), Eq(false));
    ASSERT_THAT(this->transceiverB2A->Send(8001), Eq(true));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_THAT(hasReceivedData.load(), Eq(true));

    t1.join();
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, TimedReceiveIsBlockingTillDataIsSendA2B)
{
    std::atomic_bool hasReceivedData{false};
    std::thread t1([&] {
        this->transceiverB2A->BlockingReceive();
        hasReceivedData.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_THAT(hasReceivedData.load(), Eq(false));
    ASSERT_THAT(this->transceiverA2B->Send(8001), Eq(true));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_THAT(hasReceivedData.load(), Eq(true));

    t1.join();
}

TYPED_TEST(BidirectionalCommunicationChannel_Test, TimedReceiveIsBlockingTillDataIsSendB2A)
{
    std::atomic_bool hasReceivedData{false};
    std::thread t1([&] {
        this->transceiverA2B->BlockingReceive();
        hasReceivedData.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_THAT(hasReceivedData.load(), Eq(false));
    ASSERT_THAT(this->transceiverB2A->Send(8001), Eq(true));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ASSERT_THAT(hasReceivedData.load(), Eq(true));

    t1.join();
}

namespace BidirectionalCommunicationChannelTestInternals
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
} // namespace BidirectionalCommunicationChannelTestInternals

TEST(BidirectionalCommunicationChannel_BaseTest, ConstructorArgumentsForTransportLayer)
{
    BidirectionalCommunicationChannel<int, BidirectionalCommunicationChannelTestInternals::TestProtocol> sut("ctor1",
                                                                                                             "ctor2");
    EXPECT_THAT(BidirectionalCommunicationChannelTestInternals::ctorTest, Eq("ctor1ctor2"));
}
