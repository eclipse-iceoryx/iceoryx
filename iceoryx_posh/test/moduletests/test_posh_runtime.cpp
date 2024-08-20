// Copyright (c) 2020 - 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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
#include "iceoryx_hoofs/testing/timing_test.hpp"
#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/popo/untyped_client.hpp"
#include "iceoryx_posh/popo/untyped_server.hpp"
#include "iceoryx_posh/roudi_env/minimal_iceoryx_config.hpp"
#include "iceoryx_posh/roudi_env/roudi_env.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/testing/mocks/posh_runtime_mock.hpp"
#include "iox/atomic.hpp"
#include "iox/detail/convert.hpp"
#include "iox/std_string_support.hpp"

#include "iceoryx_hoofs/testing/error_reporting/testing_support.hpp"
#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "test.hpp"

#include <type_traits>

namespace
{
using namespace ::testing;
using namespace iox::runtime;
using namespace iox::capro;
using namespace iox;
using namespace iox::testing;
using namespace iox::popo;
using namespace iox::roudi_env;

class PoshRuntime_test : public Test
{
  public:
    PoshRuntime_test()
    {
    }

    virtual ~PoshRuntime_test()
    {
    }

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    void checkClientInitialization(const ClientPortData* const portData,
                                   const ServiceDescription& sd,
                                   const ClientOptions& options,
                                   const iox::mepoo::MemoryInfo& memoryInfo) const
    {
        ASSERT_THAT(portData, Ne(nullptr));

        EXPECT_EQ(portData->m_serviceDescription, sd);
        EXPECT_EQ(portData->m_runtimeName, m_runtimeName);
        EXPECT_EQ(portData->m_connectRequested.load(), options.connectOnCreate);
        EXPECT_EQ(portData->m_chunkReceiverData.m_queue.capacity(), options.responseQueueCapacity);
        EXPECT_EQ(portData->m_chunkReceiverData.m_queueFullPolicy, options.responseQueueFullPolicy);
        EXPECT_EQ(portData->m_chunkReceiverData.m_memoryInfo.deviceId, memoryInfo.deviceId);
        EXPECT_EQ(portData->m_chunkReceiverData.m_memoryInfo.memoryType, memoryInfo.memoryType);
        EXPECT_EQ(portData->m_chunkSenderData.m_historyCapacity, iox::popo::ClientPortData::HISTORY_CAPACITY_ZERO);
        EXPECT_EQ(portData->m_chunkSenderData.m_consumerTooSlowPolicy, options.serverTooSlowPolicy);
        EXPECT_EQ(portData->m_chunkSenderData.m_memoryInfo.deviceId, memoryInfo.deviceId);
        EXPECT_EQ(portData->m_chunkSenderData.m_memoryInfo.memoryType, memoryInfo.memoryType);
    }

    void checkServerInitialization(const ServerPortData* const portData,
                                   const ServiceDescription& sd,
                                   const ServerOptions& options,
                                   const iox::mepoo::MemoryInfo& memoryInfo) const
    {
        ASSERT_THAT(portData, Ne(nullptr));

        EXPECT_EQ(portData->m_serviceDescription, sd);
        EXPECT_EQ(portData->m_runtimeName, m_runtimeName);
        EXPECT_EQ(portData->m_offeringRequested.load(), options.offerOnCreate);
        EXPECT_EQ(portData->m_chunkReceiverData.m_queue.capacity(), options.requestQueueCapacity);
        EXPECT_EQ(portData->m_chunkReceiverData.m_queueFullPolicy, options.requestQueueFullPolicy);
        EXPECT_EQ(portData->m_chunkReceiverData.m_memoryInfo.deviceId, memoryInfo.deviceId);
        EXPECT_EQ(portData->m_chunkReceiverData.m_memoryInfo.memoryType, memoryInfo.memoryType);
        EXPECT_EQ(portData->m_chunkSenderData.m_historyCapacity, iox::popo::ServerPortData::HISTORY_REQUEST_OF_ZERO);
        EXPECT_EQ(portData->m_chunkSenderData.m_consumerTooSlowPolicy, options.clientTooSlowPolicy);
        EXPECT_EQ(portData->m_chunkSenderData.m_memoryInfo.deviceId, memoryInfo.deviceId);
        EXPECT_EQ(portData->m_chunkSenderData.m_memoryInfo.memoryType, memoryInfo.memoryType);
    }

    const iox::RuntimeName_t m_runtimeName{"publisher"};
    RouDiEnv m_roudiEnv;
    PoshRuntime* m_runtime{&iox::runtime::PoshRuntime::initRuntime(m_runtimeName)};
    IpcMessage m_sendBuffer;
    IpcMessage m_receiveBuffer;
    const iox::NodeName_t m_nodeName{"testNode"};
    const iox::NodeName_t m_invalidNodeName{"invalidNode,"};
};

TEST_F(PoshRuntime_test, ValidAppName)
{
    ::testing::Test::RecordProperty("TEST_ID", "2f4f5dc1-dde0-4520-a341-79a5edd19900");
    iox::RuntimeName_t appName("valid_name");

    IOX_EXPECT_NO_FATAL_FAILURE([&] { PoshRuntime::initRuntime(appName); });
}

TEST_F(PoshRuntime_test, MaxAppNameLength)
{
    ::testing::Test::RecordProperty("TEST_ID", "dfdf3ce1-c7d4-4c57-94ea-6ed9479371e3");

    std::string maxValidName(iox::MAX_RUNTIME_NAME_LENGTH, 's');
    auto& runtime = PoshRuntime::initRuntime(into<lossy<RuntimeName_t>>(maxValidName));

    EXPECT_THAT(maxValidName, StrEq(runtime.getInstanceName().c_str()));
}

TEST_F(PoshRuntime_test, NoAppName)
{
    ::testing::Test::RecordProperty("TEST_ID", "e053d114-c79c-4391-91e1-8fcfe90ee8e4");
    const iox::RuntimeName_t invalidAppName("");

    GTEST_FLAG(death_test_style) = "threadsafe";
    EXPECT_DEATH({ PoshRuntime::initRuntime(invalidAppName); }, "");
}

// To be able to test the singleton and avoid return the exisiting instance, we don't use the test fixture
TEST(PoshRuntime, RuntimeFailsWhenAppNameIsNotAFileName)
{
    ::testing::Test::RecordProperty("TEST_ID", "77542d11-6230-4c1e-94b2-6cf3b8fa9c6e");

    for (auto i : {"/miau",
                   "/fuu/bar",
                   "plum/bus",
                   ".",
                   "..",
                   "strawberriesWithMayonnaiseIs/..",
                   "ohLookADot.",
                   "amIADirectory/",
                   "",
                   "letsFlyInto "})
    {
        const iox::RuntimeName_t invalidAppName(iox::TruncateToCapacity, i);

        GTEST_FLAG(death_test_style) = "threadsafe";
        EXPECT_DEATH(PoshRuntime::initRuntime(invalidAppName), ".*");
    }
}

// since getInstance is a singleton and test class creates instance of Poshruntime
// when getInstance() is called without parameter, it returns existing instance
// To be able to test this, we don't use the test fixture
TEST(PoshRuntime, AppNameEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "63900656-4fbb-466d-b6cc-f2139121092c");

    GTEST_FLAG(death_test_style) = "threadsafe";
    EXPECT_DEATH({ iox::runtime::PoshRuntime::getInstance(); }, ".*");
}


TEST_F(PoshRuntime_test, GetInstanceNameIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "b82d419c-2c72-43b0-9eb1-b24bb41366ce");
    const iox::RuntimeName_t appname = "app";

    auto& sut = PoshRuntime::initRuntime(appname);

    EXPECT_EQ(sut.getInstanceName(), appname);
}

TEST_F(PoshRuntime_test, GetMiddlewareInterfaceWithInvalidNodeNameIsNotSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "d207e121-d7c2-4a23-a202-1af311f6982b");

    m_runtime->getMiddlewareInterface(iox::capro::Interfaces::INTERNAL, m_invalidNodeName);

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::POSH__RUNTIME_ROUDI_GET_MW_INTERFACE_INVALID_RESPONSE);
}

TEST_F(PoshRuntime_test, GetMiddlewareInterfaceIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "50b1d15d-0cee-41b3-a9cd-146eca553cc2");
    const auto interfacePortData = m_runtime->getMiddlewareInterface(iox::capro::Interfaces::INTERNAL, m_nodeName);

    ASSERT_NE(nullptr, interfacePortData);
    EXPECT_EQ(m_runtimeName, interfacePortData->m_runtimeName);
    EXPECT_EQ(false, interfacePortData->m_toBeDestroyed.load());
}

TEST_F(PoshRuntime_test, GetMiddlewareInterfaceInterfacelistOverflow)
{
    ::testing::Test::RecordProperty("TEST_ID", "0e164d07-dede-46c3-b2a3-ad78a11c0691");

    for (auto i = 0U; i < iox::MAX_INTERFACE_NUMBER; ++i)
    {
        auto interfacePort = m_runtime->getMiddlewareInterface(iox::capro::Interfaces::INTERNAL);
        ASSERT_NE(nullptr, interfacePort);
    }

    IOX_TESTING_EXPECT_OK();

    auto interfacePort = m_runtime->getMiddlewareInterface(iox::capro::Interfaces::INTERNAL);

    EXPECT_EQ(nullptr, interfacePort);

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::PORT_POOL__INTERFACELIST_OVERFLOW);
}


TEST_F(PoshRuntime_test, SendRequestToRouDiValidMessage)
{
    ::testing::Test::RecordProperty("TEST_ID", "334e49d8-e826-4e21-9f9f-bb9c341d4706");
    m_sendBuffer << IpcMessageTypeToString(IpcMessageType::CREATE_INTERFACE) << m_runtimeName
                 << static_cast<uint32_t>(iox::capro::Interfaces::INTERNAL) << m_nodeName;

    const auto successfullySent = m_runtime->sendRequestToRouDi(m_sendBuffer, m_receiveBuffer);

    EXPECT_TRUE(m_receiveBuffer.isValid());
    EXPECT_TRUE(successfullySent);
}


TEST_F(PoshRuntime_test, SendRequestToRouDiInvalidMessage)
{
    ::testing::Test::RecordProperty("TEST_ID", "b3f4563a-7237-4f57-8952-c39ac3dbfef2");
    m_sendBuffer << IpcMessageTypeToString(IpcMessageType::CREATE_INTERFACE) << m_runtimeName
                 << static_cast<uint32_t>(iox::capro::Interfaces::INTERNAL) << m_invalidNodeName;

    const auto successfullySent = m_runtime->sendRequestToRouDi(m_sendBuffer, m_receiveBuffer);

    EXPECT_FALSE(successfullySent);
}

TEST_F(PoshRuntime_test, GetMiddlewarePublisherIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "2cb2e64b-8f21-4049-a35a-dbd7a1d6cbf4");
    iox::popo::PublisherOptions publisherOptions;
    publisherOptions.historyCapacity = 13U;
    publisherOptions.nodeName = m_nodeName;
    const auto publisherPort = m_runtime->getMiddlewarePublisher(
        iox::capro::ServiceDescription("99", "1", "20"), publisherOptions, iox::runtime::PortConfigInfo(11U, 22U, 33U));

    ASSERT_NE(nullptr, publisherPort);
    EXPECT_EQ(iox::capro::ServiceDescription("99", "1", "20"), publisherPort->m_serviceDescription);
    EXPECT_EQ(publisherOptions.historyCapacity, publisherPort->m_chunkSenderData.m_historyCapacity);
}

TEST_F(PoshRuntime_test, GetMiddlewarePublisherWithHistoryGreaterMaxCapacityClampsHistoryToMaximum)
{
    ::testing::Test::RecordProperty("TEST_ID", "407f27bb-e507-4c1c-aab1-e5b1b8d06f46");
    // arrange
    iox::popo::PublisherOptions publisherOptions;
    publisherOptions.historyCapacity = iox::MAX_PUBLISHER_HISTORY + 1U;

    // act
    const auto publisherPort =
        m_runtime->getMiddlewarePublisher(iox::capro::ServiceDescription("99", "1", "20"), publisherOptions);

    // assert
    ASSERT_NE(nullptr, publisherPort);
    EXPECT_EQ(publisherPort->m_chunkSenderData.m_historyCapacity, iox::MAX_PUBLISHER_HISTORY);
}

TEST_F(PoshRuntime_test, getMiddlewarePublisherDefaultArgs)
{
    ::testing::Test::RecordProperty("TEST_ID", "1eae6dfa-c3f2-478b-9354-768c43bd8d96");
    const auto publisherPort = m_runtime->getMiddlewarePublisher(iox::capro::ServiceDescription("99", "1", "20"));

    ASSERT_NE(nullptr, publisherPort);
}


TEST_F(PoshRuntime_test, getMiddlewarePublisherPublisherlistOverflow)
{
    ::testing::Test::RecordProperty("TEST_ID", "f1f1a662-9580-40a1-a116-6ea1cb791516");

    uint32_t i{0U};
    for (; i < (iox::MAX_PUBLISHERS - iox::NUMBER_OF_INTERNAL_PUBLISHERS); ++i)
    {
        auto publisherPort = m_runtime->getMiddlewarePublisher(
            iox::capro::ServiceDescription(into<lossy<RuntimeName_t>>(convert::toString(i)),
                                           into<lossy<RuntimeName_t>>(convert::toString(i + 1U)),
                                           into<lossy<RuntimeName_t>>(convert::toString(i + 2U))));
        ASSERT_NE(nullptr, publisherPort);
    }
    IOX_TESTING_EXPECT_OK();

    auto publisherPort = m_runtime->getMiddlewarePublisher(
        iox::capro::ServiceDescription(into<lossy<RuntimeName_t>>(convert::toString(i)),
                                       into<lossy<RuntimeName_t>>(convert::toString(i + 1U)),
                                       into<lossy<RuntimeName_t>>(convert::toString(i + 2U))));
    EXPECT_EQ(nullptr, publisherPort);

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::PORT_POOL__PUBLISHERLIST_OVERFLOW);
}

TEST_F(PoshRuntime_test, GetMiddlewarePublisherWithSameServiceDescriptionsAndOneToManyPolicyFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "77fb6dfd-a00d-459e-9dd3-90010d7b8af7");

    auto sameServiceDescription = iox::capro::ServiceDescription("99", "1", "20");

    const auto publisherPort1 = m_runtime->getMiddlewarePublisher(
        sameServiceDescription, iox::popo::PublisherOptions(), iox::runtime::PortConfigInfo(11U, 22U, 33U));

    const auto publisherPort2 = m_runtime->getMiddlewarePublisher(
        sameServiceDescription, iox::popo::PublisherOptions(), iox::runtime::PortConfigInfo(11U, 22U, 33U));

    ASSERT_NE(nullptr, publisherPort1);

    if (std::is_same<iox::build::CommunicationPolicy, iox::build::OneToManyPolicy>::value)
    {
        ASSERT_EQ(nullptr, publisherPort2);
        IOX_TESTING_EXPECT_ERROR(iox::PoshError::POSH__RUNTIME_PUBLISHER_PORT_NOT_UNIQUE);
    }
    else if (std::is_same<iox::build::CommunicationPolicy, iox::build::ManyToManyPolicy>::value)
    {
        ASSERT_NE(nullptr, publisherPort2);
    }
}

TEST_F(PoshRuntime_test, GetMiddlewarePublisherWithForbiddenServiceDescriptionsFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "130541c9-94de-4bc4-9471-0a65de310232");

    iox::vector<iox::capro::ServiceDescription, iox::NUMBER_OF_INTERNAL_PUBLISHERS> internalServices;
    const iox::capro::ServiceDescription serviceRegistry{
        iox::SERVICE_DISCOVERY_SERVICE_NAME, iox::SERVICE_DISCOVERY_INSTANCE_NAME, iox::SERVICE_DISCOVERY_EVENT_NAME};

    // Added by PortManager
    internalServices.push_back(serviceRegistry);
    internalServices.push_back(iox::roudi::IntrospectionPortService);
    internalServices.push_back(iox::roudi::IntrospectionPortThroughputService);
    internalServices.push_back(iox::roudi::IntrospectionSubscriberPortChangingDataService);

    // Added by ProcessManager
    internalServices.push_back(iox::roudi::IntrospectionMempoolService);
    internalServices.push_back(iox::roudi::IntrospectionProcessService);

    for (auto& service : internalServices)
    {
        iox::testing::ErrorHandler::instance().reset();
        const auto publisherPort = m_runtime->getMiddlewarePublisher(
            service, iox::popo::PublisherOptions(), iox::runtime::PortConfigInfo(23U, 23U, 16U));
        ASSERT_EQ(nullptr, publisherPort);
        IOX_TESTING_EXPECT_ERROR(iox::PoshError::POSH__RUNTIME_SERVICE_DESCRIPTION_FORBIDDEN);
    }
}

TEST_F(PoshRuntime_test, GetMiddlewarePublisherWithoutOfferOnCreateLeadsToNotOfferedPublisherBeingCreated)
{
    ::testing::Test::RecordProperty("TEST_ID", "5002dc8c-1f6e-4593-a2b3-4de04685c919");
    iox::popo::PublisherOptions publisherOptions;
    publisherOptions.offerOnCreate = false;

    const auto publisherPortData = m_runtime->getMiddlewarePublisher(iox::capro::ServiceDescription("69", "96", "1893"),
                                                                     publisherOptions,
                                                                     iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_FALSE(publisherPortData->m_offeringRequested);
}

TEST_F(PoshRuntime_test, GetMiddlewarePublisherWithOfferOnCreateLeadsToOfferedPublisherBeingCreated)
{
    ::testing::Test::RecordProperty("TEST_ID", "639b1a0e-218d-4cde-a447-e2eec0cf2c75");
    iox::popo::PublisherOptions publisherOptions;
    publisherOptions.offerOnCreate = true;

    const auto publisherPortData = m_runtime->getMiddlewarePublisher(
        iox::capro::ServiceDescription("17", "4", "21"), publisherOptions, iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_TRUE(publisherPortData->m_offeringRequested);
}

TEST_F(PoshRuntime_test, GetMiddlewarePublisherWithoutExplicitlySetQueueFullPolicyLeadsToDiscardOldestData)
{
    ::testing::Test::RecordProperty("TEST_ID", "208418e2-64fd-47f4-b2e2-58aa4371a6a6");
    iox::popo::PublisherOptions publisherOptions;

    const auto publisherPortData = m_runtime->getMiddlewarePublisher(iox::capro::ServiceDescription("9", "13", "1550"),
                                                                     publisherOptions,
                                                                     iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_THAT(publisherPortData->m_chunkSenderData.m_consumerTooSlowPolicy,
                Eq(iox::popo::ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA));
}

TEST_F(PoshRuntime_test, GetMiddlewarePublisherWithQueueFullPolicySetToDiscardOldestDataLeadsToDiscardOldestData)
{
    ::testing::Test::RecordProperty("TEST_ID", "67362686-3165-4a49-a15c-ac9fcaf704d8");
    iox::popo::PublisherOptions publisherOptions;
    publisherOptions.subscriberTooSlowPolicy = iox::popo::ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA;

    const auto publisherPortData =
        m_runtime->getMiddlewarePublisher(iox::capro::ServiceDescription("90", "130", "1550"),
                                          publisherOptions,
                                          iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_THAT(publisherPortData->m_chunkSenderData.m_consumerTooSlowPolicy,
                Eq(iox::popo::ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA));
}

TEST_F(PoshRuntime_test, GetMiddlewarePublisherWithQueueFullPolicySetToWaitForSubscriberLeadsToWaitForSubscriber)
{
    ::testing::Test::RecordProperty("TEST_ID", "f6439a76-69c7-422d-bcc9-7c1d82cd2990");
    iox::popo::PublisherOptions publisherOptions;
    publisherOptions.subscriberTooSlowPolicy = iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;

    const auto publisherPortData = m_runtime->getMiddlewarePublisher(iox::capro::ServiceDescription("18", "31", "400"),
                                                                     publisherOptions,
                                                                     iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_THAT(publisherPortData->m_chunkSenderData.m_consumerTooSlowPolicy,
                Eq(iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER));
}

TEST_F(PoshRuntime_test, GetMiddlewareSubscriberIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "0cc05fe7-752e-4e2a-a8f2-be7cb8b384d2");
    iox::popo::SubscriberOptions subscriberOptions;
    subscriberOptions.historyRequest = 13U;
    subscriberOptions.queueCapacity = 42U;
    subscriberOptions.nodeName = m_nodeName;

    auto subscriberPort = m_runtime->getMiddlewareSubscriber(iox::capro::ServiceDescription("99", "1", "20"),
                                                             subscriberOptions,
                                                             iox::runtime::PortConfigInfo(11U, 22U, 33U));

    ASSERT_NE(nullptr, subscriberPort);
    EXPECT_EQ(iox::capro::ServiceDescription("99", "1", "20"), subscriberPort->m_serviceDescription);
    EXPECT_EQ(subscriberOptions.historyRequest, subscriberPort->m_options.historyRequest);
    EXPECT_EQ(subscriberOptions.queueCapacity, subscriberPort->m_chunkReceiverData.m_queue.capacity());
}

TEST_F(PoshRuntime_test, GetMiddlewareSubscriberWithQueueGreaterMaxCapacityClampsQueueToMaximum)
{
    ::testing::Test::RecordProperty("TEST_ID", "85e2d246-bcba-4ead-a997-4c4137f05607");
    iox::popo::SubscriberOptions subscriberOptions;
    constexpr uint64_t MAX_QUEUE_CAPACITY = iox::popo::SubscriberPortUser::MemberType_t::ChunkQueueData_t::MAX_CAPACITY;
    subscriberOptions.queueCapacity = MAX_QUEUE_CAPACITY + 1U;

    auto subscriberPort = m_runtime->getMiddlewareSubscriber(iox::capro::ServiceDescription("99", "1", "20"),
                                                             subscriberOptions,
                                                             iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_EQ(MAX_QUEUE_CAPACITY, subscriberPort->m_chunkReceiverData.m_queue.capacity());
}

TEST_F(PoshRuntime_test, GetMiddlewareSubscriberWithQueueCapacityZeroClampsQueueCapacityToOne)
{
    ::testing::Test::RecordProperty("TEST_ID", "9da3f4da-abe8-454c-9bc6-7f866d6d0545");
    iox::popo::SubscriberOptions subscriberOptions;
    subscriberOptions.queueCapacity = 0U;

    auto subscriberPort = m_runtime->getMiddlewareSubscriber(
        iox::capro::ServiceDescription("34", "4", "4"), subscriberOptions, iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_EQ(1U, subscriberPort->m_chunkReceiverData.m_queue.capacity());
}

TEST_F(PoshRuntime_test, GetMiddlewareSubscriberWithHistoryRequestLargerThanQueueCapacityClampsToQueueCapacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "77ca8d29-ffcb-4860-bf07-0af30b352e5c");
    iox::popo::SubscriberOptions subscriberOptions;
    constexpr uint64_t EXPECTED_HISTORY_REQUEST = 1U;
    subscriberOptions.queueCapacity = EXPECTED_HISTORY_REQUEST;
    subscriberOptions.historyRequest = 42U;

    auto subscriberPort =
        m_runtime->getMiddlewareSubscriber(iox::capro::ServiceDescription("Harder", "Better", "Faster"),
                                           subscriberOptions,
                                           iox::runtime::PortConfigInfo(33U, 11U, 22U));

    EXPECT_EQ(EXPECTED_HISTORY_REQUEST, subscriberPort->m_options.historyRequest);
}


TEST_F(PoshRuntime_test,
       GetMiddlewareSubscriberWithHistoryRequestLargerThanClampedQueueCapacityClampsToClampedQueueCapacity)
{
    ::testing::Test::RecordProperty("TEST_ID", "9746468f-d191-43d9-b973-542fa8a66101");
    iox::popo::SubscriberOptions subscriberOptions;
    constexpr uint64_t MAX_QUEUE_CAPACITY = iox::popo::SubscriberPortUser::MemberType_t::ChunkQueueData_t::MAX_CAPACITY;
    subscriberOptions.queueCapacity = MAX_QUEUE_CAPACITY + 1;
    subscriberOptions.historyRequest = MAX_QUEUE_CAPACITY + 2;

    auto subscriberPort = m_runtime->getMiddlewareSubscriber(
        iox::capro::ServiceDescription("91", "1", "2"), subscriberOptions, iox::runtime::PortConfigInfo(33U, 11U, 22U));

    EXPECT_EQ(MAX_QUEUE_CAPACITY, subscriberPort->m_options.queueCapacity);
    EXPECT_EQ(MAX_QUEUE_CAPACITY, subscriberPort->m_options.historyRequest);
}
TEST_F(PoshRuntime_test, GetMiddlewareSubscriberDefaultArgs)
{
    ::testing::Test::RecordProperty("TEST_ID", "e06b999c-e237-4e32-b826-a5ffdb6bb737");
    auto subscriberPort = m_runtime->getMiddlewareSubscriber(iox::capro::ServiceDescription("99", "1", "20"));

    ASSERT_NE(nullptr, subscriberPort);
}

TEST_F(PoshRuntime_test, GetMiddlewareSubscriberSubscriberlistOverflow)
{
    ::testing::Test::RecordProperty("TEST_ID", "d1281cbd-6520-424e-aace-fbd3aa5d73e9");

    uint32_t i{0U};
    for (; i < iox::MAX_SUBSCRIBERS; ++i)
    {
        auto subscriberPort = m_runtime->getMiddlewareSubscriber(
            iox::capro::ServiceDescription(into<lossy<RuntimeName_t>>(convert::toString(i)),
                                           into<lossy<RuntimeName_t>>(convert::toString(i + 1U)),
                                           into<lossy<RuntimeName_t>>(convert::toString(i + 2U))));
        ASSERT_NE(nullptr, subscriberPort);
    }
    IOX_TESTING_EXPECT_OK();

    auto subscriberPort = m_runtime->getMiddlewareSubscriber(
        iox::capro::ServiceDescription(into<lossy<RuntimeName_t>>(convert::toString(i)),
                                       into<lossy<RuntimeName_t>>(convert::toString(i + 1U)),
                                       into<lossy<RuntimeName_t>>(convert::toString(i + 2U))));

    EXPECT_EQ(nullptr, subscriberPort);

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::PORT_POOL__SUBSCRIBERLIST_OVERFLOW);
}

TEST_F(PoshRuntime_test, GetMiddlewareSubscriberWithoutSubscribeOnCreateLeadsToSubscriberThatDoesNotWantToBeSubscribed)
{
    ::testing::Test::RecordProperty("TEST_ID", "a59e3629-9aae-43e1-b88b-5dab441b1f17");
    iox::popo::SubscriberOptions subscriberOptions;
    subscriberOptions.subscribeOnCreate = false;

    auto subscriberPortData = m_runtime->getMiddlewareSubscriber(iox::capro::ServiceDescription("17", "17", "17"),
                                                                 subscriberOptions,
                                                                 iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_FALSE(subscriberPortData->m_subscribeRequested);
}

TEST_F(PoshRuntime_test, GetMiddlewareSubscriberWithSubscribeOnCreateLeadsToSubscriberThatWantsToBeSubscribed)
{
    ::testing::Test::RecordProperty("TEST_ID", "975a6edc-cc39-46d0-9bb7-79ab69f18fc3");
    iox::popo::SubscriberOptions subscriberOptions;
    subscriberOptions.subscribeOnCreate = true;

    auto subscriberPortData = m_runtime->getMiddlewareSubscriber(
        iox::capro::ServiceDescription("1", "2", "3"), subscriberOptions, iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_TRUE(subscriberPortData->m_subscribeRequested);
}

TEST_F(PoshRuntime_test, GetMiddlewareSubscriberWithoutExplicitlySetQueueFullPolicyLeadsToDiscardOldestData)
{
    ::testing::Test::RecordProperty("TEST_ID", "7fdd60c2-8b18-481c-8bad-5f6f70431196");
    iox::popo::SubscriberOptions subscriberOptions;

    const auto subscriberPortData =
        m_runtime->getMiddlewareSubscriber(iox::capro::ServiceDescription("9", "13", "1550"),
                                           subscriberOptions,
                                           iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_THAT(subscriberPortData->m_chunkReceiverData.m_queueFullPolicy,
                Eq(iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA));
}

TEST_F(PoshRuntime_test, GetMiddlewareSubscriberWithQueueFullPolicySetToDiscardOldestDataLeadsToDiscardOldestData)
{
    ::testing::Test::RecordProperty("TEST_ID", "9e5df6bf-a752-4db8-9e27-ba5ae1f02a52");
    iox::popo::SubscriberOptions subscriberOptions;
    subscriberOptions.queueFullPolicy = iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA;

    const auto subscriberPortData =
        m_runtime->getMiddlewareSubscriber(iox::capro::ServiceDescription("90", "130", "1550"),
                                           subscriberOptions,
                                           iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_THAT(subscriberPortData->m_chunkReceiverData.m_queueFullPolicy,
                Eq(iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA));
}

TEST_F(PoshRuntime_test, GetMiddlewareSubscriberWithQueueFullPolicySetToBlockPublisherLeadsToBlockPublisher)
{
    ::testing::Test::RecordProperty("TEST_ID", "ab60b748-6425-4ebf-8041-285a29a92756");
    iox::popo::SubscriberOptions subscriberOptions;
    subscriberOptions.queueFullPolicy = iox::popo::QueueFullPolicy::BLOCK_PRODUCER;

    const auto subscriberPortData =
        m_runtime->getMiddlewareSubscriber(iox::capro::ServiceDescription("18", "31", "400"),
                                           subscriberOptions,
                                           iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_THAT(subscriberPortData->m_chunkReceiverData.m_queueFullPolicy,
                Eq(iox::popo::QueueFullPolicy::BLOCK_PRODUCER));
}

TEST_F(PoshRuntime_test, GetMiddlewareClientWithDefaultArgsIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "2db35746-e402-443f-b374-3b6a239ab5fd");
    const iox::capro::ServiceDescription sd{"moon", "light", "drive"};
    iox::popo::ClientOptions defaultOptions;
    iox::runtime::PortConfigInfo defaultPortConfigInfo;

    auto clientPort = m_runtime->getMiddlewareClient(sd);

    ASSERT_THAT(clientPort, Ne(nullptr));

    checkClientInitialization(clientPort, sd, defaultOptions, defaultPortConfigInfo.memoryInfo);
    EXPECT_EQ(clientPort->m_connectionState.load(), iox::ConnectionState::WAIT_FOR_OFFER);
}

TEST_F(PoshRuntime_test, GetMiddlewareClientWithCustomClientOptionsIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "f61a81f4-f610-4e61-853b-ac114d9a801c");
    const iox::capro::ServiceDescription sd{"my", "guitar", "weeps"};
    iox::popo::ClientOptions clientOptions;
    clientOptions.responseQueueCapacity = 13U;
    clientOptions.nodeName = m_nodeName;
    clientOptions.connectOnCreate = false;
    clientOptions.responseQueueFullPolicy = iox::popo::QueueFullPolicy::BLOCK_PRODUCER;
    clientOptions.serverTooSlowPolicy = iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;
    const iox::runtime::PortConfigInfo portConfig{11U, 22U, 33U};

    auto clientPort = m_runtime->getMiddlewareClient(sd, clientOptions, portConfig);

    ASSERT_THAT(clientPort, Ne(nullptr));

    checkClientInitialization(clientPort, sd, clientOptions, portConfig.memoryInfo);
    EXPECT_EQ(clientPort->m_connectionState.load(), iox::ConnectionState::NOT_CONNECTED);
}

TEST_F(PoshRuntime_test, GetMiddlewareClientWithQueueGreaterMaxCapacityClampsQueueToMaximum)
{
    ::testing::Test::RecordProperty("TEST_ID", "8e34f962-e7c9-40ac-9796-a12f92c4d674");
    constexpr uint64_t MAX_QUEUE_CAPACITY = iox::popo::ClientChunkQueueConfig::MAX_QUEUE_CAPACITY;
    const iox::capro::ServiceDescription sd{"take", "guns", "down"};
    iox::popo::ClientOptions clientOptions;
    clientOptions.responseQueueCapacity = MAX_QUEUE_CAPACITY + 1U;

    auto clientPort = m_runtime->getMiddlewareClient(sd, clientOptions);

    ASSERT_THAT(clientPort, Ne(nullptr));
    EXPECT_EQ(clientPort->m_chunkReceiverData.m_queue.capacity(), MAX_QUEUE_CAPACITY);
}

TEST_F(PoshRuntime_test, GetMiddlewareClientWithQueueCapacityZeroClampsQueueCapacityToOne)
{
    ::testing::Test::RecordProperty("TEST_ID", "7b6ffd68-46d4-4339-a0df-6fecb621f765");
    const iox::capro::ServiceDescription sd{"rock", "and", "roll"};
    iox::popo::ClientOptions clientOptions;
    clientOptions.responseQueueCapacity = 0U;

    auto clientPort = m_runtime->getMiddlewareClient(sd, clientOptions);

    ASSERT_THAT(clientPort, Ne(nullptr));
    EXPECT_EQ(clientPort->m_chunkReceiverData.m_queue.capacity(), 1U);
}

TEST_F(PoshRuntime_test, GetMiddlewareClientWhenMaxClientsAreUsedResultsInClientlistOverflow)
{
    ::testing::Test::RecordProperty("TEST_ID", "6f2de2bf-5e7e-47b1-be42-92cf3fa71ba6");

    uint32_t i{0U};
    for (; i < iox::MAX_CLIENTS; ++i)
    {
        auto clientPort = m_runtime->getMiddlewareClient(
            iox::capro::ServiceDescription(into<lossy<RuntimeName_t>>(convert::toString(i)),
                                           into<lossy<RuntimeName_t>>(convert::toString(i + 1U)),
                                           into<lossy<RuntimeName_t>>(convert::toString(i + 2U))));
        ASSERT_THAT(clientPort, Ne(nullptr));
    }
    IOX_TESTING_EXPECT_OK();

    auto clientPort = m_runtime->getMiddlewareClient(
        iox::capro::ServiceDescription(into<lossy<RuntimeName_t>>(convert::toString(i)),
                                       into<lossy<RuntimeName_t>>(convert::toString(i + 1U)),
                                       into<lossy<RuntimeName_t>>(convert::toString(i + 2U))));
    EXPECT_THAT(clientPort, Eq(nullptr));

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::PORT_POOL__CLIENTLIST_OVERFLOW);
}

TEST_F(PoshRuntime_test, GetMiddlewareClientWithInvalidNodeNameLeadsToErrorHandlerCall)
{
    ::testing::Test::RecordProperty("TEST_ID", "b4433dfd-d2f8-4567-9483-aed956275ce8");
    const iox::capro::ServiceDescription sd{"great", "gig", "sky"};
    iox::popo::ClientOptions clientOptions;
    clientOptions.nodeName = m_invalidNodeName;

    m_runtime->getMiddlewareClient(sd, clientOptions);

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::POSH__RUNTIME_ROUDI_REQUEST_CLIENT_INVALID_RESPONSE);
}

TEST_F(PoshRuntime_test, GetMiddlewareServerWithDefaultArgsIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "cb3c1b4d-0d81-494c-954d-c1de10c244d7");
    const iox::capro::ServiceDescription sd{"ghouls", "night", "out"};
    iox::popo::ServerOptions defaultOptions;
    iox::runtime::PortConfigInfo defaultPortConfigInfo;

    auto serverPort = m_runtime->getMiddlewareServer(sd);

    ASSERT_THAT(serverPort, Ne(nullptr));
    checkServerInitialization(serverPort, sd, defaultOptions, defaultPortConfigInfo.memoryInfo);
    EXPECT_EQ(serverPort->m_offered.load(), true);
}

TEST_F(PoshRuntime_test, GetMiddlewareServerWithCustomServerOptionsIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "881c342c-58b9-4094-9e77-b4e68ab9a52a");
    const iox::capro::ServiceDescription sd{"take", "power", "back"};
    iox::popo::ServerOptions serverOptions;
    serverOptions.requestQueueCapacity = 13U;
    serverOptions.nodeName = m_nodeName;
    serverOptions.offerOnCreate = false;
    serverOptions.requestQueueFullPolicy = iox::popo::QueueFullPolicy::BLOCK_PRODUCER;
    serverOptions.clientTooSlowPolicy = iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;
    const iox::runtime::PortConfigInfo portConfig{11U, 22U, 33U};

    auto serverPort = m_runtime->getMiddlewareServer(sd, serverOptions, portConfig);

    ASSERT_THAT(serverPort, Ne(nullptr));
    checkServerInitialization(serverPort, sd, serverOptions, portConfig.memoryInfo);
    EXPECT_EQ(serverPort->m_offered.load(), false);
}

TEST_F(PoshRuntime_test, GetMiddlewareServerWithQueueGreaterMaxCapacityClampsQueueToMaximum)
{
    ::testing::Test::RecordProperty("TEST_ID", "91b21e80-0f98-4ae3-982c-54deaab93d96");
    constexpr uint64_t MAX_QUEUE_CAPACITY = iox::popo::ServerChunkQueueConfig::MAX_QUEUE_CAPACITY;
    const iox::capro::ServiceDescription sd{"stray", "cat", "blues"};
    iox::popo::ServerOptions serverOptions;
    serverOptions.requestQueueCapacity = MAX_QUEUE_CAPACITY + 1U;

    auto serverPort = m_runtime->getMiddlewareServer(sd, serverOptions);

    ASSERT_THAT(serverPort, Ne(nullptr));
    EXPECT_EQ(serverPort->m_chunkReceiverData.m_queue.capacity(), MAX_QUEUE_CAPACITY);
}

TEST_F(PoshRuntime_test, GetMiddlewareServerWithQueueCapacityZeroClampsQueueCapacityToOne)
{
    ::testing::Test::RecordProperty("TEST_ID", "a28a30eb-f3be-43c9-a948-26c71c5f12c9");
    const iox::capro::ServiceDescription sd{"she", "talks", "rainbow"};
    iox::popo::ServerOptions serverOptions;
    serverOptions.requestQueueCapacity = 0U;

    auto serverPort = m_runtime->getMiddlewareServer(sd, serverOptions);

    ASSERT_THAT(serverPort, Ne(nullptr));
    EXPECT_EQ(serverPort->m_chunkReceiverData.m_queue.capacity(), 1U);
}

TEST_F(PoshRuntime_test, GetMiddlewareServerWhenMaxServerAreUsedResultsInServerlistOverflow)
{
    ::testing::Test::RecordProperty("TEST_ID", "8f679838-3332-440c-aa95-d5c82d53a7cd");

    uint32_t i{0U};
    for (; i < iox::MAX_SERVERS; ++i)
    {
        auto serverPort = m_runtime->getMiddlewareServer(
            iox::capro::ServiceDescription(into<lossy<RuntimeName_t>>(convert::toString(i)),
                                           into<lossy<RuntimeName_t>>(convert::toString(i + 1U)),
                                           into<lossy<RuntimeName_t>>(convert::toString(i + 2U))));
        ASSERT_THAT(serverPort, Ne(nullptr));
    }
    IOX_TESTING_EXPECT_OK();

    auto serverPort = m_runtime->getMiddlewareServer(
        iox::capro::ServiceDescription(into<lossy<RuntimeName_t>>(convert::toString(i)),
                                       into<lossy<RuntimeName_t>>(convert::toString(i + 1U)),
                                       into<lossy<RuntimeName_t>>(convert::toString(i + 2U))));
    EXPECT_THAT(serverPort, Eq(nullptr));

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::PORT_POOL__SERVERLIST_OVERFLOW);
}

TEST_F(PoshRuntime_test, GetMiddlewareServerWithInvalidNodeNameLeadsToErrorHandlerCall)
{
    ::testing::Test::RecordProperty("TEST_ID", "95603ddc-1051-4dd7-a163-1c621f8a211a");
    const iox::capro::ServiceDescription sd{"it's", "over", "now"};
    iox::popo::ServerOptions serverOptions;
    serverOptions.nodeName = m_invalidNodeName;

    m_runtime->getMiddlewareServer(sd, serverOptions);

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::POSH__RUNTIME_ROUDI_REQUEST_SERVER_INVALID_RESPONSE);
}

TEST_F(PoshRuntime_test, GetMiddlewareConditionVariableIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "f2ccdca8-53ec-46d8-a34e-f56f996f57e0");
    auto conditionVariable = m_runtime->getMiddlewareConditionVariable();

    ASSERT_NE(nullptr, conditionVariable);
}

TEST_F(PoshRuntime_test, GetMiddlewareConditionVariableListOverflow)
{
    ::testing::Test::RecordProperty("TEST_ID", "6776a648-03c7-4bd0-ab24-72ed7e118e4f");

    for (uint32_t i = 0U; i < iox::MAX_NUMBER_OF_CONDITION_VARIABLES; ++i)
    {
        auto conditionVariable = m_runtime->getMiddlewareConditionVariable();
        ASSERT_NE(nullptr, conditionVariable);
    }
    IOX_TESTING_EXPECT_OK();

    auto conditionVariable = m_runtime->getMiddlewareConditionVariable();
    EXPECT_EQ(nullptr, conditionVariable);

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::PORT_POOL__CONDITION_VARIABLE_LIST_OVERFLOW);
}

TEST_F(PoshRuntime_test, ShutdownUnblocksBlockingPublisher)
{
    ::testing::Test::RecordProperty("TEST_ID", "c3a97770-ee9a-46a4-baf7-80ebbac74f4b");
    // get publisher and subscriber
    iox::capro::ServiceDescription serviceDescription{"don't", "stop", "me"};

    iox::popo::PublisherOptions publisherOptions{
        0U, iox::NodeName_t("node"), true, iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER};
    iox::popo::SubscriberOptions subscriberOptions{
        1U, 0U, iox::NodeName_t("node"), true, iox::popo::QueueFullPolicy::BLOCK_PRODUCER};

    iox::popo::Publisher<uint8_t> publisher{serviceDescription, publisherOptions};
    iox::popo::Subscriber<uint8_t> subscriber{serviceDescription, subscriberOptions};

    ASSERT_TRUE(publisher.hasSubscribers());
    ASSERT_THAT(subscriber.getSubscriptionState(), Eq(iox::SubscribeState::SUBSCRIBED));

    // send samples to fill subscriber queue
    ASSERT_FALSE(publisher.publishCopyOf(42U).has_error());

    iox::concurrent::Atomic<bool> wasSampleSent{false};

    constexpr iox::units::Duration DEADLOCK_TIMEOUT{5_s};
    Watchdog deadlockWatchdog{DEADLOCK_TIMEOUT};
    deadlockWatchdog.watchAndActOnFailure([] { std::terminate(); });

    // block in a separate thread
    Barrier isThreadStarted(1U);
    std::thread blockingPublisher([&] {
        isThreadStarted.notify();
        ASSERT_FALSE(publisher.publishCopyOf(42U).has_error());
        wasSampleSent = true;
    });

    // wait some time to check if the publisher is blocked
    isThreadStarted.wait();
    constexpr std::chrono::milliseconds SLEEP_TIME{100U};
    std::this_thread::sleep_for(SLEEP_TIME);
    EXPECT_THAT(wasSampleSent.load(), Eq(false));

    m_runtime->shutdown();

    blockingPublisher.join(); // ensure the wasChunkSent store happens before the read
    EXPECT_THAT(wasSampleSent.load(), Eq(true));
}

TEST_F(PoshRuntime_test, ShutdownUnblocksBlockingClient)
{
    ::testing::Test::RecordProperty("TEST_ID", "f67db1c5-8db9-4798-b73c-7175255c90fd");
    // get client and server
    iox::capro::ServiceDescription serviceDescription{"stop", "and", "smell"};

    iox::popo::ClientOptions clientOptions;
    clientOptions.responseQueueCapacity = 10U;
    clientOptions.responseQueueFullPolicy = iox::popo::QueueFullPolicy::BLOCK_PRODUCER;
    clientOptions.serverTooSlowPolicy = iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;

    iox::popo::ServerOptions serverOptions;
    serverOptions.requestQueueCapacity = 1U;
    serverOptions.requestQueueFullPolicy = iox::popo::QueueFullPolicy::BLOCK_PRODUCER;
    serverOptions.clientTooSlowPolicy = iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;

    iox::popo::UntypedClient client{serviceDescription, clientOptions};
    iox::popo::UntypedServer server{serviceDescription, serverOptions};

    ASSERT_TRUE(server.hasClients());
    ASSERT_THAT(client.getConnectionState(), Eq(iox::ConnectionState::CONNECTED));

    iox::concurrent::Atomic<bool> wasRequestSent{false};

    constexpr iox::units::Duration DEADLOCK_TIMEOUT{5_s};
    Watchdog deadlockWatchdog{DEADLOCK_TIMEOUT};
    deadlockWatchdog.watchAndActOnFailure([] { std::terminate(); });

    // block in a separate thread
    Barrier isThreadStarted(1U);
    std::thread blockingClient([&] {
        auto sendRequest = [&](bool expectError) {
            auto clientLoanResult = client.loan(sizeof(uint64_t), alignof(uint64_t));
            ASSERT_FALSE(clientLoanResult.has_error());
            auto sendResult = client.send(clientLoanResult.value());
            ASSERT_THAT(sendResult.has_error(), Eq(expectError));
            if (expectError)
            {
                EXPECT_THAT(sendResult.error(), Eq(iox::popo::ClientSendError::SERVER_NOT_AVAILABLE));
            }
        };

        // send request till queue is full
        for (uint64_t i = 0; i < serverOptions.requestQueueCapacity; ++i)
        {
            constexpr bool EXPECT_ERROR_INDICATOR{false};
            sendRequest(EXPECT_ERROR_INDICATOR);
        }

        // signal that an blocking send is expected
        isThreadStarted.notify();
        constexpr bool EXPECT_ERROR_INDICATOR{true};
        sendRequest(EXPECT_ERROR_INDICATOR);
        wasRequestSent = true;
    });

    // wait some time to check if the client is blocked
    constexpr std::chrono::milliseconds SLEEP_TIME{100U};
    isThreadStarted.wait();
    std::this_thread::sleep_for(SLEEP_TIME);
    EXPECT_THAT(wasRequestSent.load(), Eq(false));

    m_runtime->shutdown();

    blockingClient.join(); // ensure the wasRequestSent store happens before the read
    EXPECT_THAT(wasRequestSent.load(), Eq(true));
}

TEST_F(PoshRuntime_test, ShutdownUnblocksBlockingServer)
{
    ::testing::Test::RecordProperty("TEST_ID", "82128975-04e4-4a12-9a47-b884ad6ca97f");
    // get client and server
    iox::capro::ServiceDescription serviceDescription{"stop", "name", "love"};

    iox::popo::ClientOptions clientOptions;
    clientOptions.responseQueueCapacity = 1U;
    clientOptions.responseQueueFullPolicy = iox::popo::QueueFullPolicy::BLOCK_PRODUCER;
    clientOptions.serverTooSlowPolicy = iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;

    iox::popo::ServerOptions serverOptions;
    serverOptions.requestQueueCapacity = 10U;
    serverOptions.requestQueueFullPolicy = iox::popo::QueueFullPolicy::BLOCK_PRODUCER;
    serverOptions.clientTooSlowPolicy = iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;

    iox::popo::UntypedClient client{serviceDescription, clientOptions};
    iox::popo::UntypedServer server{serviceDescription, serverOptions};

    ASSERT_TRUE(server.hasClients());
    ASSERT_THAT(client.getConnectionState(), Eq(iox::ConnectionState::CONNECTED));

    // send requests to fill request queue
    for (uint64_t i = 0; i < clientOptions.responseQueueCapacity + 1; ++i)
    {
        auto clientLoanResult = client.loan(sizeof(uint64_t), alignof(uint64_t));
        ASSERT_FALSE(clientLoanResult.has_error());
        EXPECT_FALSE(client.send(clientLoanResult.value()).has_error());
    }

    iox::concurrent::Atomic<bool> wasResponseSent{false};

    constexpr iox::units::Duration DEADLOCK_TIMEOUT{5_s};
    Watchdog deadlockWatchdog{DEADLOCK_TIMEOUT};
    deadlockWatchdog.watchAndActOnFailure([] { std::terminate(); });

    // block in a separate thread
    Barrier isThreadStarted(1U);
    std::thread blockingServer([&] {
        auto processRequest = [&](bool expectError) {
            auto takeResult = server.take();
            ASSERT_FALSE(takeResult.has_error());
            auto loanResult = server.loan(
                iox::popo::RequestHeader::fromPayload(takeResult.value()), sizeof(uint64_t), alignof(uint64_t));
            ASSERT_FALSE(loanResult.has_error());
            auto sendResult = server.send(loanResult.value());
            ASSERT_THAT(sendResult.has_error(), Eq(expectError));
            if (expectError)
            {
                EXPECT_THAT(sendResult.error(), Eq(iox::popo::ServerSendError::CLIENT_NOT_AVAILABLE));
            }
        };

        for (uint64_t i = 0; i < clientOptions.responseQueueCapacity; ++i)
        {
            constexpr bool EXPECT_ERROR_INDICATOR{false};
            processRequest(EXPECT_ERROR_INDICATOR);
        }

        isThreadStarted.notify();
        constexpr bool EXPECT_ERROR_INDICATOR{true};
        processRequest(EXPECT_ERROR_INDICATOR);
        wasResponseSent = true;
    });

    // wait some time to check if the server is blocked
    constexpr std::chrono::milliseconds SLEEP_TIME{100U};
    isThreadStarted.wait();
    std::this_thread::sleep_for(SLEEP_TIME);
    EXPECT_THAT(wasResponseSent.load(), Eq(false));

    m_runtime->shutdown();

    blockingServer.join(); // ensure the wasResponseSent store happens before the read
    EXPECT_THAT(wasResponseSent.load(), Eq(true));
}

TEST(PoshRuntimeFactory_test, SetValidRuntimeFactorySucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "59c4e1e6-36f6-4f6d-b4c2-e84fa891f014");
    constexpr const char HYPNOTOAD[]{"hypnotoad"};
    constexpr const char BRAIN_SLUG[]{"brain-slug"};

    auto mockRuntime = PoshRuntimeMock::create(HYPNOTOAD);
    EXPECT_THAT(PoshRuntime::getInstance().getInstanceName().c_str(), StrEq(HYPNOTOAD));
    mockRuntime.reset();

    // if the PoshRuntimeMock could not change the runtime factory, the instance name would still be the old one
    mockRuntime = PoshRuntimeMock::create(BRAIN_SLUG);
    EXPECT_THAT(PoshRuntime::getInstance().getInstanceName().c_str(), StrEq(BRAIN_SLUG));
}

TEST(PoshRuntimeFactory_test, SetEmptyRuntimeFactoryFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "530ec778-b480-4a1e-8562-94f93cee2f5c");
    // this ensures resetting of the runtime factory in case the death test doesn't succeed
    auto mockRuntime = PoshRuntimeMock::create("hypnotoad");

    // do not use the setRuntimeFactory in a test with a running RouDiEnvironment
    GTEST_FLAG(death_test_style) = "threadsafe";
    EXPECT_DEATH(
        {
            class FactoryAccess : public PoshRuntime
            {
              public:
                using PoshRuntime::factory_t;
                using PoshRuntime::setRuntimeFactory;

              private:
                FactoryAccess(iox::optional<const iox::RuntimeName_t*> s)
                    : PoshRuntime(s)
                {
                }
            };

            FactoryAccess::setRuntimeFactory(FactoryAccess::factory_t());
        },
        ".*");
}

} // namespace
