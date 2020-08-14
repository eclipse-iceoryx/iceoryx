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

class SubscriberPortSingleProducer_test : public Test
{
  public:
    static const iox::capro::ServiceDescription TEST_SERVICE_DESCRIPTION;

  protected:
    SubscriberPortSingleProducer_test()
    {
    }

    ~SubscriberPortSingleProducer_test()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

    iox::cxx::GenericRAII m_uniqueRouDiId{[] { iox::popo::internal::setUniqueRouDiId(0); },
                                          [] { iox::popo::internal::unsetUniqueRouDiId(); }};
    iox::popo::SubscriberPortData m_subscriberPortDataSingleProducer{
        TEST_SERVICE_DESCRIPTION, "myApp", iox::cxx::VariantQueueTypes::SoFi_SingleProducerSingleConsumer};
    iox::popo::SubscriberPortUser m_sutUserSideSingleProducer{&m_subscriberPortDataSingleProducer};
    iox::popo::SubscriberPortSingleProducer m_sutRouDiSideSingleProducer{&m_subscriberPortDataSingleProducer};
};

const iox::capro::ServiceDescription SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION("x", "y", "z");

TEST_F(SubscriberPortSingleProducer_test, initialStateNotSubscribed)
{
    EXPECT_THAT(m_sutUserSideSingleProducer.getSubscriptionState(), Eq(iox::SubscribeState::NOT_SUBSCRIBED));
}

TEST_F(SubscriberPortSingleProducer_test, initialStateNoChunksAvailable)
{
    auto maybeChunk = m_sutUserSideSingleProducer.getChunk();

    EXPECT_FALSE(maybeChunk.has_error());
    EXPECT_FALSE(maybeChunk.get_value().has_value());
    EXPECT_FALSE(m_sutUserSideSingleProducer.hasNewChunks());
}

TEST_F(SubscriberPortSingleProducer_test, initialStateNoChunksLost)
{
    EXPECT_FALSE(m_sutUserSideSingleProducer.hasLostChunks());
}

TEST_F(SubscriberPortSingleProducer_test, initialStateReturnsNoCaProMessage)
{
    auto maybeCaproMessage = m_sutRouDiSideSingleProducer.getCaProMessage();

    EXPECT_FALSE(maybeCaproMessage.has_value());
}

TEST_F(SubscriberPortSingleProducer_test, subscribeCallResultsInSubCaProMessage)
{
    m_sutUserSideSingleProducer.subscribe();

    auto maybeCaproMessage = m_sutRouDiSideSingleProducer.getCaProMessage();

    EXPECT_TRUE(maybeCaproMessage.has_value());
    auto caproMessage = maybeCaproMessage.value();
    EXPECT_THAT(caproMessage.m_type, Eq(iox::capro::CaproMessageType::SUB));
    EXPECT_THAT(caproMessage.m_serviceDescription, Eq(SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION));
    EXPECT_THAT(caproMessage.m_historyCapacity, Eq(0u));
}

TEST_F(SubscriberPortSingleProducer_test, subscribeRequestedWhenCallingSubscribe)
{
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::SUBSCRIBE_REQUESTED));
}

TEST_F(SubscriberPortSingleProducer_test, nackResponseOnSubResultsInWaitForOffer)
{
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::NACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::WAIT_FOR_OFFER));
}

TEST_F(SubscriberPortSingleProducer_test, ackResponseOnSubResultsInSubscribed)
{
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(SubscriberPortSingleProducer_test, offerInWaitForOfferTriggersSubMessage)
{
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::NACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);
    caproMessage.m_type = iox::capro::CaproMessageType::OFFER;

    auto maybeCaproMessage = m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);

    EXPECT_TRUE(maybeCaproMessage.has_value());
    auto caproMessageResponse = maybeCaproMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::SUB));
    EXPECT_THAT(caproMessageResponse.m_serviceDescription,
                Eq(SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION));
    EXPECT_THAT(caproMessageResponse.m_historyCapacity, Eq(0u));
}

TEST_F(SubscriberPortSingleProducer_test, offerInWaitForOfferResultsInSubscribeRequested)
{
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::NACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);
    caproMessage.m_type = iox::capro::CaproMessageType::OFFER;
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::SUBSCRIBE_REQUESTED));
}

TEST_F(SubscriberPortSingleProducer_test, unsubscribeInWaitForOfferResultsInNotSubscribed)
{
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::NACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);
    m_sutUserSideSingleProducer.unsubscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::NOT_SUBSCRIBED));
}

TEST_F(SubscriberPortSingleProducer_test, StopOfferInSubscribedResultsInWaitForOffer)
{
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);
    caproMessage.m_type = iox::capro::CaproMessageType::STOP_OFFER;
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::WAIT_FOR_OFFER));
}

TEST_F(SubscriberPortSingleProducer_test, unsubscribeInSubscribedTriggersUnsubMessage)
{
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);
    m_sutUserSideSingleProducer.unsubscribe();

    auto maybeCaproMessage = m_sutRouDiSideSingleProducer.getCaProMessage();

    EXPECT_TRUE(maybeCaproMessage.has_value());
    auto caproMessageResponse = maybeCaproMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::UNSUB));
    EXPECT_THAT(caproMessageResponse.m_serviceDescription,
                Eq(SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION));
}

TEST_F(SubscriberPortSingleProducer_test, unsubscribeInSubscribedResultsInUnsubscribeRequested)
{
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);
    m_sutUserSideSingleProducer.unsubscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::UNSUBSCRIBE_REQUESTED));
}

TEST_F(SubscriberPortSingleProducer_test, ackInUnsubscribeRequestedResultsInNotSubscribed)
{
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);
    m_sutUserSideSingleProducer.unsubscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::NOT_SUBSCRIBED));
}

TEST_F(SubscriberPortSingleProducer_test, nackInUnsubscribeRequestedResultsInNotSubscribed)
{
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);
    m_sutUserSideSingleProducer.unsubscribe();
    m_sutRouDiSideSingleProducer.getCaProMessage(); // only RouDi changes state
    caproMessage.m_type = iox::capro::CaproMessageType::NACK;
    m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::NOT_SUBSCRIBED));
}

TEST_F(SubscriberPortSingleProducer_test, invalidMessageResultsInError)
{
    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&errorHandlerCalled](const iox::Error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
        });
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);

    auto maybeCaproMessage = m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);

    EXPECT_FALSE(maybeCaproMessage.has_value());
    EXPECT_TRUE(errorHandlerCalled);
}

TEST_F(SubscriberPortSingleProducer_test, ackWhenNotWaitingForResultsInError)
{
    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&errorHandlerCalled](const iox::Error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
        });
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);

    auto maybeCaproMessage = m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);

    EXPECT_FALSE(maybeCaproMessage.has_value());
    EXPECT_TRUE(errorHandlerCalled);
}

TEST_F(SubscriberPortSingleProducer_test, nackWhenNotWaitingForResultsInError)
{
    auto errorHandlerCalled{false};
    iox::Error receivedError;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&errorHandlerCalled,
         &receivedError](const iox::Error error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
            receivedError = error;
        });
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::NACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);

    auto maybeCaproMessage = m_sutRouDiSideSingleProducer.dispatchCaProMessage(caproMessage);

    EXPECT_FALSE(maybeCaproMessage.has_value());
    EXPECT_TRUE(errorHandlerCalled);
    ASSERT_THAT(receivedError, Eq(iox::Error::kPOPO__CAPRO_PROTOCOL_ERROR));
}

class SubscriberPortMultiProducer_test : public Test
{
  protected:
    SubscriberPortMultiProducer_test()
    {
    }

    ~SubscriberPortMultiProducer_test()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

    iox::cxx::GenericRAII m_uniqueRouDiId{[] { iox::popo::internal::setUniqueRouDiId(0); },
                                          [] { iox::popo::internal::unsetUniqueRouDiId(); }};
    iox::popo::SubscriberPortData m_subscriberPortDataMultiProducer{
        SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION,
        "myApp",
        iox::cxx::VariantQueueTypes::SoFi_MultiProducerSingleConsumer};
    iox::popo::SubscriberPortUser m_sutUserSideMultiProducer{&m_subscriberPortDataMultiProducer};
    iox::popo::SubscriberPortMultiProducer m_sutRouDiSideMultiProducer{&m_subscriberPortDataMultiProducer};
};

TEST_F(SubscriberPortMultiProducer_test, initialStateNotSubscribed)
{
    EXPECT_THAT(m_sutUserSideMultiProducer.getSubscriptionState(), Eq(iox::SubscribeState::NOT_SUBSCRIBED));
}

TEST_F(SubscriberPortMultiProducer_test, initialStateReturnsNoCaProMessage)
{
    auto maybeCaproMessage = m_sutRouDiSideMultiProducer.getCaProMessage();

    EXPECT_FALSE(maybeCaproMessage.has_value());
}

TEST_F(SubscriberPortMultiProducer_test, subscribeCallResultsInSubCaProMessage)
{
    m_sutUserSideMultiProducer.subscribe();

    auto maybeCaproMessage = m_sutRouDiSideMultiProducer.getCaProMessage();

    EXPECT_TRUE(maybeCaproMessage.has_value());
    auto caproMessage = maybeCaproMessage.value();
    EXPECT_THAT(caproMessage.m_type, Eq(iox::capro::CaproMessageType::SUB));
    EXPECT_THAT(caproMessage.m_serviceDescription, Eq(SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION));
    EXPECT_THAT(caproMessage.m_historyCapacity, Eq(0u));
}

TEST_F(SubscriberPortMultiProducer_test, subscribedWhenCallingSubscribe)
{
    m_sutUserSideMultiProducer.subscribe();
    m_sutRouDiSideMultiProducer.getCaProMessage(); // only RouDi changes state

    const auto subscriptionState = m_sutUserSideMultiProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(SubscriberPortMultiProducer_test, nackResponseOnSubStillSubscribed)
{
    m_sutUserSideMultiProducer.subscribe();
    m_sutRouDiSideMultiProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::NACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideMultiProducer.dispatchCaProMessage(caproMessage);

    const auto subscriptionState = m_sutUserSideMultiProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(SubscriberPortMultiProducer_test, ackResponseOnSubStillSubscribed)
{
    m_sutUserSideMultiProducer.subscribe();
    m_sutRouDiSideMultiProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideMultiProducer.dispatchCaProMessage(caproMessage);

    const auto subscriptionState = m_sutUserSideMultiProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(SubscriberPortMultiProducer_test, offerInSubscribedTriggersSubMessage)
{
    m_sutUserSideMultiProducer.subscribe();
    m_sutRouDiSideMultiProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::OFFER,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);

    auto maybeCaproMessage = m_sutRouDiSideMultiProducer.dispatchCaProMessage(caproMessage);

    EXPECT_TRUE(maybeCaproMessage.has_value());
    auto caproMessageResponse = maybeCaproMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::SUB));
    EXPECT_THAT(caproMessageResponse.m_serviceDescription,
                Eq(SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION));
    EXPECT_THAT(caproMessageResponse.m_historyCapacity, Eq(0u));
}

TEST_F(SubscriberPortMultiProducer_test, unsubscribeInSubscribedResultsInNotSubscribed)
{
    m_sutUserSideMultiProducer.subscribe();
    m_sutRouDiSideMultiProducer.getCaProMessage(); // only RouDi changes state
    m_sutUserSideMultiProducer.unsubscribe();
    m_sutRouDiSideMultiProducer.getCaProMessage(); // only RouDi changes state

    const auto subscriptionState = m_sutUserSideMultiProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::NOT_SUBSCRIBED));
}

TEST_F(SubscriberPortMultiProducer_test, StopOfferInSubscribedRemainsInSubscribed)
{
    m_sutUserSideMultiProducer.subscribe();
    m_sutRouDiSideMultiProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideMultiProducer.dispatchCaProMessage(caproMessage);
    caproMessage.m_type = iox::capro::CaproMessageType::STOP_OFFER;
    m_sutRouDiSideMultiProducer.dispatchCaProMessage(caproMessage);

    const auto subscriptionState = m_sutUserSideMultiProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(SubscriberPortMultiProducer_test, unsubscribeInSubscribedTriggersUnsubMessage)
{
    m_sutUserSideMultiProducer.subscribe();
    m_sutRouDiSideMultiProducer.getCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideMultiProducer.dispatchCaProMessage(caproMessage);
    m_sutUserSideMultiProducer.unsubscribe();

    auto maybeCaproMessage = m_sutRouDiSideMultiProducer.getCaProMessage();

    EXPECT_TRUE(maybeCaproMessage.has_value());
    auto caproMessageResponse = maybeCaproMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::UNSUB));
    EXPECT_THAT(caproMessageResponse.m_serviceDescription,
                Eq(SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION));
}

TEST_F(SubscriberPortMultiProducer_test, invalidMessageResultsInError)
{
    auto errorHandlerCalled{false};
    iox::Error receivedError;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&errorHandlerCalled,
         &receivedError](const iox::Error error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
            receivedError = error;
        });
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::UNSUB,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);

    auto maybeCaproMessage = m_sutRouDiSideMultiProducer.dispatchCaProMessage(caproMessage);

    EXPECT_FALSE(maybeCaproMessage.has_value());
    EXPECT_TRUE(errorHandlerCalled);
    ASSERT_THAT(receivedError, Eq(iox::Error::kPOPO__CAPRO_PROTOCOL_ERROR));
}
