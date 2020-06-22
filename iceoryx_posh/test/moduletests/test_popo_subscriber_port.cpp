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

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_data.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_popper.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_multi_producer.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_single_producer.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"

#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"
#include "test.hpp"

#include <memory>

using namespace ::testing;

class SubscriberPort_test : public Test
{
  protected:
    SubscriberPort_test()
    {
    }

    ~SubscriberPort_test()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

    iox::popo::SubscriberPortData m_subscriberPortDataSingleProducer{
        iox::capro::ServiceDescription("x", "y", "z"),
        "myApp",
        iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::popo::SubscriberPortUser m_sutUserSideSingleProducer{&m_subscriberPortDataSingleProducer};
    iox::popo::SubscriberPortSingleProducer m_sutRouDiSideSingleProducer{&m_subscriberPortDataSingleProducer};
};

TEST_F(SubscriberPort_test, initialStateNotSubscribed)
{
    EXPECT_THAT(m_sutUserSideSingleProducer.getSubscriptionState(), Eq(iox::SubscribeState::NOT_SUBSCRIBED));
}

TEST_F(SubscriberPort_test, initialStateNoChunksAvailable)
{
    auto maybeChunk = m_sutUserSideSingleProducer.getChunk();

    EXPECT_FALSE(maybeChunk.has_error());
    EXPECT_FALSE(maybeChunk.get_value().has_value());
    EXPECT_FALSE(m_sutUserSideSingleProducer.hasNewChunks());
}

TEST_F(SubscriberPort_test, initialStateNoChunksLost)
{
    EXPECT_FALSE(m_sutUserSideSingleProducer.hasLostChunks());
}

TEST_F(SubscriberPort_test, initialStateReturnsNoCaProMessage)
{
    auto maybeCaproMessage = m_sutRouDiSideSingleProducer.getCaProMessage();

    EXPECT_FALSE(maybeCaproMessage.has_value());
}

TEST_F(SubscriberPort_test, subscribeCallResultsInSubCaProMessage)
{
    m_sutUserSideSingleProducer.subscribe();

    auto maybeCaproMessage = m_sutRouDiSideSingleProducer.getCaProMessage();

    EXPECT_TRUE(maybeCaproMessage.has_value());
    auto caproMessage = maybeCaproMessage.value();
    EXPECT_THAT(caproMessage.m_type, Eq(iox::capro::CaproMessageType::SUB));
    EXPECT_THAT(caproMessage.m_serviceDescription, Eq(iox::capro::ServiceDescription("x", "y", "z")));
    EXPECT_THAT(caproMessage.m_historyCapacity, Eq(0u));
}

TEST_F(SubscriberPort_test, subscribeRequestedWhenCallingSubscribe)
{
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::SUBSCRIBE_REQUESTED));
}

TEST_F(SubscriberPort_test, nackResponseOnSubResultsInWaitForOffer)
{
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::NACK,
                                          iox::capro::ServiceDescription("x", "y", "z"));
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::WAIT_FOR_OFFER));
}

TEST_F(SubscriberPort_test, ackResponseOnSubResultsInSubscribed)
{
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          iox::capro::ServiceDescription("x", "y", "z"));
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(SubscriberPort_test, offerInWaitForOfferTriggersSubMessage)
{
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::NACK,
                                          iox::capro::ServiceDescription("x", "y", "z"));
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);
    caproMessage.m_type = iox::capro::CaproMessageType::OFFER;

    auto maybeCaproMessage = m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);

    EXPECT_TRUE(maybeCaproMessage.has_value());
    auto caproMessageResponse = maybeCaproMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::SUB));
    EXPECT_THAT(caproMessageResponse.m_serviceDescription, Eq(iox::capro::ServiceDescription("x", "y", "z")));
    EXPECT_THAT(caproMessageResponse.m_historyCapacity, Eq(0u));
}

TEST_F(SubscriberPort_test, offerInWaitForOfferResultsInSubscribeRequested)
{
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::NACK,
                                          iox::capro::ServiceDescription("x", "y", "z"));
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);
    caproMessage.m_type = iox::capro::CaproMessageType::OFFER;
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::SUBSCRIBE_REQUESTED));
}

TEST_F(SubscriberPort_test, unsubscribeInWaitForOfferResultsInNotSubscribed)
{
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::NACK,
                                          iox::capro::ServiceDescription("x", "y", "z"));
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);
    m_sutUserSideSingleProducer.unsubscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::NOT_SUBSCRIBED));
}

TEST_F(SubscriberPort_test, StopOfferInSubscribedResultsInWaitForOffer)
{
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          iox::capro::ServiceDescription("x", "y", "z"));
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);
    caproMessage.m_type = iox::capro::CaproMessageType::STOP_OFFER;
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::WAIT_FOR_OFFER));
}

TEST_F(SubscriberPort_test, unsubscribeInSubscribedTriggersUnsubMessage)
{
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          iox::capro::ServiceDescription("x", "y", "z"));
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);
    m_sutUserSideSingleProducer.unsubscribe();

    auto maybeCaproMessage = m_sutRouDiSideSingleProducer.getCaProMessage();

    EXPECT_TRUE(maybeCaproMessage.has_value());
    auto caproMessageResponse = maybeCaproMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::UNSUB));
    EXPECT_THAT(caproMessageResponse.m_serviceDescription, Eq(iox::capro::ServiceDescription("x", "y", "z")));
}

TEST_F(SubscriberPort_test, unsubscribeInSubscribedResultsInUnsubscribeRequested)
{
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          iox::capro::ServiceDescription("x", "y", "z"));
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);
    m_sutUserSideSingleProducer.unsubscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::UNSUBSCRIBE_REQUESTED));
}

TEST_F(SubscriberPort_test, ackInUnsubscribeRequestedResultsInNotSubscribed)
{
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          iox::capro::ServiceDescription("x", "y", "z"));
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);
    m_sutUserSideSingleProducer.unsubscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::NOT_SUBSCRIBED));
}

TEST_F(SubscriberPort_test, nackInUnsubscribeRequestedResultsInNotSubscribed)
{
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          iox::capro::ServiceDescription("x", "y", "z"));
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);
    m_sutUserSideSingleProducer.unsubscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    caproMessage.m_type = iox::capro::CaproMessageType::NACK;
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::NOT_SUBSCRIBED));
}

TEST_F(SubscriberPort_test, invalidMessageResultsInError)
{
    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler([&errorHandlerCalled](
        const iox::Error, const std::function<void()>, const iox::ErrorLevel) { errorHandlerCalled = true; });
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                          iox::capro::ServiceDescription("x", "y", "z"));

    auto maybeCaproMessage = m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);

    EXPECT_FALSE(maybeCaproMessage.has_value());
    EXPECT_TRUE(errorHandlerCalled);
}