// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_data.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_queue_popper.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_multi_producer.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_single_producer.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/popo/subscriber_options.hpp"

#include "iceoryx_hoofs/testing/error_reporting/testing_support.hpp"
#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "test.hpp"

#include <memory>

namespace
{
using namespace ::testing;
using namespace iox::testing;

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

    iox::popo::SubscriberOptions m_noSubscribeOnCreateOptions{
        iox::popo::SubscriberPortData::ChunkQueueData_t::MAX_CAPACITY, 0U, iox::NodeName_t(""), false};
    iox::popo::SubscriberPortData m_subscriberPortDataSingleProducer{
        TEST_SERVICE_DESCRIPTION,
        "myApp",
        iox::roudi::DEFAULT_UNIQUE_ROUDI_ID,
        iox::popo::VariantQueueTypes::SoFi_SingleProducerSingleConsumer,
        m_noSubscribeOnCreateOptions};
    iox::popo::SubscriberPortUser m_sutUserSideSingleProducer{&m_subscriberPortDataSingleProducer};
    iox::popo::SubscriberPortSingleProducer m_sutRouDiSideSingleProducer{&m_subscriberPortDataSingleProducer};

    iox::popo::SubscriberOptions m_defaultSubscriberOptions{};
    iox::popo::SubscriberPortData m_subscriberPortDataDefaultOptions{
        TEST_SERVICE_DESCRIPTION,
        "myApp",
        iox::roudi::DEFAULT_UNIQUE_ROUDI_ID,
        iox::popo::VariantQueueTypes::SoFi_SingleProducerSingleConsumer,
        m_defaultSubscriberOptions};
    iox::popo::SubscriberPortUser m_sutUserSideDefaultOptions{&m_subscriberPortDataDefaultOptions};
    iox::popo::SubscriberPortSingleProducer m_sutRouDiSideDefaultOptions{&m_subscriberPortDataDefaultOptions};
};

const iox::capro::ServiceDescription SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION("x", "y", "z");

TEST_F(SubscriberPortSingleProducer_test, InitialStateNotSubscribed)
{
    ::testing::Test::RecordProperty("TEST_ID", "3f4429fa-aa94-42de-9f53-bcd15a1e5b60");
    EXPECT_THAT(m_sutUserSideSingleProducer.getSubscriptionState(), Eq(iox::SubscribeState::NOT_SUBSCRIBED));
}

TEST_F(SubscriberPortSingleProducer_test, InitialStateNoChunksAvailable)
{
    ::testing::Test::RecordProperty("TEST_ID", "11a25a0b-eea6-42f9-8937-955ab014916c");
    auto maybeChunkHeader = m_sutUserSideSingleProducer.tryGetChunk();

    ASSERT_TRUE(maybeChunkHeader.has_error());
    EXPECT_EQ(maybeChunkHeader.error(), iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE);
    EXPECT_FALSE(m_sutUserSideSingleProducer.hasNewChunks());
}

TEST_F(SubscriberPortSingleProducer_test, InitialStateNoChunksLost)
{
    ::testing::Test::RecordProperty("TEST_ID", "d59df0c5-8635-41ab-b0fe-51c57fb9d66a");
    EXPECT_FALSE(m_sutUserSideSingleProducer.hasLostChunksSinceLastCall());
}

TEST_F(SubscriberPortSingleProducer_test, InitialStateReturnsNoCaProMessageWhenNoSubOnCreate)
{
    ::testing::Test::RecordProperty("TEST_ID", "2957282d-2c80-4d1c-bace-59d3f8e23a3f");
    auto maybeCaproMessage = m_sutRouDiSideSingleProducer.tryGetCaProMessage();

    EXPECT_FALSE(maybeCaproMessage.has_value());
}

TEST_F(SubscriberPortSingleProducer_test, InitialStateReturnsSubCaProMessageWithDefaultOptions)
{
    ::testing::Test::RecordProperty("TEST_ID", "e4e92212-eff9-40cb-87a2-10c650185217");
    auto maybeCaproMessage = m_sutRouDiSideDefaultOptions.tryGetCaProMessage();

    ASSERT_TRUE(maybeCaproMessage.has_value());
    auto caproMessage = maybeCaproMessage.value();
    EXPECT_THAT(caproMessage.m_type, Eq(iox::capro::CaproMessageType::SUB));
}

TEST_F(SubscriberPortSingleProducer_test, SubscribeCallResultsInSubCaProMessage)
{
    ::testing::Test::RecordProperty("TEST_ID", "3efdb6ed-4b04-4a28-8e5a-aeb7329ad188");
    m_sutUserSideSingleProducer.subscribe();

    auto maybeCaproMessage = m_sutRouDiSideSingleProducer.tryGetCaProMessage();

    ASSERT_TRUE(maybeCaproMessage.has_value());
    auto caproMessage = maybeCaproMessage.value();
    EXPECT_THAT(caproMessage.m_type, Eq(iox::capro::CaproMessageType::SUB));
    EXPECT_THAT(caproMessage.m_serviceDescription, Eq(SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION));
    EXPECT_THAT(caproMessage.m_historyCapacity, Eq(0u));
}

TEST_F(SubscriberPortSingleProducer_test, SubscribeRequestedWhenCallingSubscribe)
{
    ::testing::Test::RecordProperty("TEST_ID", "cccc4f43-61c9-4785-af2d-29cbad7420da");
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.tryGetCaProMessage(); // only RouDi changes state

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::SUBSCRIBE_REQUESTED));
}

TEST_F(SubscriberPortSingleProducer_test, NackResponseOnSubResultsInWaitForOffer)
{
    ::testing::Test::RecordProperty("TEST_ID", "83a4dedd-bb97-4991-a247-28334939263f");
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.tryGetCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::NACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideSingleProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::WAIT_FOR_OFFER));
}

TEST_F(SubscriberPortSingleProducer_test, AckResponseOnSubResultsInSubscribed)
{
    ::testing::Test::RecordProperty("TEST_ID", "f12ab584-0e74-438c-b39a-a447e2540d5c");
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.tryGetCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideSingleProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(SubscriberPortSingleProducer_test, OfferInWaitForOfferTriggersSubMessage)
{
    ::testing::Test::RecordProperty("TEST_ID", "ffa573b9-db23-401c-95ee-85ba80833464");
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.tryGetCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::NACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideSingleProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    caproMessage.m_type = iox::capro::CaproMessageType::OFFER;

    auto maybeCaproMessage = m_sutRouDiSideSingleProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    ASSERT_TRUE(maybeCaproMessage.has_value());
    auto caproMessageResponse = maybeCaproMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::SUB));
    EXPECT_THAT(caproMessageResponse.m_serviceDescription,
                Eq(SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION));
    EXPECT_THAT(caproMessageResponse.m_historyCapacity, Eq(0u));
}

TEST_F(SubscriberPortSingleProducer_test, OfferInWaitForOfferResultsInSubscribeRequested)
{
    ::testing::Test::RecordProperty("TEST_ID", "4045be99-d1ef-4f3e-98c1-e956a19da1a1");
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.tryGetCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::NACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideSingleProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    caproMessage.m_type = iox::capro::CaproMessageType::OFFER;
    m_sutRouDiSideSingleProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::SUBSCRIBE_REQUESTED));
}

TEST_F(SubscriberPortSingleProducer_test, UnsubscribeInWaitForOfferResultsInNotSubscribed)
{
    ::testing::Test::RecordProperty("TEST_ID", "8bc335a2-441f-47e0-8681-d753d824fb52");
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.tryGetCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::NACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideSingleProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    m_sutUserSideSingleProducer.unsubscribe();
    m_sutRouDiSideSingleProducer.tryGetCaProMessage(); // only RouDi changes state

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::NOT_SUBSCRIBED));
}

TEST_F(SubscriberPortSingleProducer_test, StopOfferInSubscribedResultsInWaitForOffer)
{
    ::testing::Test::RecordProperty("TEST_ID", "227d649e-0a53-41f5-8d22-4ead7a39379d");
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.tryGetCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideSingleProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    caproMessage.m_type = iox::capro::CaproMessageType::STOP_OFFER;
    m_sutRouDiSideSingleProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::WAIT_FOR_OFFER));
}

TEST_F(SubscriberPortSingleProducer_test, UnsubscribeInSubscribedTriggersUnsubMessage)
{
    ::testing::Test::RecordProperty("TEST_ID", "e865a067-d68d-4d29-a6d4-55819632c8dc");
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.tryGetCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideSingleProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    m_sutUserSideSingleProducer.unsubscribe();

    auto maybeCaproMessage = m_sutRouDiSideSingleProducer.tryGetCaProMessage();

    ASSERT_TRUE(maybeCaproMessage.has_value());
    auto caproMessageResponse = maybeCaproMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::UNSUB));
    EXPECT_THAT(caproMessageResponse.m_serviceDescription,
                Eq(SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION));
}

TEST_F(SubscriberPortSingleProducer_test, UnsubscribeInSubscribedResultsInUnsubscribeRequested)
{
    ::testing::Test::RecordProperty("TEST_ID", "040b1cbb-5f63-4da4-b63d-a8bd02773888");
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.tryGetCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideSingleProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    m_sutUserSideSingleProducer.unsubscribe();
    m_sutRouDiSideSingleProducer.tryGetCaProMessage(); // only RouDi changes state

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::UNSUBSCRIBE_REQUESTED));
}

TEST_F(SubscriberPortSingleProducer_test, AckInUnsubscribeRequestedResultsInNotSubscribed)
{
    ::testing::Test::RecordProperty("TEST_ID", "62a1d42c-6d5f-46ed-a758-411ae074e2cf");
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.tryGetCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideSingleProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    m_sutUserSideSingleProducer.unsubscribe();
    m_sutRouDiSideSingleProducer.tryGetCaProMessage(); // only RouDi changes state
    m_sutRouDiSideSingleProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::NOT_SUBSCRIBED));
}

TEST_F(SubscriberPortSingleProducer_test, NackInUnsubscribeRequestedResultsInNotSubscribed)
{
    ::testing::Test::RecordProperty("TEST_ID", "32592636-20c6-4b0f-8d27-c48c0fc8aa0f");
    m_sutUserSideSingleProducer.subscribe();
    m_sutRouDiSideSingleProducer.tryGetCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideSingleProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    m_sutUserSideSingleProducer.unsubscribe();
    m_sutRouDiSideSingleProducer.tryGetCaProMessage(); // only RouDi changes state
    caproMessage.m_type = iox::capro::CaproMessageType::NACK;
    m_sutRouDiSideSingleProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    const auto subscriptionState = m_sutUserSideSingleProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::NOT_SUBSCRIBED));
}

TEST_F(SubscriberPortSingleProducer_test, InvalidMessageResultsInError)
{
    ::testing::Test::RecordProperty("TEST_ID", "23aaa4fd-5567-4831-b539-802c5de238ab");

    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::SUB,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);

    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            m_sutRouDiSideSingleProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage)
                .and_then([&](const auto& responseCaproMessage) {
                    GTEST_FAIL() << "Expected no CaPro message but got: " << responseCaproMessage.m_type;
                });
        },
        iox::PoshError::POPO__CAPRO_PROTOCOL_ERROR);
}

TEST_F(SubscriberPortSingleProducer_test, AckWhenNotWaitingForResultsInError)
{
    ::testing::Test::RecordProperty("TEST_ID", "541719e5-fdfa-4ef8-86f6-a9baf4919fe8");

    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);

    IOX_EXPECT_FATAL_FAILURE(
        [&] {
            m_sutRouDiSideSingleProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage)
                .and_then([&](const auto& responseCaproMessage) {
                    GTEST_FAIL() << "Expected no CaPro message but got: " << responseCaproMessage.m_type;
                });
        },
        iox::PoshError::POPO__CAPRO_PROTOCOL_ERROR);
}

TEST_F(SubscriberPortSingleProducer_test, NackWhenNotWaitingForResultsInError)
{
    ::testing::Test::RecordProperty("TEST_ID", "063e3a61-209b-4755-abfa-69aed6258ab3");

    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::NACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);

    IOX_EXPECT_FATAL_FAILURE(
        [&] { m_sutRouDiSideSingleProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage); },
        iox::PoshError::POPO__CAPRO_PROTOCOL_ERROR);
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

    iox::popo::SubscriberPortData m_subscriberPortDataMultiProducer{
        SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION,
        "myApp",
        iox::roudi::DEFAULT_UNIQUE_ROUDI_ID,
        iox::popo::VariantQueueTypes::SoFi_MultiProducerSingleConsumer,
        iox::popo::SubscriberOptions()};
    iox::popo::SubscriberPortUser m_sutUserSideMultiProducer{&m_subscriberPortDataMultiProducer};
    iox::popo::SubscriberPortMultiProducer m_sutRouDiSideMultiProducer{&m_subscriberPortDataMultiProducer};
};

TEST_F(SubscriberPortMultiProducer_test, InitialStateNotSubscribed)
{
    ::testing::Test::RecordProperty("TEST_ID", "03cdb90f-d34d-47c7-866a-097db0d2852f");
    EXPECT_THAT(m_sutUserSideMultiProducer.getSubscriptionState(), Eq(iox::SubscribeState::NOT_SUBSCRIBED));
}

TEST_F(SubscriberPortMultiProducer_test, InitialStateReturnsSubCaProMessageWithDefaultOptions)
{
    ::testing::Test::RecordProperty("TEST_ID", "967afa81-ca32-4895-91d6-b1217d0408fa");
    auto maybeCaproMessage = m_sutRouDiSideMultiProducer.tryGetCaProMessage();

    ASSERT_TRUE(maybeCaproMessage.has_value());
    auto caproMessage = maybeCaproMessage.value();
    EXPECT_THAT(caproMessage.m_type, Eq(iox::capro::CaproMessageType::SUB));
}

TEST_F(SubscriberPortMultiProducer_test, SubscribeCallResultsInSubCaProMessage)
{
    ::testing::Test::RecordProperty("TEST_ID", "3676eadc-a1f1-4ea8-838c-b9940977bba7");
    m_sutUserSideMultiProducer.subscribe();

    auto maybeCaproMessage = m_sutRouDiSideMultiProducer.tryGetCaProMessage();

    ASSERT_TRUE(maybeCaproMessage.has_value());
    auto caproMessage = maybeCaproMessage.value();
    EXPECT_THAT(caproMessage.m_type, Eq(iox::capro::CaproMessageType::SUB));
    EXPECT_THAT(caproMessage.m_serviceDescription, Eq(SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION));
    EXPECT_THAT(caproMessage.m_historyCapacity, Eq(0u));
}

TEST_F(SubscriberPortMultiProducer_test, SubscribedWhenCallingSubscribe)
{
    ::testing::Test::RecordProperty("TEST_ID", "a9ee3134-6135-477f-82b4-5f11fdb534c5");
    m_sutUserSideMultiProducer.subscribe();
    m_sutRouDiSideMultiProducer.tryGetCaProMessage(); // only RouDi changes state

    const auto subscriptionState = m_sutUserSideMultiProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(SubscriberPortMultiProducer_test, NackResponseOnSubStillSubscribed)
{
    ::testing::Test::RecordProperty("TEST_ID", "c741d9d0-e644-4417-8842-2c9a6923fab2");
    m_sutUserSideMultiProducer.subscribe();
    m_sutRouDiSideMultiProducer.tryGetCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::NACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideMultiProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    const auto subscriptionState = m_sutUserSideMultiProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(SubscriberPortMultiProducer_test, AckResponseOnSubStillSubscribed)
{
    ::testing::Test::RecordProperty("TEST_ID", "9592b3f7-6fe7-46a9-92e4-dd263bd3d748");
    m_sutUserSideMultiProducer.subscribe();
    m_sutRouDiSideMultiProducer.tryGetCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideMultiProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    const auto subscriptionState = m_sutUserSideMultiProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(SubscriberPortMultiProducer_test, OfferInSubscribedTriggersSubMessage)
{
    ::testing::Test::RecordProperty("TEST_ID", "386e1cf5-26fd-4883-9f22-214922ff50d5");
    m_sutUserSideMultiProducer.subscribe();
    m_sutRouDiSideMultiProducer.tryGetCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::OFFER,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);

    auto maybeCaproMessage = m_sutRouDiSideMultiProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    ASSERT_TRUE(maybeCaproMessage.has_value());
    auto caproMessageResponse = maybeCaproMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::SUB));
    EXPECT_THAT(caproMessageResponse.m_serviceDescription,
                Eq(SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION));
    EXPECT_THAT(caproMessageResponse.m_historyCapacity, Eq(0u));
}

TEST_F(SubscriberPortMultiProducer_test, UnsubscribeInSubscribedResultsInNotSubscribed)
{
    ::testing::Test::RecordProperty("TEST_ID", "61291a6b-d199-4be4-baba-aeac1cdc97e1");
    m_sutUserSideMultiProducer.subscribe();
    m_sutRouDiSideMultiProducer.tryGetCaProMessage(); // only RouDi changes state
    m_sutUserSideMultiProducer.unsubscribe();
    m_sutRouDiSideMultiProducer.tryGetCaProMessage(); // only RouDi changes state

    const auto subscriptionState = m_sutUserSideMultiProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::NOT_SUBSCRIBED));
}

TEST_F(SubscriberPortMultiProducer_test, StopOfferInSubscribedRemainsInSubscribed)
{
    ::testing::Test::RecordProperty("TEST_ID", "fe2996eb-435d-4196-a772-6834f4937c5a");
    m_sutUserSideMultiProducer.subscribe();
    m_sutRouDiSideMultiProducer.tryGetCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideMultiProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    caproMessage.m_type = iox::capro::CaproMessageType::STOP_OFFER;
    m_sutRouDiSideMultiProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage);

    const auto subscriptionState = m_sutUserSideMultiProducer.getSubscriptionState();

    EXPECT_THAT(subscriptionState, Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(SubscriberPortMultiProducer_test, UnsubscribeInSubscribedTriggersUnsubMessage)
{
    ::testing::Test::RecordProperty("TEST_ID", "1a466097-60a6-4566-b7a3-5aedc7f71dd7");
    m_sutUserSideMultiProducer.subscribe();
    m_sutRouDiSideMultiProducer.tryGetCaProMessage(); // only RouDi changes state
    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::ACK,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);
    m_sutRouDiSideMultiProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage);
    m_sutUserSideMultiProducer.unsubscribe();

    auto maybeCaproMessage = m_sutRouDiSideMultiProducer.tryGetCaProMessage();

    ASSERT_TRUE(maybeCaproMessage.has_value());
    auto caproMessageResponse = maybeCaproMessage.value();
    EXPECT_THAT(caproMessageResponse.m_type, Eq(iox::capro::CaproMessageType::UNSUB));
    EXPECT_THAT(caproMessageResponse.m_serviceDescription,
                Eq(SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION));
}

TEST_F(SubscriberPortMultiProducer_test, InvalidMessageResultsInError)
{
    ::testing::Test::RecordProperty("TEST_ID", "419aa91f-991b-4814-b1ee-11637ee14d30");

    iox::capro::CaproMessage caproMessage(iox::capro::CaproMessageType::UNSUB,
                                          SubscriberPortSingleProducer_test::TEST_SERVICE_DESCRIPTION);

    IOX_EXPECT_FATAL_FAILURE(
        [&] { m_sutRouDiSideMultiProducer.dispatchCaProMessageAndGetPossibleResponse(caproMessage); },
        iox::PoshError::POPO__CAPRO_PROTOCOL_ERROR);
}

} // namespace
