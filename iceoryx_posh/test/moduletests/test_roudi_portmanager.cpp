// Copyright (c) 2019 - 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/testing/barrier.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iox/atomic.hpp"
#include "iox/std_string_support.hpp"
#include "test_roudi_portmanager_fixture.hpp"

#include "iceoryx_hoofs/testing/error_reporting/testing_support.hpp"

namespace iox_test_roudi_portmanager
{
using iox::into;
using iox::lossy;

PublisherOptions createTestPubOptions()
{
    return PublisherOptions{0U, iox::NodeName_t("node"), true, iox::popo::ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA};
}

SubscriberOptions createTestSubOptions()
{
    return SubscriberOptions{1U, 0U, iox::NodeName_t("node"), true, QueueFullPolicy::DISCARD_OLDEST_DATA, false};
}

TEST_F(PortManager_test, DoDiscoveryWithSingleShotPublisherFirst)
{
    ::testing::Test::RecordProperty("TEST_ID", "f767cb6a-ae82-45e5-9969-d75be1077fc0");
    PublisherOptions publisherOptions{1U, iox::NodeName_t("node"), false};
    SubscriberOptions subscriberOptions{1U, 1U, iox::NodeName_t("node"), false};

    PublisherPortUser publisher(
        m_portManager
            ->acquirePublisherPortData(
                {"1", "1", "1"}, publisherOptions, "guiseppe", m_payloadDataSegmentMemoryManager, PortConfigInfo())
            .value());
    ASSERT_TRUE(publisher);
    publisher.offer();
    // no doDiscovery() at this position is intentional

    SubscriberPortUser subscriber(
        m_portManager->acquireSubscriberPortData({"1", "1", "1"}, subscriberOptions, "schlomo", PortConfigInfo())
            .value());
    ASSERT_TRUE(subscriber);
    subscriber.subscribe();

    m_portManager->doDiscovery();

    ASSERT_TRUE(publisher.hasSubscribers());
    EXPECT_THAT(subscriber.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(PortManager_test, DoDiscoveryWithSingleShotSubscriberFirst)
{
    ::testing::Test::RecordProperty("TEST_ID", "bef1fc7f-3661-4dcc-98dd-fbf951ed275c");
    PublisherOptions publisherOptions{1U, iox::NodeName_t("node"), false};
    SubscriberOptions subscriberOptions{1U, 1U, iox::NodeName_t("node"), false};

    SubscriberPortUser subscriber(
        m_portManager->acquireSubscriberPortData({"1", "1", "1"}, subscriberOptions, "schlomo", PortConfigInfo())
            .value());
    ASSERT_TRUE(subscriber);
    subscriber.subscribe();
    // no doDiscovery() at this position is intentional

    PublisherPortUser publisher(
        m_portManager
            ->acquirePublisherPortData(
                {"1", "1", "1"}, publisherOptions, "guiseppe", m_payloadDataSegmentMemoryManager, PortConfigInfo())
            .value());
    ASSERT_TRUE(publisher);
    publisher.offer();

    m_portManager->doDiscovery();

    ASSERT_TRUE(publisher.hasSubscribers());
    EXPECT_THAT(subscriber.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(PortManager_test, DoDiscoveryWithDiscoveryLoopInBetweenCreationOfSubscriberAndPublisher)
{
    ::testing::Test::RecordProperty("TEST_ID", "bbd475bd-23fd-4b8f-b2ae-88e41c39e6e2");
    PublisherOptions publisherOptions{1U, iox::NodeName_t("node"), false};
    SubscriberOptions subscriberOptions{1U, 1U, iox::NodeName_t("node"), false};

    SubscriberPortUser subscriber(
        m_portManager->acquireSubscriberPortData({"1", "1", "1"}, subscriberOptions, " schlomo ", PortConfigInfo())
            .value());
    ASSERT_TRUE(subscriber);
    subscriber.subscribe();
    m_portManager->doDiscovery();

    PublisherPortUser publisher(
        m_portManager
            ->acquirePublisherPortData(
                {"1", "1", "1"}, publisherOptions, "guiseppe", m_payloadDataSegmentMemoryManager, PortConfigInfo())
            .value());
    ASSERT_TRUE(publisher);
    publisher.offer();

    m_portManager->doDiscovery();

    ASSERT_TRUE(publisher.hasSubscribers());
    EXPECT_THAT(subscriber.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(PortManager_test, DoDiscoveryWithSubscribersCreatedBeforeAndAfterCreationOfPublisher)
{
    ::testing::Test::RecordProperty("TEST_ID", "b1c5bf2e-066e-4f01-b92a-edab9197a5dd");
    PublisherOptions publisherOptions{1U, iox::NodeName_t("node"), false};
    SubscriberOptions subscriberOptions{1U, 1U, iox::NodeName_t("node"), false};

    SubscriberPortUser subscriber1(
        m_portManager->acquireSubscriberPortData({"1", "1", "1"}, subscriberOptions, "schlomo", PortConfigInfo())
            .value());
    ASSERT_TRUE(subscriber1);
    subscriber1.subscribe();

    m_portManager->doDiscovery();

    PublisherPortUser publisher(
        m_portManager
            ->acquirePublisherPortData(
                {"1", "1", "1"}, publisherOptions, "guiseppe", m_payloadDataSegmentMemoryManager, PortConfigInfo())
            .value());
    ASSERT_TRUE(publisher);
    publisher.offer();

    SubscriberPortUser subscriber2(
        m_portManager->acquireSubscriberPortData({"1", "1", " 1 "}, subscriberOptions, " ingnatz ", PortConfigInfo())
            .value());
    ASSERT_TRUE(subscriber2);
    subscriber2.subscribe();

    m_portManager->doDiscovery();

    ASSERT_TRUE(publisher.hasSubscribers());
    EXPECT_THAT(subscriber1.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
    EXPECT_THAT(subscriber2.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(PortManager_test, SubscribeOnCreateSubscribesWithoutDiscoveryLoopWhenPublisherAvailable)
{
    ::testing::Test::RecordProperty("TEST_ID", "5a94cf82-d1f6-4129-88ca-34344d94e04e");
    PublisherOptions publisherOptions{1U, iox::NodeName_t("node"), false};
    SubscriberOptions subscriberOptions{1U, 1U, iox::NodeName_t("node"), true};
    PublisherPortUser publisher(
        m_portManager
            ->acquirePublisherPortData(
                {"1", "1", "1"}, publisherOptions, "guiseppe", m_payloadDataSegmentMemoryManager, PortConfigInfo())
            .value());
    publisher.offer();
    m_portManager->doDiscovery();

    SubscriberPortUser subscriber(
        m_portManager->acquireSubscriberPortData({"1", "1", "1"}, subscriberOptions, "schlomo", PortConfigInfo())
            .value());

    ASSERT_TRUE(publisher.hasSubscribers());
    EXPECT_THAT(subscriber.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(PortManager_test, OfferOnCreateSubscribesWithoutDiscoveryLoopWhenSubscriberAvailable)
{
    ::testing::Test::RecordProperty("TEST_ID", "f6cb4274-d137-4035-b38c-3644b8158006");
    PublisherOptions publisherOptions{1U, iox::NodeName_t("node"), true};
    SubscriberOptions subscriberOptions{1U, 1U, iox::NodeName_t("node"), false};
    SubscriberPortUser subscriber(
        m_portManager->acquireSubscriberPortData({"1", "1", "1"}, subscriberOptions, "schlomo", PortConfigInfo())
            .value());
    subscriber.subscribe();
    m_portManager->doDiscovery();

    PublisherPortUser publisher(
        m_portManager
            ->acquirePublisherPortData(
                {"1", "1", "1"}, publisherOptions, "guiseppe", m_payloadDataSegmentMemoryManager, PortConfigInfo())
            .value());

    ASSERT_TRUE(publisher.hasSubscribers());
    EXPECT_THAT(subscriber.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(PortManager_test, OfferOnCreateAndSubscribeOnCreateNeedsNoMoreDiscoveryLoopSubscriberFirst)
{
    ::testing::Test::RecordProperty("TEST_ID", "28798301-1630-458a-81b5-77f53e75d0fb");
    PublisherOptions publisherOptions{1U, iox::NodeName_t("node"), true};
    SubscriberOptions subscriberOptions{1U, 1U, iox::NodeName_t("node"), true};
    SubscriberPortUser subscriber(
        m_portManager->acquireSubscriberPortData({"1", "1", "1"}, subscriberOptions, "schlomo", PortConfigInfo())
            .value());

    PublisherPortUser publisher(
        m_portManager
            ->acquirePublisherPortData(
                {"1", "1", "1"}, publisherOptions, "guiseppe", m_payloadDataSegmentMemoryManager, PortConfigInfo())
            .value());

    ASSERT_TRUE(publisher.hasSubscribers());
    EXPECT_THAT(subscriber.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(PortManager_test, OfferOnCreateAndSubscribeOnCreateNeedsNoMoreDiscoveryLoopPublisherFirst)
{
    ::testing::Test::RecordProperty("TEST_ID", "8a51d940-fc35-428e-b4dd-b19c7e9d1f4a");
    PublisherOptions publisherOptions{1U, iox::NodeName_t("node"), true};
    SubscriberOptions subscriberOptions{1U, 1U, iox::NodeName_t("node"), true};
    PublisherPortUser publisher(
        m_portManager
            ->acquirePublisherPortData(
                {"1", "1", "1"}, publisherOptions, "guiseppe", m_payloadDataSegmentMemoryManager, PortConfigInfo())
            .value());

    SubscriberPortUser subscriber(
        m_portManager->acquireSubscriberPortData({"1", "1", "1"}, subscriberOptions, "schlomo", PortConfigInfo())
            .value());


    ASSERT_TRUE(publisher.hasSubscribers());
    EXPECT_THAT(subscriber.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
}


TEST_F(PortManager_test, AcquiringOneMoreThanMaximumNumberOfPublishersFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "617abda0-36f7-4f98-9eb9-572622e0ffa1");
    iox::RuntimeName_t runtimeName = "test1";
    PublisherOptions publisherOptions{1U, iox::NodeName_t("run1")};

    for (unsigned int i = 0; i < iox::MAX_PUBLISHERS; i++)
    {
        auto publisherPortDataResult = m_portManager->acquirePublisherPortData(
            getUniqueSD(), publisherOptions, runtimeName, m_payloadDataSegmentMemoryManager, PortConfigInfo());

        ASSERT_FALSE(publisherPortDataResult.has_error());
    }

    { // test if overflow errors get hit
        IOX_TESTING_EXPECT_OK();

        auto publisherPortDataResult = m_portManager->acquirePublisherPortData(
            getUniqueSD(), publisherOptions, runtimeName, m_payloadDataSegmentMemoryManager, PortConfigInfo());
        IOX_TESTING_EXPECT_ERROR(iox::PoshError::PORT_POOL__PUBLISHERLIST_OVERFLOW);
        ASSERT_TRUE(publisherPortDataResult.has_error());
        EXPECT_THAT(publisherPortDataResult.error(), Eq(PortPoolError::PUBLISHER_PORT_LIST_FULL));
    }
}

TEST_F(PortManager_test, AcquiringPublisherAsUserWithAnyInternalServiceDescriptionFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "c902d189-de40-4ecd-9596-bdd8f03e2837");

    const iox::RuntimeName_t runtimeName = "foobar";
    addInternalPublisherOfPortManagerToVector();

    for (auto& service : internalServices)
    {
        auto publisherPortDataResult = m_portManager->acquirePublisherPortData(
            service, iox::popo::PublisherOptions(), runtimeName, m_payloadDataSegmentMemoryManager, PortConfigInfo());
        ASSERT_TRUE(publisherPortDataResult.has_error());
        EXPECT_THAT(publisherPortDataResult.error(), Eq(PortPoolError::INTERNAL_SERVICE_DESCRIPTION_IS_FORBIDDEN));
    }
}

TEST_F(PortManager_test, AcquiringPublisherAsRoudiWithAnyInternalServiceDescriptionIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "500ba79a-a026-4e67-b6c0-550e3b585521");

    addInternalPublisherOfPortManagerToVector();

    for (auto& service : internalServices)
    {
        auto publisherPortDataResult = m_portManager->acquireInternalPublisherPortData(
            service, iox::popo::PublisherOptions(), m_payloadDataSegmentMemoryManager);
        EXPECT_THAT(publisherPortDataResult, Ne(nullptr));
    }
}

constexpr bool IS_COMMUNICATION_POLICY_ONE_TO_MANY_ONLY{
    std::is_same<iox::build::CommunicationPolicy, iox::build::OneToManyPolicy>::value};

TEST_F(PortManager_test, AcquirePublisherPortDataWithSameServiceDescriptionTwiceWorksAccordingCommunicationPolicy)
{
    ::testing::Test::RecordProperty("TEST_ID", "6b26220c-01a3-4f3a-8af4-06c66d6f98ef");
    const iox::capro::ServiceDescription sd{"hyp", "no", "toad"};
    const iox::RuntimeName_t runtimeName{"hypnotoad"};
    auto publisherOptions = createTestPubOptions();

    // first call must be successful
    m_portManager->acquirePublisherPortData(sd, publisherOptions, runtimeName, m_payloadDataSegmentMemoryManager, {})
        .or_else([&](const auto& error) {
            GTEST_FAIL() << "Expected ClientPortData but got PortPoolError: " << static_cast<uint8_t>(error);
        });

    // second call
    auto acquirePublisherPortResult = m_portManager->acquirePublisherPortData(
        sd, publisherOptions, runtimeName, m_payloadDataSegmentMemoryManager, {});

    if (IS_COMMUNICATION_POLICY_ONE_TO_MANY_ONLY)
    {
        ASSERT_TRUE(acquirePublisherPortResult.has_error());
        EXPECT_THAT(acquirePublisherPortResult.error(), Eq(PortPoolError::UNIQUE_PUBLISHER_PORT_ALREADY_EXISTS));
        IOX_TESTING_EXPECT_ERROR(iox::PoshError::POSH__PORT_MANAGER_PUBLISHERPORT_NOT_UNIQUE);
    }
    else
    {
        IOX_TESTING_EXPECT_OK();
    }
}

TEST_F(PortManager_test,
       AcquirePublisherPortDataWithSameServiceDescriptionTwiceAndFirstPortMarkedToBeDestroyedReturnsPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "0840c279-5429-4c93-a120-738735a89100");
    const iox::capro::ServiceDescription sd{"hyp", "no", "toad"};
    const iox::RuntimeName_t runtimeName{"hypnotoad"};
    auto publisherOptions = createTestPubOptions();

    // first call must be successful
    auto publisherPortDataResult = m_portManager->acquirePublisherPortData(
        sd, publisherOptions, runtimeName, m_payloadDataSegmentMemoryManager, {});

    ASSERT_FALSE(publisherPortDataResult.has_error());

    publisherPortDataResult.value()->m_toBeDestroyed = true;

    // second call must now also succeed
    m_portManager->acquirePublisherPortData(sd, publisherOptions, runtimeName, m_payloadDataSegmentMemoryManager, {})
        .or_else([&](const auto& error) {
            GTEST_FAIL() << "Expected ClientPortData but got PortPoolError: " << static_cast<uint8_t>(error);
        });

    IOX_TESTING_EXPECT_OK();
}

TEST_F(PortManager_test, AcquiringOneMoreThanMaximumNumberOfSubscribersFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "5039eff5-f2d1-4f58-8bd2-fc9768a9bc92");
    iox::RuntimeName_t runtimeName1 = "test1";
    SubscriberOptions subscriberOptions{1U, 1U, iox::NodeName_t("run1")};

    for (unsigned int i = 0; i < iox::MAX_SUBSCRIBERS; i++)
    {
        auto subscriberPortDataResult =
            m_portManager->acquireSubscriberPortData(getUniqueSD(), subscriberOptions, runtimeName1, PortConfigInfo());
        ASSERT_FALSE(subscriberPortDataResult.has_error());
    }

    { // test if overflow errors get hit
        IOX_TESTING_EXPECT_OK();

        auto subscriberPortDataResult =
            m_portManager->acquireSubscriberPortData(getUniqueSD(), subscriberOptions, runtimeName1, PortConfigInfo());
        ASSERT_TRUE(subscriberPortDataResult.has_error());
        EXPECT_THAT(subscriberPortDataResult.error(), Eq(PortPoolError::SUBSCRIBER_PORT_LIST_FULL));

        IOX_TESTING_EXPECT_ERROR(iox::PoshError::PORT_POOL__SUBSCRIBERLIST_OVERFLOW);
    }
}

TEST_F(PortManager_test, AcquiringOneMoreThanMaximumNumberOfInterfacesFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "129706cc-18e5-4457-b314-d6b7ae347ea0");
    std::string runtimeName = "itf";

    // first aquire all possible Interfaces
    acquireMaxNumberOfInterfaces(runtimeName);

    // test if overflow errors get hit
    {
        IOX_TESTING_EXPECT_OK();

        auto interfacePort = m_portManager->acquireInterfacePortData(iox::capro::Interfaces::INTERNAL, "itfPenguin");
        EXPECT_EQ(interfacePort, nullptr);

        IOX_TESTING_EXPECT_ERROR(iox::PoshError::PORT_POOL__INTERFACELIST_OVERFLOW);
    }
}

TEST_F(PortManager_test, DoDiscoveryPublisherCanWaitAndSubscriberRequestsBlockingLeadsToConnect)
{
    ::testing::Test::RecordProperty("TEST_ID", "34380b13-5541-4fdf-b266-beccb90f5215");
    PublisherOptions publisherOptions{
        1U, iox::NodeName_t("node"), true, iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER};
    SubscriberOptions subscriberOptions{1U, 1U, iox::NodeName_t("node"), true, QueueFullPolicy::BLOCK_PRODUCER};
    PublisherPortUser publisher(
        m_portManager
            ->acquirePublisherPortData(
                {"1", "1", "1"}, publisherOptions, "guiseppe", m_payloadDataSegmentMemoryManager, PortConfigInfo())
            .value());
    ASSERT_TRUE(publisher);
    SubscriberPortUser subscriber(
        m_portManager->acquireSubscriberPortData({"1", "1", "1"}, subscriberOptions, "schlomo", PortConfigInfo())
            .value());
    ASSERT_TRUE(subscriber);

    ASSERT_TRUE(publisher.hasSubscribers());
    EXPECT_THAT(subscriber.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(PortManager_test, DoDiscoveryBothDiscardOldestPolicyLeadsToConnect)
{
    ::testing::Test::RecordProperty("TEST_ID", "3cf03140-9ca6-47a1-b45b-8cfa70e3fd5c");
    PublisherOptions publisherOptions{
        1U, iox::NodeName_t("node"), true, iox::popo::ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA};
    SubscriberOptions subscriberOptions{1U, 1U, iox::NodeName_t("node"), true, QueueFullPolicy::DISCARD_OLDEST_DATA};
    PublisherPortUser publisher(
        m_portManager
            ->acquirePublisherPortData(
                {"1", "1", "1"}, publisherOptions, "guiseppe", m_payloadDataSegmentMemoryManager, PortConfigInfo())
            .value());
    ASSERT_TRUE(publisher);
    SubscriberPortUser subscriber(
        m_portManager->acquireSubscriberPortData({"1", "1", "1"}, subscriberOptions, "schlomo", PortConfigInfo())
            .value());
    ASSERT_TRUE(subscriber);

    ASSERT_TRUE(publisher.hasSubscribers());
    EXPECT_THAT(subscriber.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(PortManager_test, DoDiscoveryPublisherDoesNotAllowBlockingAndSubscriberRequestsBlockingLeadsToNoConnect)
{
    ::testing::Test::RecordProperty("TEST_ID", "31d879bf-ca07-4f29-90cd-a46f09a98f7c");
    PublisherOptions publisherOptions{
        1U, iox::NodeName_t("node"), true, iox::popo::ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA};
    SubscriberOptions subscriberOptions{1U, 1U, iox::NodeName_t("node"), true, QueueFullPolicy::BLOCK_PRODUCER};
    PublisherPortUser publisher(
        m_portManager
            ->acquirePublisherPortData(
                {"1", "1", "1"}, publisherOptions, "guiseppe", m_payloadDataSegmentMemoryManager, PortConfigInfo())
            .value());
    ASSERT_TRUE(publisher);
    SubscriberPortUser subscriber(
        m_portManager->acquireSubscriberPortData({"1", "1", "1"}, subscriberOptions, "schlomo", PortConfigInfo())
            .value());
    ASSERT_TRUE(subscriber);

    ASSERT_FALSE(publisher.hasSubscribers());
}

TEST_F(PortManager_test, DoDiscoveryPublisherCanWaitAndSubscriberDiscardOldestLeadsToConnect)
{
    ::testing::Test::RecordProperty("TEST_ID", "f2ea15a6-0672-4a98-8f80-f2900b247ac0");
    PublisherOptions publisherOptions{
        1U, iox::NodeName_t("node"), true, iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER};
    SubscriberOptions subscriberOptions{1U, 1U, iox::NodeName_t("node"), true, QueueFullPolicy::DISCARD_OLDEST_DATA};
    PublisherPortUser publisher(
        m_portManager
            ->acquirePublisherPortData(
                {"1", "1", "1"}, publisherOptions, "guiseppe", m_payloadDataSegmentMemoryManager, PortConfigInfo())
            .value());
    ASSERT_TRUE(publisher);

    SubscriberPortUser subscriber(
        m_portManager->acquireSubscriberPortData({"1", "1", "1"}, subscriberOptions, "schlomo", PortConfigInfo())
            .value());
    ASSERT_TRUE(subscriber);

    ASSERT_TRUE(publisher.hasSubscribers());
    EXPECT_THAT(subscriber.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
}

TEST_F(PortManager_test, SubscriberRequiringHistorySupportDoesNotConnectToPublisherWithoutHistorySupport)
{
    ::testing::Test::RecordProperty("TEST_ID", "43f3ea1e-777a-4cc6-8478-13f981b7c941");

    auto publisherOptions = createTestPubOptions();
    auto subscriberOptions = createTestSubOptions();

    publisherOptions.historyCapacity = 0;
    subscriberOptions.historyRequest = 1;
    subscriberOptions.requiresPublisherHistorySupport = true;

    auto publisher = createPublisher(publisherOptions);
    auto subscriber = createSubscriber(subscriberOptions);

    ASSERT_TRUE(publisher);
    ASSERT_TRUE(subscriber);
    EXPECT_FALSE(publisher.hasSubscribers());
}

TEST_F(PortManager_test, SubscriberNotRequiringHistorySupportDoesConnectToPublisherWithNoHistorySupport)
{
    ::testing::Test::RecordProperty("TEST_ID", "080a94db-3a89-4d98-94a6-900015e608e2");

    auto publisherOptions = createTestPubOptions();
    auto subscriberOptions = createTestSubOptions();

    publisherOptions.historyCapacity = 0;
    subscriberOptions.historyRequest = 1;
    subscriberOptions.requiresPublisherHistorySupport = false;

    auto publisher = createPublisher(publisherOptions);
    auto subscriber = createSubscriber(subscriberOptions);

    ASSERT_TRUE(publisher);
    ASSERT_TRUE(subscriber);
    EXPECT_TRUE(publisher.hasSubscribers());
}

TEST_F(PortManager_test, SubscriberRequiringHistorySupportDoesConnectToPublisherWithEqualHistorySupport)
{
    ::testing::Test::RecordProperty("TEST_ID", "20749a22-2771-4ec3-92f8-81bbdbd4aab6");

    auto publisherOptions = createTestPubOptions();
    auto subscriberOptions = createTestSubOptions();

    publisherOptions.historyCapacity = 3;
    subscriberOptions.historyRequest = 3;
    subscriberOptions.requiresPublisherHistorySupport = true;

    auto publisher = createPublisher(publisherOptions);
    auto subscriber = createSubscriber(subscriberOptions);

    ASSERT_TRUE(publisher);
    ASSERT_TRUE(subscriber);
    EXPECT_TRUE(publisher.hasSubscribers());
}

TEST_F(PortManager_test, SubscriberRequiringHistorySupportDoesConnectToPublisherWithLowerHistorySupport)
{
    ::testing::Test::RecordProperty("TEST_ID", "e2567667-4583-482b-9999-029f91c0cb71");

    auto publisherOptions = createTestPubOptions();
    auto subscriberOptions = createTestSubOptions();

    publisherOptions.historyCapacity = 5;
    subscriberOptions.historyRequest = 6;
    subscriberOptions.requiresPublisherHistorySupport = true;

    auto publisher = createPublisher(publisherOptions);
    auto subscriber = createSubscriber(subscriberOptions);

    ASSERT_TRUE(publisher);
    ASSERT_TRUE(subscriber);
    EXPECT_TRUE(publisher.hasSubscribers());
}

TEST_F(PortManager_test, SubscriberNotRequiringHistorySupportDoesConnectToPublisherWithLowerHistorySupport)
{
    ::testing::Test::RecordProperty("TEST_ID", "e6c7cee4-cb4a-4a14-8790-4dbfce7d8584");

    auto publisherOptions = createTestPubOptions();
    auto subscriberOptions = createTestSubOptions();

    publisherOptions.historyCapacity = 2;
    subscriberOptions.historyRequest = 3;
    subscriberOptions.requiresPublisherHistorySupport = false;

    auto publisher = createPublisher(publisherOptions);
    auto subscriber = createSubscriber(subscriberOptions);

    ASSERT_TRUE(publisher);
    ASSERT_TRUE(subscriber);
    EXPECT_TRUE(publisher.hasSubscribers());
}

TEST_F(PortManager_test, DeleteInterfacePortfromMaximumNumberAndAddOneIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "2e682da4-aea0-4c37-8895-4049506db936");
    std::string runtimeName = "itf";

    // first aquire all possible Interfaces
    acquireMaxNumberOfInterfaces(runtimeName);

    // delete one and add one should be possible now
    {
        unsigned int testi = 0;
        auto newProcessName = runtimeName + iox::convert::toString(testi);
        // this is done because there is no removeInterfaceData method in the PortManager class
        m_portManager->deletePortsOfProcess(into<lossy<iox::RuntimeName_t>>(newProcessName));

        auto interfacePort = m_portManager->acquireInterfacePortData(iox::capro::Interfaces::INTERNAL,
                                                                     into<lossy<iox::RuntimeName_t>>(newProcessName));
        EXPECT_NE(interfacePort, nullptr);
    }
}

TEST_F(PortManager_test, AcquireInterfacePortDataAfterDestroyingPreviouslyAcquiredOnesIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "0a8a52c8-2c6f-44d3-ab32-0d92f6e285f1");
    std::vector<iox::popo::InterfacePortData*> interfaceContainer;
    std::string runtimeName = "itf";

    // first aquire all possible interfaces
    acquireMaxNumberOfInterfaces(runtimeName, [&](auto interafcePort) { interfaceContainer.push_back(interafcePort); });

    // set the destroy flag and let the discovery loop take care
    setDestroyFlagAndClearContainer(interfaceContainer);
    m_portManager->doDiscovery();

    // so we should able to get some more now
    acquireMaxNumberOfInterfaces(runtimeName);
}

TEST_F(PortManager_test, AcquiringOneMoreThanMaximumNumberOfConditionVariablesFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "5b7f4106-cf38-4c89-9453-fca1e3887ee5");
    std::string runtimeName = "HypnoToadForEver";

    // first aquire all possible condition variables
    acquireMaxNumberOfConditionVariables(runtimeName);

    // test if overflow errors get hit
    {
        IOX_TESTING_EXPECT_OK();

        auto conditionVariableResult = m_portManager->acquireConditionVariableData("AnotherToad");
        ASSERT_TRUE(conditionVariableResult.has_error());
        EXPECT_THAT(conditionVariableResult.error(), Eq(PortPoolError::CONDITION_VARIABLE_LIST_FULL));

        IOX_TESTING_EXPECT_ERROR(iox::PoshError::PORT_POOL__CONDITION_VARIABLE_LIST_OVERFLOW);
    }
}

TEST_F(PortManager_test, DeleteConditionVariablePortfromMaximumNumberAndAddOneIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "81de0f58-7bf8-43c4-a4ef-b365c0e74c47");
    std::string runtimeName = "HypnoToadForEver";

    // first aquire all possible condition variables
    acquireMaxNumberOfConditionVariables(runtimeName);

    // delete one and add one should be possible now
    {
        unsigned int testi = 0;
        auto newProcessName = runtimeName + iox::convert::toString(testi);
        // this is done because there is no removeConditionVariableData method in the PortManager class
        m_portManager->deletePortsOfProcess(into<lossy<iox::RuntimeName_t>>(newProcessName));

        auto conditionVariableResult =
            m_portManager->acquireConditionVariableData(into<lossy<iox::RuntimeName_t>>(newProcessName));
        EXPECT_FALSE(conditionVariableResult.has_error());
    }
}

TEST_F(PortManager_test, AcquireConditionVariablesDataAfterDestroyingPreviouslyAcquiredOnesIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "5b82b069-b300-4018-88d6-83cca8157066");
    std::vector<iox::popo::ConditionVariableData*> condVarContainer;

    std::string runtimeName = "HypnoToadForEver";

    // first aquire all possible condition variables
    acquireMaxNumberOfConditionVariables(runtimeName, [&](auto condVar) { condVarContainer.push_back(condVar); });

    // set the destroy flag and let the discovery loop take care
    setDestroyFlagAndClearContainer(condVarContainer);
    m_portManager->doDiscovery();

    // so we should able to get some more now
    acquireMaxNumberOfConditionVariables(runtimeName);
}

TEST_F(PortManager_test, UnblockRouDiShutdownMakesAllPublisherStopOffer)
{
    ::testing::Test::RecordProperty("TEST_ID", "aa0cd25c-4e9d-476a-a8a6-d5c650fb9fff");
    PublisherOptions publisherOptions{1U, iox::NodeName_t("node"), true};
    iox::vector<PublisherPortUser, iox::MAX_PUBLISHERS> publisher;

    for (unsigned int i = 0; i < iox::MAX_PUBLISHERS; i++)
    {
        auto servideDescription = getUniqueSD();
        auto publisherRuntimeName = into<lossy<iox::RuntimeName_t>>("pub_" + std::to_string(i));
        auto publisherPortDataResult = m_portManager->acquirePublisherPortData(servideDescription,
                                                                               publisherOptions,
                                                                               publisherRuntimeName,
                                                                               m_payloadDataSegmentMemoryManager,
                                                                               PortConfigInfo());
        ASSERT_FALSE(publisherPortDataResult.has_error());
        publisher.emplace_back(publisherPortDataResult.value());

        EXPECT_TRUE(publisher.back().isOffered());
    }

    m_portManager->unblockRouDiShutdown();

    for (const auto& pub : publisher)
    {
        EXPECT_FALSE(pub.isOffered());
    }
}

TEST_F(PortManager_test, UnblockProcessShutdownMakesPublisherStopOffer)
{
    ::testing::Test::RecordProperty("TEST_ID", "a013e699-2a6b-42e9-b6e8-5f65ea235c0a");
    const iox::RuntimeName_t publisherRuntimeName{"guiseppe"};

    // get publisher and subscriber
    PublisherOptions publisherOptions{
        0U, iox::NodeName_t("node"), true, iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER};
    PublisherPortUser publisher(m_portManager
                                    ->acquirePublisherPortData({"1", "1", "1"},
                                                               publisherOptions,
                                                               publisherRuntimeName,
                                                               m_payloadDataSegmentMemoryManager,
                                                               PortConfigInfo())
                                    .value());

    EXPECT_TRUE(publisher.isOffered());

    m_portManager->unblockProcessShutdown(publisherRuntimeName);

    EXPECT_FALSE(publisher.isOffered());
}

void PortManager_test::setupAndTestBlockingPublisher(const iox::RuntimeName_t& publisherRuntimeName,
                                                     std::function<void()> testHook) noexcept
{
    // get publisher and subscriber
    PublisherOptions publisherOptions{
        0U, iox::NodeName_t("node"), true, iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER};
    SubscriberOptions subscriberOptions{
        1U, 0U, iox::NodeName_t("node"), true, iox::popo::QueueFullPolicy::BLOCK_PRODUCER};
    PublisherPortUser publisher(m_portManager
                                    ->acquirePublisherPortData({"1", "1", "1"},
                                                               publisherOptions,
                                                               publisherRuntimeName,
                                                               m_payloadDataSegmentMemoryManager,
                                                               PortConfigInfo())
                                    .value());

    SubscriberPortUser subscriber(
        m_portManager->acquireSubscriberPortData({"1", "1", "1"}, subscriberOptions, "schlomo", PortConfigInfo())
            .value());

    ASSERT_TRUE(publisher.hasSubscribers());
    ASSERT_THAT(subscriber.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));

    // send chunk to fill subscriber queue
    auto maybeChunk = publisher.tryAllocateChunk(42U, 8U);
    ASSERT_FALSE(maybeChunk.has_error());
    publisher.sendChunk(maybeChunk.value());

    iox::concurrent::Atomic<bool> wasChunkSent{false};

    constexpr iox::units::Duration DEADLOCK_TIMEOUT{5_s};
    Watchdog deadlockWatchdog{DEADLOCK_TIMEOUT};
    deadlockWatchdog.watchAndActOnFailure([] { std::terminate(); });

    // block in a separate thread
    Barrier isThreadStarted(1U);
    std::thread blockingPublisher([&] {
        auto maybeChunk = publisher.tryAllocateChunk(42U, 8U);
        ASSERT_FALSE(maybeChunk.has_error());
        isThreadStarted.notify();
        publisher.sendChunk(maybeChunk.value());
        wasChunkSent = true;
    });

    // wait some time to check if the publisher is blocked
    constexpr int64_t SLEEP_IN_MS = 100;
    isThreadStarted.wait();
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_IN_MS));
    EXPECT_THAT(wasChunkSent.load(), Eq(false));

    ASSERT_TRUE(testHook);
    testHook();

    blockingPublisher.join(); // ensure the wasChunkSent store happens before the read
    EXPECT_THAT(wasChunkSent.load(), Eq(true));
}

TEST_F(PortManager_test, UnblockRouDiShutdownUnblocksBlockedPublisher)
{
    ::testing::Test::RecordProperty("TEST_ID", "5200e46f-5006-4a71-966a-a4aff8a0bf85");
    const iox::RuntimeName_t publisherRuntimeName{"guiseppe"};
    setupAndTestBlockingPublisher(publisherRuntimeName, [&] { m_portManager->unblockRouDiShutdown(); });
}

TEST_F(PortManager_test, UnblockProcessShutdownUnblocksBlockedPublisher)
{
    ::testing::Test::RecordProperty("TEST_ID", "2c5a3f87-20fb-4fd8-a3b5-033b79598800");
    const iox::RuntimeName_t publisherRuntimeName{"guiseppe"};
    setupAndTestBlockingPublisher(publisherRuntimeName,
                                  [&] { m_portManager->unblockProcessShutdown(publisherRuntimeName); });
}

TEST_F(PortManager_test, PortsDestroyInProcess2ChangeStatesOfPortsInProcess1)
{
    ::testing::Test::RecordProperty("TEST_ID", "65815512-0298-46b7-9d19-64bc51079c1a");
    iox::RuntimeName_t runtimeName1 = "myApp1";
    iox::RuntimeName_t runtimeName2 = "myApp2";
    iox::capro::ServiceDescription cap1("1", "1", "1");
    iox::capro::ServiceDescription cap2("2", "2", "2");
    PublisherOptions publisherOptions{1U, iox::NodeName_t("node"), false};
    SubscriberOptions subscriberOptions{1U, 1U, iox::NodeName_t("node"), false};

    // two applications app1 and app2 each with a publisher and subscriber that match to the other applications
    auto publisherData1 =
        m_portManager
            ->acquirePublisherPortData(
                cap1, publisherOptions, runtimeName1, m_payloadDataSegmentMemoryManager, PortConfigInfo())
            .value();
    auto subscriberData1 =
        m_portManager->acquireSubscriberPortData(cap2, subscriberOptions, runtimeName1, PortConfigInfo()).value();

    auto publisherData2 =
        m_portManager
            ->acquirePublisherPortData(
                cap2, publisherOptions, runtimeName2, m_payloadDataSegmentMemoryManager, PortConfigInfo())
            .value();
    auto subscriberData2 =
        m_portManager->acquireSubscriberPortData(cap1, subscriberOptions, runtimeName2, PortConfigInfo()).value();

    // let them connect
    {
        PublisherPortUser publisher1(publisherData1);
        ASSERT_TRUE(publisher1);
        publisher1.offer();
        SubscriberPortUser subscriber1(subscriberData1);
        ASSERT_TRUE(subscriber1);
        subscriber1.subscribe();

        PublisherPortUser publisher2(publisherData2);
        ASSERT_TRUE(publisher2);
        publisher2.offer();
        SubscriberPortUser subscriber2(subscriberData2);
        ASSERT_TRUE(subscriber2);
        subscriber2.subscribe();

        m_portManager->doDiscovery();

        ASSERT_TRUE(publisher1.hasSubscribers());
        ASSERT_TRUE(publisher2.hasSubscribers());
        EXPECT_THAT(subscriber1.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
        EXPECT_THAT(subscriber2.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
    }

    // destroy the ports of process2 and check if states of ports in process1 changed as expected
    {
        PublisherPortUser publisher1(publisherData1);
        ASSERT_TRUE(publisher1);
        SubscriberPortUser subscriber1(subscriberData1);
        ASSERT_TRUE(subscriber1);

        PublisherPortUser publisher2(publisherData2);
        ASSERT_TRUE(publisher2);
        publisher2.destroy();
        SubscriberPortUser subscriber2(subscriberData2);
        ASSERT_TRUE(subscriber2);
        subscriber2.destroy();

        m_portManager->doDiscovery();

        ASSERT_FALSE(publisher1.hasSubscribers());
        if (std::is_same<iox::build::CommunicationPolicy, iox::build::OneToManyPolicy>::value)
        {
            EXPECT_THAT(subscriber1.getSubscriptionState(), Eq(iox::SubscribeState::WAIT_FOR_OFFER));
        }
    }

    // re-create the ports of process runtimeName2
    publisherData2 = m_portManager
                         ->acquirePublisherPortData(
                             cap2, publisherOptions, runtimeName2, m_payloadDataSegmentMemoryManager, PortConfigInfo())
                         .value();
    subscriberData2 =
        m_portManager->acquireSubscriberPortData(cap1, subscriberOptions, runtimeName2, PortConfigInfo()).value();

    // let them connect
    {
        PublisherPortUser publisher1(publisherData1);
        ASSERT_TRUE(publisher1);
        SubscriberPortUser subscriber1(subscriberData1);
        ASSERT_TRUE(subscriber1);

        PublisherPortUser publisher2(publisherData2);
        ASSERT_TRUE(publisher2);
        publisher2.offer();
        SubscriberPortUser subscriber2(subscriberData2);
        ASSERT_TRUE(subscriber2);
        subscriber2.subscribe();

        m_portManager->doDiscovery();

        ASSERT_TRUE(publisher1.hasSubscribers());
        ASSERT_TRUE(publisher2.hasSubscribers());
        EXPECT_THAT(subscriber1.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
        EXPECT_THAT(subscriber2.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));
    }

    // cleanup process process2 and check if states of ports in process1 changed  as expected
    {
        m_portManager->deletePortsOfProcess(runtimeName2);
        PublisherPortUser publisher1(publisherData1);
        ASSERT_TRUE(publisher1);
        SubscriberPortUser subscriber1(subscriberData1);
        ASSERT_TRUE(subscriber1);

        ASSERT_FALSE(publisher1.hasSubscribers());
        if (std::is_same<iox::build::CommunicationPolicy, iox::build::OneToManyPolicy>::value)
        {
            EXPECT_THAT(subscriber1.getSubscriptionState(), Eq(iox::SubscribeState::WAIT_FOR_OFFER));
        }
    }
}

} // namespace iox_test_roudi_portmanager
