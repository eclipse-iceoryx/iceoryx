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

#include <atomic>
#include <gmock/gmock.h>
#include <thread>

using namespace testing;
using namespace iox;
using namespace iox::units;

/// @brief Every communication channel protocol MUST pass this test. To add a
///         new protocol procede as following:
///         1. create a new factory method for your protocol
///             template<> MyNewProtocol<int> * CreateProtocol() ...
///         2. Add the new protocol into the Implementations alias like
///             using Implementations = Types<...., MyNewProtocol<int> >;
///        If all tests are passing you are ready to go!

template <typename T>
T* CreateProtocol();

template <typename DataType>
using FiFoTestProtocol = FiFoProtocol<DataType, 100>;


template <>
FiFoTestProtocol<int>* CreateProtocol()
{
    return new FiFoTestProtocol<int>();
}

template <typename CommunicationChannelGenericProtocol_t>
class CommunicationChannelGenericProtocol_Test : public Test
{
  public:
    CommunicationChannelGenericProtocol_Test()
        : sut(CreateProtocol<CommunicationChannelGenericProtocol_t>())
    {
    }

    ~CommunicationChannelGenericProtocol_Test()
    {
        delete sut;
    }

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    CommunicationChannelGenericProtocol_t* sut;
};

using Implementations = Types<FiFoTestProtocol<int>>;
TYPED_TEST_CASE(CommunicationChannelGenericProtocol_Test, Implementations);

TYPED_TEST(CommunicationChannelGenericProtocol_Test, SendAndTryReceive)
{
    ASSERT_THAT(this->sut->Send(313), Eq(true));
    auto result = this->sut->TryReceive();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(313));
}

TYPED_TEST(CommunicationChannelGenericProtocol_Test, TryReceiveWithoutSend)
{
    auto result = this->sut->TryReceive();
    ASSERT_THAT(result.has_value(), Eq(false));
}

TYPED_TEST(CommunicationChannelGenericProtocol_Test, MultipleSendAndTryReceive)
{
    int limit = 12;
    for (int i = 0; i < limit; ++i)
        ASSERT_THAT(this->sut->Send(i), Eq(true));

    for (int i = 0; i < limit; ++i)
    {
        auto result = this->sut->TryReceive();
        ASSERT_THAT(result.has_value(), Eq(true));
        EXPECT_THAT(*result, Eq(i));
    }
}

TYPED_TEST(CommunicationChannelGenericProtocol_Test, OneSendTwoTryReceive)
{
    ASSERT_THAT(this->sut->Send(8001), Eq(true));

    auto result = this->sut->TryReceive();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(8001));

    auto result2 = this->sut->TryReceive();
    ASSERT_THAT(result2.has_value(), Eq(false));
}

TYPED_TEST(CommunicationChannelGenericProtocol_Test, SendAndBlockingReceive)
{
    ASSERT_THAT(this->sut->Send(6313), Eq(true));
    auto result = this->sut->BlockingReceive();
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(6313));
}

TYPED_TEST(CommunicationChannelGenericProtocol_Test, MultipleSendAndBlockingReceive)
{
    int limit = 12;
    for (int i = 0; i < limit; ++i)
        ASSERT_THAT(this->sut->Send(i * 5), Eq(true));

    for (int i = 0; i < limit; ++i)
    {
        auto result = this->sut->BlockingReceive();
        ASSERT_THAT(result.has_value(), Eq(true));
        EXPECT_THAT(*result, Eq(i * 5));
    }
}

TYPED_TEST(CommunicationChannelGenericProtocol_Test, SendAndTimedReceive)
{
    ASSERT_THAT(this->sut->Send(313), Eq(true));
    auto result = this->sut->timedReceive(10_ms);
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(313));
}

TYPED_TEST(CommunicationChannelGenericProtocol_Test, TimedReceiveWithoutSend)
{
    auto result = this->sut->timedReceive(10_ms);
    ASSERT_THAT(result.has_value(), Eq(false));
}

TYPED_TEST(CommunicationChannelGenericProtocol_Test, MultipleSendAndTimedReceive)
{
    int limit = 12;
    for (int i = 0; i < limit; ++i)
        ASSERT_THAT(this->sut->Send(i), Eq(true));

    for (int i = 0; i < limit; ++i)
    {
        auto result = this->sut->timedReceive(10_ms);
        ASSERT_THAT(result.has_value(), Eq(true));
        EXPECT_THAT(*result, Eq(i));
    }
}

TYPED_TEST(CommunicationChannelGenericProtocol_Test, OneSendTwoTimedReceive)
{
    ASSERT_THAT(this->sut->Send(8001), Eq(true));

    auto result = this->sut->timedReceive(10_ms);
    ASSERT_THAT(result.has_value(), Eq(true));
    EXPECT_THAT(*result, Eq(8001));

    auto result2 = this->sut->timedReceive(10_ms);
    ASSERT_THAT(result2.has_value(), Eq(false));
}

TYPED_TEST(CommunicationChannelGenericProtocol_Test, BlockingReceiveIsBlockingTillDataIsSend)
{
    std::atomic_bool hasReceivedData{false};
    std::thread t1([&] {
        this->sut->BlockingReceive();
        hasReceivedData.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_THAT(hasReceivedData.load(), Eq(false));
    ASSERT_THAT(this->sut->Send(8001), Eq(true));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_THAT(hasReceivedData.load(), Eq(true));

    t1.join();
}

TYPED_TEST(CommunicationChannelGenericProtocol_Test, TimedReceiveIsBlockingTillDataIsSend)
{
    std::atomic_bool hasReceivedData{false};
    std::thread t1([&] {
        this->sut->timedReceive(1000_s);
        hasReceivedData.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_THAT(hasReceivedData.load(), Eq(false));
    ASSERT_THAT(this->sut->Send(8001), Eq(true));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_THAT(hasReceivedData.load(), Eq(true));

    t1.join();
}

TYPED_TEST(CommunicationChannelGenericProtocol_Test, TimedReceiveHasTimeout)
{
    std::atomic_bool hasTimeout{false};
    std::thread t1([&] {
        this->sut->timedReceive(100_ms);
        hasTimeout.store(true);
    });

    ASSERT_THAT(hasTimeout.load(), Eq(false));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    ASSERT_THAT(hasTimeout.load(), Eq(true));

    t1.join();
}
