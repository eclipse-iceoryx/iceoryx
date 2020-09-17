// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/experimental/popo/base_subscriber.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/unique_ptr.hpp"

#include "mocks/subscriber_mock.hpp"
#include "test.hpp"

using namespace ::testing;
using ::testing::_;

struct DummyData{
    uint64_t val = 42;
};

template<typename T, typename port_t>
class StubbedBaseSubscriber : public iox::popo::BaseSubscriber<T, port_t>
{
public:
    StubbedBaseSubscriber(iox::capro::ServiceDescription sd) : iox::popo::BaseSubscriber<T, port_t>::BaseSubscriber(sd)
    {
    }
    uid_t uid() const noexcept
    {
        return iox::popo::BaseSubscriber<T, port_t>::uid();
    }
    iox::capro::ServiceDescription getServiceDescription() const noexcept
    {
        return iox::popo::BaseSubscriber<T, port_t>::getServiceDescription();
    }
    void subscribe(const uint64_t queueCapacity = iox::MAX_SUBSCRIBER_QUEUE_CAPACITY) noexcept
    {
        return iox::popo::BaseSubscriber<T, port_t>::subscribe(queueCapacity);
    }
    iox::SubscribeState getSubscriptionState() const noexcept
    {
        return iox::popo::BaseSubscriber<T, port_t>::getSubscriptionState();
    }
    void unsubscribe() noexcept
    {
        return iox::popo::BaseSubscriber<T, port_t>::unsubscribe();
    }
    bool hasNewSamples() const noexcept
    {
        return iox::popo::BaseSubscriber<T, port_t>::hasNewSamples();
    }
    iox::cxx::expected<iox::cxx::optional<iox::popo::Sample<T>>, iox::popo::ChunkReceiveError> receive() noexcept
    {
        return iox::popo::BaseSubscriber<T, port_t>::receive();
    }
    iox::cxx::optional<iox::cxx::unique_ptr<iox::mepoo::ChunkHeader>> receiveHeader() noexcept
    {
        return iox::popo::BaseSubscriber<T, port_t>::receiveHeader();
    }
    void clearReceiveBuffer() noexcept
    {
        return iox::popo::BaseSubscriber<T, port_t>::clearReceiveBuffer();
    }
    bool setConditionVariable(iox::popo::ConditionVariableData* const conditionVariableDataPtr) noexcept override
    {
        return iox::popo::BaseSubscriber<T, port_t>::setConditionVariable(conditionVariableDataPtr);
    }
    bool unsetConditionVariable() noexcept override
    {
        return iox::popo::BaseSubscriber<T, port_t>::unsetConditionVariable();
    }
    virtual bool hasTriggered() const noexcept override
    {
        return iox::popo::BaseSubscriber<T, port_t>::hasTriggered();
    }
    port_t& getMockedPort()
    {
        return iox::popo::BaseSubscriber<T, port_t>::m_port;
    }
    bool getUnderlyingSubscriptionRequestState()
    {
        return iox::popo::BaseSubscriber<T, port_t>::m_subscriptionRequested;
    }
};

using TestBaseSubscriber = StubbedBaseSubscriber<DummyData, MockSubscriberPortUser>;

// ========================= Base Publisher Tests ========================= //

class ExperimentalBaseSubscriberTest : public Test {

public:
    ExperimentalBaseSubscriberTest()
    {

    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

protected:
    TestBaseSubscriber sut{{"", "", ""}};
};

TEST_F(ExperimentalBaseSubscriberTest, SubscriptionRequestIsSetOnSubscribe)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), subscribe(iox::MAX_SUBSCRIBER_QUEUE_CAPACITY)).Times(1);
    // ===== Test ===== //
    sut.subscribe();
    // ===== Verify ===== //
    EXPECT_EQ(true, sut.getUnderlyingSubscriptionRequestState());
    // ===== Cleanup ===== //
}

TEST_F(ExperimentalBaseSubscriberTest, SubscribeCallForwardedToUnderlyingSubscriberPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), subscribe(iox::MAX_SUBSCRIBER_QUEUE_CAPACITY)).Times(1);
    // ===== Test ===== //
    sut.subscribe();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(ExperimentalBaseSubscriberTest, GetSubscriptionStateCallForwardedToUnderlyingSubscriberPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), getSubscriptionState).Times(1);
    // ===== Test ===== //
    sut.getSubscriptionState();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(ExperimentalBaseSubscriberTest, UnsubscribeCallForwardedToUnderlyingSubscriberPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), unsubscribe).Times(1);
    // ===== Test ===== //
    sut.unsubscribe();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(ExperimentalBaseSubscriberTest, SubscriptionRequestIsResetOnUnsubscribe)
{
    // ===== Setup ===== //
    // ===== Test ===== //
    sut.subscribe();
    sut.unsubscribe();
    // ===== Verify ===== //
    EXPECT_EQ(false, sut.getUnderlyingSubscriptionRequestState());
    // ===== Cleanup ===== //
}

TEST_F(ExperimentalBaseSubscriberTest, HasNewSamplesCallForwardedToUnderlyingSubscriberPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), hasNewChunks).Times(1);
    // ===== Test ===== //
    sut.hasNewSamples();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(ExperimentalBaseSubscriberTest, ReceiveReturnsAllocatedMemoryChunksInSamples)
{
    // ===== Setup ===== //
    auto chunk = reinterpret_cast<iox::mepoo::ChunkHeader*>(iox::cxx::alignedAlloc(32, sizeof(iox::mepoo::ChunkHeader)));
    EXPECT_CALL(sut.getMockedPort(), tryGetChunk).WillOnce(Return(ByMove(
                                                                      iox::cxx::success<iox::cxx::optional<const iox::mepoo::ChunkHeader*>>(const_cast<const iox::mepoo::ChunkHeader*>(chunk))
                                                                      )));
    // ===== Test ===== //
    auto result = sut.receive();
    // ===== Verify ===== //
    EXPECT_EQ(false, result.has_error());
    EXPECT_EQ(true, result.get_value().has_value());
    EXPECT_EQ(reinterpret_cast<DummyData*>(chunk->payload()), result.get_value().value().get()); // Checks they point to the same memory location.
    // ===== Cleanup ===== //
}

TEST_F(ExperimentalBaseSubscriberTest, ReceiveForwardsErrorsFromUnderlyingPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), tryGetChunk).WillOnce(Return(ByMove(
                                                                      iox::cxx::error<iox::popo::ChunkReceiveError>(iox::popo::ChunkReceiveError::TOO_MANY_CHUNKS_HELD_IN_PARALLEL)
                                                                      )));
    // ===== Test ===== //
    auto result = sut.receive();
    // ===== Verify ===== //
    EXPECT_EQ(true, result.has_error());
    // ===== Cleanup ===== //
}

TEST_F(ExperimentalBaseSubscriberTest, ReceiveReturnsEmptyOptionalIfUnderlyingPortReturnsEmptyOptional)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), tryGetChunk).WillOnce(Return(ByMove(
                                                                      iox::cxx::success<iox::cxx::optional<const iox::mepoo::ChunkHeader*>>(iox::cxx::nullopt)
                                                                      )));
    // ===== Test ===== //
    auto result = sut.receive();
    // ===== Verify ===== //
    EXPECT_EQ(false, result.has_error());
    EXPECT_EQ(false, result.get_value().has_value());
    // ===== Cleanup ===== //
}

TEST_F(ExperimentalBaseSubscriberTest, ClearReceiveBufferCallForwardedToUnderlyingSubscriberPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), releaseQueuedChunks).Times(1);
    // ===== Test ===== //
    sut.clearReceiveBuffer();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(ExperimentalBaseSubscriberTest, SetConditionVariableCallForwardedToUnderlyingSubscriberPort)
{
    // ===== Setup ===== //
    auto conditionVariable = new iox::popo::ConditionVariableData();
    EXPECT_CALL(sut.getMockedPort(), setConditionVariable(conditionVariable)).Times(1);
    // ===== Test ===== //
    sut.setConditionVariable(conditionVariable);
    // ===== Verify ===== //
    // ===== Cleanup ===== //
    delete conditionVariable;
}

TEST_F(ExperimentalBaseSubscriberTest, UnsetConditionVariableCallForwardedToUnderlyingSubscriberPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), unsetConditionVariable).Times(1);
    // ===== Test ===== //
    sut.unsetConditionVariable();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}

TEST_F(ExperimentalBaseSubscriberTest, HasTriggeredCallForwardedToUnderlyingSubscriberPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.getMockedPort(), hasNewChunks).Times(1);
    // ===== Test ===== //
    sut.hasTriggered();
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}



