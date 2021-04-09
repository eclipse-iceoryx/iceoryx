// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2020 - 2021 by Robert Bosch GmbH. All rights reserved.
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
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/testing/roudi_environment/roudi_environment.hpp"
#include "iceoryx_utils/testing/timing_test.hpp"
#include "test.hpp"

#include <type_traits>

using namespace ::testing;
using namespace iox::runtime;
using iox::roudi::RouDiEnvironment;

class PoshRuntimeTestAccess : public PoshRuntime
{
  public:
    using PoshRuntime::factory_t;
    /// @attention do not use the setRuntimeFactory in a test with a running RouDiEnvironment
    using PoshRuntime::setRuntimeFactory;

    PoshRuntimeTestAccess(iox::cxx::optional<const iox::RuntimeName_t*> s)
        : PoshRuntime(s)
    {
    }

    static PoshRuntime& getDefaultRuntime(iox::cxx::optional<const iox::RuntimeName_t*> name)
    {
        return PoshRuntime::defaultRuntimeFactory(name);
    }

    static void resetRuntimeFactory()
    {
        PoshRuntime::setRuntimeFactory(PoshRuntime::defaultRuntimeFactory);
    }
};

namespace
{
bool callbackWasCalled = false;
PoshRuntime& testFactory(iox::cxx::optional<const iox::RuntimeName_t*> name)
{
    callbackWasCalled = true;
    return PoshRuntimeTestAccess::getDefaultRuntime(name);
}
} // namespace

class PoshRuntime_test : public Test
{
  public:
    PoshRuntime_test()
    {
    }

    virtual ~PoshRuntime_test()
    {
    }

    virtual void SetUp()
    {
        callbackWasCalled = false;
        internal::CaptureStdout();
    };

    virtual void TearDown()
    {
        std::string output = internal::GetCapturedStdout();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    };

    void InterOpWait()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    const iox::RuntimeName_t m_runtimeName{"publisher"};
    RouDiEnvironment m_roudiEnv{iox::RouDiConfig_t().setDefaults()};
    PoshRuntime* m_runtime{&iox::runtime::PoshRuntime::initRuntime(m_runtimeName)};
    IpcMessage m_sendBuffer;
    IpcMessage m_receiveBuffer;
    const iox::NodeName_t m_nodeName{"testNode"};
    const iox::NodeName_t m_invalidNodeName{"invalidNode,"};
    static bool m_errorHandlerCalled;
};

bool PoshRuntime_test::m_errorHandlerCalled{false};


TEST_F(PoshRuntime_test, ValidAppName)
{
    iox::RuntimeName_t appName("valid_name");

    EXPECT_NO_FATAL_FAILURE({ PoshRuntime::initRuntime(appName); });
}

TEST_F(PoshRuntime_test, MaxAppNameLength)
{
    std::string maxValidName(iox::MAX_RUNTIME_NAME_LENGTH, 's');

    auto& runtime = PoshRuntime::initRuntime(iox::RuntimeName_t(iox::cxx::TruncateToCapacity, maxValidName));

    EXPECT_THAT(maxValidName, StrEq(runtime.getInstanceName().c_str()));
}

TEST_F(PoshRuntime_test, NoAppName)
{
    const iox::RuntimeName_t invalidAppName("");

    EXPECT_DEATH({ PoshRuntime::initRuntime(invalidAppName); },
                 "Cannot initialize runtime. Application name must not be empty!");
}

TEST_F(PoshRuntime_test, LeadingSlashAppName)
{
    const iox::RuntimeName_t invalidAppName = "/miau";

    EXPECT_DEATH({ PoshRuntime::initRuntime(invalidAppName); },
                 "Cannot initialize runtime. Please remove leading slash from Application name /miau");
}

// since getInstance is a singleton and test class creates instance of Poshruntime
// when getInstance() is called without parameter, it returns existing instance
// To be able to test this, we don't use the test fixture
TEST(PoshRuntime, AppNameEmpty)
{
    EXPECT_DEATH({ iox::runtime::PoshRuntime::getInstance(); },
                 "Cannot initialize runtime. Application name has not been specified!");
}


TEST_F(PoshRuntime_test, GetInstanceNameIsSuccessful)
{
    const iox::RuntimeName_t appname = "app";

    auto& sut = PoshRuntime::initRuntime(appname);

    EXPECT_EQ(sut.getInstanceName(), appname);
}


TEST_F(PoshRuntime_test, GetMiddlewareApplicationIsSuccessful)
{
    const auto applicationPortData = m_runtime->getMiddlewareApplication();

    ASSERT_NE(nullptr, applicationPortData);
    EXPECT_EQ(m_runtimeName, applicationPortData->m_runtimeName);
    EXPECT_EQ(iox::capro::ServiceDescription(0U, 0U, 0U), applicationPortData->m_serviceDescription);
    EXPECT_EQ(false, applicationPortData->m_toBeDestroyed);
}

TEST_F(PoshRuntime_test, GetMiddlewareInterfaceWithInvalidNodeNameIsNotSuccessful)
{
    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_THAT(errorLevel, Eq(iox::ErrorLevel::SEVERE));
        });

    m_runtime->getMiddlewareInterface(iox::capro::Interfaces::INTERNAL, m_invalidNodeName);

    ASSERT_THAT(detectedError.has_value(), Eq(true));
    EXPECT_THAT(detectedError.value(),
                Eq(iox::Error::kPOSH__RUNTIME_ROUDI_GET_MW_INTERFACE_WRONG_IPC_MESSAGE_RESPONSE));
}

TEST_F(PoshRuntime_test, GetMiddlewareApplicationApplicationlistOverflow)
{
    auto applicationlistOverflowDetected{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&applicationlistOverflowDetected](const iox::Error error, const std::function<void()>, const iox::ErrorLevel) {
            applicationlistOverflowDetected = true;
            EXPECT_THAT(error, Eq(iox::Error::kPORT_POOL__APPLICATIONLIST_OVERFLOW));
        });

    // i = 1 because there is already an active runtime in test fixture class which acquired an application port
    for (auto i = 1U; i < iox::MAX_PROCESS_NUMBER; ++i)
    {
        auto appPort = m_runtime->getMiddlewareApplication();
        ASSERT_NE(nullptr, appPort);
    }

    EXPECT_FALSE(applicationlistOverflowDetected);

    auto appPort = m_runtime->getMiddlewareApplication();

    EXPECT_EQ(nullptr, appPort);
    EXPECT_TRUE(applicationlistOverflowDetected);
}

TEST_F(PoshRuntime_test, GetMiddlewareInterfaceIsSuccessful)
{
    const auto interfacePortData = m_runtime->getMiddlewareInterface(iox::capro::Interfaces::INTERNAL, m_nodeName);

    ASSERT_NE(nullptr, interfacePortData);
    EXPECT_EQ(m_runtimeName, interfacePortData->m_runtimeName);
    EXPECT_EQ(iox::capro::ServiceDescription(0U, 0U, 0U), interfacePortData->m_serviceDescription);
    EXPECT_EQ(false, interfacePortData->m_toBeDestroyed);
    EXPECT_EQ(true, interfacePortData->m_doInitialOfferForward);
}

TEST_F(PoshRuntime_test, GetMiddlewareInterfaceInterfacelistOverflow)
{
    auto interfacelistOverflowDetected{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&interfacelistOverflowDetected](const iox::Error error, const std::function<void()>, const iox::ErrorLevel) {
            interfacelistOverflowDetected = true;
            EXPECT_THAT(error, Eq(iox::Error::kPORT_POOL__INTERFACELIST_OVERFLOW));
        });

    for (auto i = 0U; i < iox::MAX_INTERFACE_NUMBER; ++i)
    {
        auto interfacePort = m_runtime->getMiddlewareInterface(iox::capro::Interfaces::INTERNAL);
        ASSERT_NE(nullptr, interfacePort);
    }

    EXPECT_FALSE(interfacelistOverflowDetected);

    auto interfacePort = m_runtime->getMiddlewareInterface(iox::capro::Interfaces::INTERNAL);

    EXPECT_EQ(nullptr, interfacePort);
    EXPECT_TRUE(interfacelistOverflowDetected);
}


TEST_F(PoshRuntime_test, SendRequestToRouDiValidMessage)
{
    m_sendBuffer << IpcMessageTypeToString(IpcMessageType::CREATE_INTERFACE) << m_runtimeName
                 << static_cast<uint32_t>(iox::capro::Interfaces::INTERNAL) << m_nodeName;

    const auto successfullySent = m_runtime->sendRequestToRouDi(m_sendBuffer, m_receiveBuffer);

    EXPECT_TRUE(m_receiveBuffer.isValid());
    EXPECT_TRUE(successfullySent);
}


TEST_F(PoshRuntime_test, SendRequestToRouDiInvalidMessage)
{
    m_sendBuffer << IpcMessageTypeToString(IpcMessageType::CREATE_INTERFACE) << m_runtimeName
                 << static_cast<uint32_t>(iox::capro::Interfaces::INTERNAL) << m_invalidNodeName;

    const auto successfullySent = m_runtime->sendRequestToRouDi(m_sendBuffer, m_receiveBuffer);

    EXPECT_FALSE(successfullySent);
}

TEST_F(PoshRuntime_test, GetMiddlewarePublisherIsSuccessful)
{
    iox::popo::PublisherOptions publisherOptions;
    publisherOptions.historyCapacity = 13U;
    publisherOptions.nodeName = m_nodeName;
    const auto publisherPort = m_runtime->getMiddlewarePublisher(
        iox::capro::ServiceDescription(99U, 1U, 20U), publisherOptions, iox::runtime::PortConfigInfo(11U, 22U, 33U));

    ASSERT_NE(nullptr, publisherPort);
    EXPECT_EQ(iox::capro::ServiceDescription(99U, 1U, 20U), publisherPort->m_serviceDescription);
    EXPECT_EQ(publisherOptions.historyCapacity, publisherPort->m_chunkSenderData.m_historyCapacity);
}

TEST_F(PoshRuntime_test, GetMiddlewarePublisherWithHistoryGreaterMaxCapacityClampsHistoryToMaximum)
{
    // arrange
    iox::popo::PublisherOptions publisherOptions;
    publisherOptions.historyCapacity = iox::MAX_PUBLISHER_HISTORY + 1U;

    // act
    const auto publisherPort =
        m_runtime->getMiddlewarePublisher(iox::capro::ServiceDescription(99U, 1U, 20U), publisherOptions);

    // assert
    ASSERT_NE(nullptr, publisherPort);
    EXPECT_EQ(publisherPort->m_chunkSenderData.m_historyCapacity, iox::MAX_PUBLISHER_HISTORY);
}

TEST_F(PoshRuntime_test, getMiddlewarePublisherDefaultArgs)
{
    const auto publisherPort = m_runtime->getMiddlewarePublisher(iox::capro::ServiceDescription(99U, 1U, 20U));

    ASSERT_NE(nullptr, publisherPort);
}


TEST_F(PoshRuntime_test, getMiddlewarePublisherPublisherlistOverflow)
{
    auto publisherlistOverflowDetected{false};

    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&publisherlistOverflowDetected](const iox::Error error, const std::function<void()>, const iox::ErrorLevel) {
            if (error == iox::Error::kPORT_POOL__PUBLISHERLIST_OVERFLOW)
            {
                publisherlistOverflowDetected = true;
            }
        });

    uint32_t i{0U};
    for (; i < (iox::MAX_PUBLISHERS - iox::PUBLISHERS_RESERVED_FOR_INTROSPECTION); ++i)
    {
        auto publisherPort = m_runtime->getMiddlewarePublisher(iox::capro::ServiceDescription(i, i + 1U, i + 2U));
        ASSERT_NE(nullptr, publisherPort);
    }
    EXPECT_FALSE(publisherlistOverflowDetected);

    auto publisherPort = m_runtime->getMiddlewarePublisher(iox::capro::ServiceDescription(i, i + 1U, i + 2U));
    EXPECT_EQ(nullptr, publisherPort);
    EXPECT_TRUE(publisherlistOverflowDetected);
}

TEST_F(PoshRuntime_test, GetMiddlewarePublisherWithSameServiceDescriptionsAndOneToManyPolicyFails)
{
    auto publisherDuplicateDetected{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&publisherDuplicateDetected](const iox::Error error, const std::function<void()>, const iox::ErrorLevel) {
            if (error == iox::Error::kPOSH__RUNTIME_PUBLISHER_PORT_NOT_UNIQUE)
            {
                publisherDuplicateDetected = true;
            }
        });

    auto sameServiceDescription = iox::capro::ServiceDescription(99U, 1U, 20U);

    const auto publisherPort1 = m_runtime->getMiddlewarePublisher(
        sameServiceDescription, iox::popo::PublisherOptions(), iox::runtime::PortConfigInfo(11U, 22U, 33U));

    const auto publisherPort2 = m_runtime->getMiddlewarePublisher(
        sameServiceDescription, iox::popo::PublisherOptions(), iox::runtime::PortConfigInfo(11U, 22U, 33U));

    ASSERT_NE(nullptr, publisherPort1);

    if (std::is_same<iox::build::CommunicationPolicy, iox::build::OneToManyPolicy>::value)
    {
        ASSERT_EQ(nullptr, publisherPort2);
        EXPECT_TRUE(publisherDuplicateDetected);
    }
    else if (std::is_same<iox::build::CommunicationPolicy, iox::build::ManyToManyPolicy>::value)
    {
        ASSERT_NE(nullptr, publisherPort2);
    }
}

TEST_F(PoshRuntime_test, GetMiddlewarePublisherWithoutOfferOnCreateLeadsToNotOfferedPublisherBeingCreated)
{
    iox::popo::PublisherOptions publisherOptions;
    publisherOptions.offerOnCreate = false;

    const auto publisherPortData = m_runtime->getMiddlewarePublisher(
        iox::capro::ServiceDescription(69U, 96U, 1893U), publisherOptions, iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_FALSE(publisherPortData->m_offeringRequested);
}

TEST_F(PoshRuntime_test, GetMiddlewarePublisherWithOfferOnCreateLeadsToOfferedPublisherBeingCreated)
{
    iox::popo::PublisherOptions publisherOptions;
    publisherOptions.offerOnCreate = true;

    const auto publisherPortData = m_runtime->getMiddlewarePublisher(
        iox::capro::ServiceDescription(17U, 4U, 21U), publisherOptions, iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_TRUE(publisherPortData->m_offeringRequested);
}

TEST_F(PoshRuntime_test, GetMiddlewarePublisherWithoutExplicitlySetQueueFullPolicyLeadsToDiscardOldestData)
{
    iox::popo::PublisherOptions publisherOptions;

    const auto publisherPortData = m_runtime->getMiddlewarePublisher(
        iox::capro::ServiceDescription(9U, 13U, 1550U), publisherOptions, iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_THAT(publisherPortData->m_chunkSenderData.m_subscriberTooSlowPolicy,
                Eq(iox::popo::SubscriberTooSlowPolicy::DISCARD_OLDEST_DATA));
}

TEST_F(PoshRuntime_test, GetMiddlewarePublisherWithQueueFullPolicySetToDiscardOldestDataLeadsToDiscardOldestData)
{
    iox::popo::PublisherOptions publisherOptions;
    publisherOptions.subscriberTooSlowPolicy = iox::popo::SubscriberTooSlowPolicy::DISCARD_OLDEST_DATA;

    const auto publisherPortData = m_runtime->getMiddlewarePublisher(iox::capro::ServiceDescription(90U, 130U, 1550U),
                                                                     publisherOptions,
                                                                     iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_THAT(publisherPortData->m_chunkSenderData.m_subscriberTooSlowPolicy,
                Eq(iox::popo::SubscriberTooSlowPolicy::DISCARD_OLDEST_DATA));
}

TEST_F(PoshRuntime_test, GetMiddlewarePublisherWithQueueFullPolicySetToWaitForSubscriberLeadsToWaitForSubscriber)
{
    iox::popo::PublisherOptions publisherOptions;
    publisherOptions.subscriberTooSlowPolicy = iox::popo::SubscriberTooSlowPolicy::WAIT_FOR_SUBSCRIBER;

    const auto publisherPortData = m_runtime->getMiddlewarePublisher(
        iox::capro::ServiceDescription(18U, 31U, 400U), publisherOptions, iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_THAT(publisherPortData->m_chunkSenderData.m_subscriberTooSlowPolicy,
                Eq(iox::popo::SubscriberTooSlowPolicy::WAIT_FOR_SUBSCRIBER));
}

TEST_F(PoshRuntime_test, GetMiddlewareSubscriberIsSuccessful)
{
    iox::popo::SubscriberOptions subscriberOptions;
    subscriberOptions.historyRequest = 13U;
    subscriberOptions.queueCapacity = 42U;
    subscriberOptions.nodeName = m_nodeName;

    auto subscriberPort = m_runtime->getMiddlewareSubscriber(
        iox::capro::ServiceDescription(99U, 1U, 20U), subscriberOptions, iox::runtime::PortConfigInfo(11U, 22U, 33U));

    ASSERT_NE(nullptr, subscriberPort);
    EXPECT_EQ(iox::capro::ServiceDescription(99U, 1U, 20U), subscriberPort->m_serviceDescription);
    EXPECT_EQ(subscriberOptions.historyRequest, subscriberPort->m_historyRequest);
    EXPECT_EQ(subscriberOptions.queueCapacity, subscriberPort->m_chunkReceiverData.m_queue.capacity());
}

TEST_F(PoshRuntime_test, GetMiddlewareSubscriberWithQueueGreaterMaxCapacityClampsQueueToMaximum)
{
    iox::popo::SubscriberOptions subscriberOptions;
    constexpr uint64_t MAX_QUEUE_CAPACITY = iox::popo::SubscriberPortUser::MemberType_t::ChunkQueueData_t::MAX_CAPACITY;
    subscriberOptions.queueCapacity = MAX_QUEUE_CAPACITY + 1U;

    auto subscriberPort = m_runtime->getMiddlewareSubscriber(
        iox::capro::ServiceDescription(99U, 1U, 20U), subscriberOptions, iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_EQ(MAX_QUEUE_CAPACITY, subscriberPort->m_chunkReceiverData.m_queue.capacity());
}

TEST_F(PoshRuntime_test, GetMiddlewareSubscriberWithQueueCapacityZeroClampsQueueCapacityTo1)
{
    iox::popo::SubscriberOptions subscriberOptions;
    subscriberOptions.queueCapacity = 0U;

    auto subscriberPort = m_runtime->getMiddlewareSubscriber(
        iox::capro::ServiceDescription(34U, 4U, 4U), subscriberOptions, iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_EQ(1U, subscriberPort->m_chunkReceiverData.m_queue.capacity());
}

TEST_F(PoshRuntime_test, GetMiddlewareSubscriberDefaultArgs)
{
    auto subscriberPort = m_runtime->getMiddlewareSubscriber(iox::capro::ServiceDescription(99U, 1U, 20U));

    ASSERT_NE(nullptr, subscriberPort);
}

TEST_F(PoshRuntime_test, GetMiddlewareSubscriberSubscriberlistOverflow)
{
    auto subscriberlistOverflowDetected{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&subscriberlistOverflowDetected](const iox::Error error, const std::function<void()>, const iox::ErrorLevel) {
            if (error == iox::Error::kPORT_POOL__SUBSCRIBERLIST_OVERFLOW)
            {
                subscriberlistOverflowDetected = true;
            }
        });

    uint32_t i{0U};
    for (; i < iox::MAX_SUBSCRIBERS; ++i)
    {
        auto subscriberPort = m_runtime->getMiddlewareSubscriber(iox::capro::ServiceDescription(i, i + 1U, i + 2U));
        ASSERT_NE(nullptr, subscriberPort);
    }
    EXPECT_FALSE(subscriberlistOverflowDetected);

    auto subscriberPort = m_runtime->getMiddlewareSubscriber(iox::capro::ServiceDescription(i, i + 1U, i + 2U));

    EXPECT_EQ(nullptr, subscriberPort);
    EXPECT_TRUE(subscriberlistOverflowDetected);
}

TEST_F(PoshRuntime_test, GetMiddlewareSubscriberWithoutSubscribeOnCreateLeadsToSubscriberThatDoesNotWantToBeSubscribed)
{
    iox::popo::SubscriberOptions subscriberOptions;
    subscriberOptions.subscribeOnCreate = false;

    auto subscriberPortData = m_runtime->getMiddlewareSubscriber(
        iox::capro::ServiceDescription(17U, 17U, 17U), subscriberOptions, iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_FALSE(subscriberPortData->m_subscribeRequested);
}

TEST_F(PoshRuntime_test, GetMiddlewareSubscriberWithSubscribeOnCreateLeadsToSubscriberThatWantsToBeSubscribed)
{
    iox::popo::SubscriberOptions subscriberOptions;
    subscriberOptions.subscribeOnCreate = true;

    auto subscriberPortData = m_runtime->getMiddlewareSubscriber(
        iox::capro::ServiceDescription(1U, 2U, 3U), subscriberOptions, iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_TRUE(subscriberPortData->m_subscribeRequested);
}

TEST_F(PoshRuntime_test, GetMiddlewareSubscriberWithoutExplicitlySetQueueFullPolicyLeadsToDiscardOldestData)
{
    iox::popo::SubscriberOptions subscriberOptions;

    const auto subscriberPortData = m_runtime->getMiddlewareSubscriber(
        iox::capro::ServiceDescription(9U, 13U, 1550U), subscriberOptions, iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_THAT(subscriberPortData->m_chunkReceiverData.m_queueFullPolicy,
                Eq(iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA));
}

TEST_F(PoshRuntime_test, GetMiddlewareSubscriberWithQueueFullPolicySetToDiscardOldestDataLeadsToDiscardOldestData)
{
    iox::popo::SubscriberOptions subscriberOptions;
    subscriberOptions.queueFullPolicy = iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA;

    const auto subscriberPortData = m_runtime->getMiddlewareSubscriber(iox::capro::ServiceDescription(90U, 130U, 1550U),
                                                                       subscriberOptions,
                                                                       iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_THAT(subscriberPortData->m_chunkReceiverData.m_queueFullPolicy,
                Eq(iox::popo::QueueFullPolicy::DISCARD_OLDEST_DATA));
}

TEST_F(PoshRuntime_test, GetMiddlewareSubscriberWithQueueFullPolicySetToBlockPublisherLeadsToBlockPublisher)
{
    iox::popo::SubscriberOptions subscriberOptions;
    subscriberOptions.queueFullPolicy = iox::popo::QueueFullPolicy::BLOCK_PUBLISHER;

    const auto subscriberPortData = m_runtime->getMiddlewareSubscriber(
        iox::capro::ServiceDescription(18U, 31U, 400U), subscriberOptions, iox::runtime::PortConfigInfo(11U, 22U, 33U));

    EXPECT_THAT(subscriberPortData->m_chunkReceiverData.m_queueFullPolicy,
                Eq(iox::popo::QueueFullPolicy::BLOCK_PUBLISHER));
}

TEST_F(PoshRuntime_test, GetMiddlewareConditionVariableIsSuccessful)
{
    auto conditionVariable = m_runtime->getMiddlewareConditionVariable();

    ASSERT_NE(nullptr, conditionVariable);
}

TEST_F(PoshRuntime_test, GetMiddlewareConditionVariableListOverflow)
{
    auto conditionVariableListOverflowDetected{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&conditionVariableListOverflowDetected](
            const iox::Error error, const std::function<void()>, const iox::ErrorLevel) {
            if (error == iox::Error::kPORT_POOL__CONDITION_VARIABLE_LIST_OVERFLOW)
            {
                conditionVariableListOverflowDetected = true;
            }
        });

    for (uint32_t i = 0U; i < iox::MAX_NUMBER_OF_CONDITION_VARIABLES; ++i)
    {
        auto conditionVariable = m_runtime->getMiddlewareConditionVariable();
        ASSERT_NE(nullptr, conditionVariable);
    }
    EXPECT_FALSE(conditionVariableListOverflowDetected);

    auto conditionVariable = m_runtime->getMiddlewareConditionVariable();
    EXPECT_EQ(nullptr, conditionVariable);
    EXPECT_TRUE(conditionVariableListOverflowDetected);
}

TIMING_TEST_F(PoshRuntime_test, GetServiceRegistryChangeCounterOfferStopOfferService, Repeat(5), [&] {
    auto serviceCounter = m_runtime->getServiceRegistryChangeCounter();
    auto initialCout = serviceCounter->load();

    m_runtime->offerService({"service1", "instance1"});
    this->InterOpWait();

    TIMING_TEST_EXPECT_TRUE(initialCout + 1 == serviceCounter->load());

    m_runtime->stopOfferService({"service1", "instance1"});
    this->InterOpWait();

    TIMING_TEST_EXPECT_TRUE(initialCout + 2 == serviceCounter->load());
});


TEST_F(PoshRuntime_test, CreateNodeReturnValue)
{
    const uint32_t nodeDeviceIdentifier = 1U;
    iox::runtime::NodeProperty nodeProperty(m_nodeName, nodeDeviceIdentifier);

    auto nodeData = m_runtime->createNode(nodeProperty);

    EXPECT_EQ(m_runtimeName, nodeData->m_runtimeName);
    EXPECT_EQ(m_nodeName, nodeData->m_nodeName);

    /// @todo I am passing nodeDeviceIdentifier as 1, but it returns 0, is this expected?
    // EXPECT_EQ(nodeDeviceIdentifier, nodeData->m_nodeDeviceIdentifier);
}

TEST_F(PoshRuntime_test, CreatingNodeWithInvalidNameLeadsToTermination)
{
    const uint32_t nodeDeviceIdentifier = 1U;
    iox::runtime::NodeProperty nodeProperty(m_invalidNodeName, nodeDeviceIdentifier);

    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_THAT(errorLevel, Eq(iox::ErrorLevel::SEVERE));
        });

    m_runtime->createNode(nodeProperty);

    ASSERT_THAT(detectedError.has_value(), Eq(true));
    EXPECT_THAT(detectedError.value(), Eq(iox::Error::kPOSH__RUNTIME_ROUDI_CREATE_NODE_WRONG_IPC_MESSAGE_RESPONSE));
}

TEST_F(PoshRuntime_test, OfferDefaultServiceDescriptionIsInvalid)
{
    auto isServiceOffered = m_runtime->offerService(iox::capro::ServiceDescription());

    EXPECT_FALSE(isServiceOffered);
}

TEST_F(PoshRuntime_test, OfferANYServiceStringIsInvalid)
{
    auto isServiceOffered = m_runtime->offerService(iox::capro::ServiceDescription(
        iox::capro::AnyServiceString, iox::capro::AnyInstanceString, iox::capro::AnyEventString));

    EXPECT_FALSE(isServiceOffered);
}

TEST_F(PoshRuntime_test, OfferANYServiceIdIsInvalid)
{
    auto isServiceOffered = m_runtime->offerService(
        iox::capro::ServiceDescription(iox::capro::AnyService, iox::capro::AnyInstance, iox::capro::AnyEvent));

    EXPECT_FALSE(isServiceOffered);
}

TEST_F(PoshRuntime_test, FindServiceReturnsNoInstanceForDefaultDescription)
{
    PoshRuntime* m_receiverRuntime{&iox::runtime::PoshRuntime::initRuntime("subscriber")};

    m_runtime->offerService(iox::capro::ServiceDescription());
    this->InterOpWait();
    auto instanceContainer = m_receiverRuntime->findService(iox::capro::ServiceDescription());

    EXPECT_THAT(0u, instanceContainer.value().size());
}

// disabled because we cannot use the RouDiEnvironment but need a RouDi for this test
// will be re-enabled with the PoshRuntime Mock from #449
TEST(PoshRuntimeFactory_test, DISABLED_SetValidRuntimeFactorySucceeds)
{
    // do not use the setRuntimeFactory in a test with a running RouDiEnvironment
    PoshRuntimeTestAccess::setRuntimeFactory(testFactory);
    PoshRuntimeTestAccess::initRuntime("instance");
    PoshRuntimeTestAccess::resetRuntimeFactory();

    EXPECT_TRUE(callbackWasCalled);
}

// disabled because we cannot use the RouDiEnvironment but need a RouDi for this test
// will be re-enabled with the PoshRuntime Mock from #449
TEST(PoshRuntimeFactory_test, DISABLED_SetEmptyRuntimeFactoryFails)
{
    // do not use the setRuntimeFactory in a test with a running RouDiEnvironment
    EXPECT_DEATH({ PoshRuntimeTestAccess::setRuntimeFactory(PoshRuntimeTestAccess::factory_t()); },
                 "Cannot set runtime factory. Passed factory must not be empty!");
    // just in case the previous test doesn't die and breaks the following tests
    PoshRuntimeTestAccess::resetRuntimeFactory();
}
