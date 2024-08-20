// Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iceoryx_posh/internal/roudi/port_pool_data.hpp"
#include "iceoryx_posh/popo/client_options.hpp"
#include "iceoryx_posh/popo/subscriber_options.hpp"
#include "iceoryx_posh/roudi/port_pool.hpp"
#include "iox/detail/convert.hpp"
#include "iox/std_string_support.hpp"

#include "iceoryx_hoofs/testing/error_reporting/testing_support.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::capro;
using namespace iox;

static constexpr uint32_t DEFAULT_DEVICE_ID{20U};
static constexpr uint32_t DEFAULT_MEMORY_TYPE{100U};
static constexpr uint32_t QUEUE_CAPACITY{10U};
class PortPool_test : public Test
{
  public:
    bool addClientPorts(uint32_t numberOfClientPortsToAdd,
                        std::function<void(const capro::ServiceDescription& sd,
                                           const RuntimeName_t& runtimeName,
                                           const popo::ClientPortData& clientPort)> onAdd)
    {
        for (uint32_t i = 0; i < numberOfClientPortsToAdd; ++i)
        {
            std::string service = "service" + convert::toString(i);
            auto serviceId = into<lossy<IdString_t>>(service);
            ServiceDescription sd{serviceId, "instance", "event"};
            RuntimeName_t runtimeName = into<lossy<RuntimeName_t>>("AppName" + convert::toString(i));

            auto clientPortResult = sut.addClientPort(sd, &m_memoryManager, runtimeName, m_clientOptions, m_memoryInfo);
            if (clientPortResult.has_error())
            {
                return false;
            }
            onAdd(sd, runtimeName, *clientPortResult.value());
        }

        return true;
    }

    bool addServerPorts(uint32_t numberOfServerPortsToAdd,
                        std::function<void(const capro::ServiceDescription& sd,
                                           const RuntimeName_t& runtimeName,
                                           const popo::ServerPortData& serverPortPort)> onAdd)
    {
        for (uint32_t i = 0; i < numberOfServerPortsToAdd; ++i)
        {
            std::string service = "service" + convert::toString(i);
            auto serviceId = into<lossy<IdString_t>>(service);
            ServiceDescription sd{serviceId, "instance", "event"};
            RuntimeName_t runtimeName = into<lossy<RuntimeName_t>>("AppName" + convert::toString(i));

            auto serverPortResult = sut.addServerPort(sd, &m_memoryManager, runtimeName, m_serverOptions, m_memoryInfo);
            if (serverPortResult.has_error())
            {
                return false;
            }
            onAdd(sd, runtimeName, *serverPortResult.value());
        }

        return true;
    }

  public:
    roudi::PortPoolData m_portPoolData{roudi::DEFAULT_UNIQUE_ROUDI_ID};
    roudi::PortPool sut{m_portPoolData};

    ServiceDescription m_serviceDescription{"service1", "instance1", "event1"};
    RuntimeName_t m_applicationName{"AppName"};
    RuntimeName_t m_runtimeName{"runtimeName"};
    NodeName_t m_nodeName{"nodeName"};
    const uint64_t m_nodeDeviceId = 999U;
    mepoo::MemoryManager m_memoryManager;
    popo::PublisherOptions m_publisherOptions{10U, m_nodeName};
    popo::SubscriberOptions m_subscriberOptions{
        iox::popo::SubscriberPortData::ChunkQueueData_t::MAX_CAPACITY, 10U, m_nodeName};
    popo::ClientOptions m_clientOptions{QUEUE_CAPACITY, m_nodeName};
    popo::ServerOptions m_serverOptions{QUEUE_CAPACITY, m_nodeName};
    iox::mepoo::MemoryInfo m_memoryInfo{DEFAULT_DEVICE_ID, DEFAULT_MEMORY_TYPE};
};

// BEGIN PublisherPort tests

TEST_F(PortPool_test, AddPublisherPortIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "8e2271f5-65a9-41ea-bffa-7f1a55321cc0");
    auto publisherPort = sut.addPublisherPort(
        m_serviceDescription, &m_memoryManager, m_applicationName, m_publisherOptions, m_memoryInfo);

    ASSERT_THAT(publisherPort.has_error(), Eq(false));
    EXPECT_EQ(publisherPort.value()->m_serviceDescription, m_serviceDescription);
    EXPECT_EQ(publisherPort.value()->m_runtimeName, m_applicationName);
    EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_historyCapacity, m_publisherOptions.historyCapacity);
    EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
    EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
}

TEST_F(PortPool_test, AddPublisherPortWithMaxCapacityIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "3328692a-77a7-42d4-8ec2-154e1e89f8cd");
    for (uint32_t i = 0U; i < MAX_PUBLISHERS; ++i)
    {
        RuntimeName_t applicationName = into<lossy<RuntimeName_t>>("AppName" + convert::toString(i));

        auto publisherPort = sut.addPublisherPort(
            m_serviceDescription, &m_memoryManager, applicationName, m_publisherOptions, m_memoryInfo);

        ASSERT_THAT(publisherPort.has_error(), Eq(false));
        EXPECT_EQ(publisherPort.value()->m_serviceDescription, m_serviceDescription);
        EXPECT_EQ(publisherPort.value()->m_runtimeName, applicationName);
        EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_historyCapacity, m_publisherOptions.historyCapacity);
        EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
        EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
    }
}

TEST_F(PortPool_test, AddPublisherPortWhenPublisherListOverflowsReturnsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "a0dcb81c-d7cf-448a-bb6d-5c7feaa7da6c");
    auto addPublisherPort = [&](const uint32_t i) -> bool {
        std::string service = "service" + convert::toString(i);
        std::string instance = "instance" + convert::toString(i);
        RuntimeName_t applicationName = into<lossy<RuntimeName_t>>("AppName" + convert::toString(i));

        return sut
            .addPublisherPort({into<lossy<IdString_t>>(service), into<lossy<IdString_t>>(instance), "foo"},
                              &m_memoryManager,
                              applicationName,
                              m_publisherOptions)
            .has_error();
    };

    for (uint32_t i = 0U; i < MAX_PUBLISHERS; ++i)
    {
        EXPECT_FALSE(addPublisherPort(i));
    }

    IOX_TESTING_EXPECT_OK();

    EXPECT_TRUE(addPublisherPort(MAX_PUBLISHERS));

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::PORT_POOL__PUBLISHERLIST_OVERFLOW);
}

TEST_F(PortPool_test, GetPublisherPortDataListIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "1650a6e0-8079-4ac4-ad03-723a7fc70217");

    EXPECT_EQ(sut.getPublisherPortDataList().size(), 0U);

    ASSERT_FALSE(sut.addPublisherPort(m_serviceDescription, &m_memoryManager, m_applicationName, m_publisherOptions)
                     .has_error());

    EXPECT_EQ(sut.getPublisherPortDataList().size(), 1U);
}

TEST_F(PortPool_test, GetPublisherPortDataListWhenEmptyIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "01fc41aa-4961-4bb6-98b7-a35ca3f93c1d");

    EXPECT_EQ(sut.getPublisherPortDataList().size(), 0U);
}

TEST_F(PortPool_test, GetPublisherPortDataListCompletelyFilledSuccessfully)
{
    ::testing::Test::RecordProperty("TEST_ID", "1e0c7c72-6a67-4481-8873-afba04850d03");
    for (uint32_t i = 0U; i < MAX_PUBLISHERS; ++i)
    {
        std::string service = "service" + convert::toString(i);
        std::string instance = "instance" + convert::toString(i);
        RuntimeName_t applicationName = into<lossy<RuntimeName_t>>("AppName" + convert::toString(i));

        ASSERT_FALSE(sut.addPublisherPort({into<lossy<IdString_t>>(service), into<lossy<IdString_t>>(instance), "foo"},
                                          &m_memoryManager,
                                          applicationName,
                                          m_publisherOptions)
                         .has_error());
    }

    EXPECT_EQ(sut.getPublisherPortDataList().size(), MAX_PUBLISHERS);
}

TEST_F(PortPool_test, RemovePublisherPortIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "7c0e06c2-e07b-468e-bb95-1dcad99e29b4");
    auto publisherPort =
        sut.addPublisherPort(m_serviceDescription, &m_memoryManager, m_applicationName, m_publisherOptions);
    sut.removePublisherPort(publisherPort.value());

    EXPECT_EQ(sut.getPublisherPortDataList().size(), 0U);
}

// END PublisherPort tests

// BEGIN SubscriberPort tests

TEST_F(PortPool_test, AddSubscriberPortIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "b4703d69-bec1-49cf-8f7b-00e805577d8f");
    auto subscriberPort =
        sut.addSubscriberPort(m_serviceDescription, m_applicationName, m_subscriberOptions, m_memoryInfo);

    ASSERT_THAT(subscriberPort.has_error(), Eq(false));
    EXPECT_EQ(subscriberPort.value()->m_serviceDescription, m_serviceDescription);
    EXPECT_EQ(subscriberPort.value()->m_runtimeName, m_applicationName);
    EXPECT_EQ(subscriberPort.value()->m_options.historyRequest, m_subscriberOptions.historyRequest);
    EXPECT_EQ(subscriberPort.value()->m_chunkReceiverData.m_queue.capacity(), 256U);
    EXPECT_EQ(subscriberPort.value()->m_chunkReceiverData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
    EXPECT_EQ(subscriberPort.value()->m_chunkReceiverData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
}

TEST_F(PortPool_test, AddSubscriberPortToMaxCapacityIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "380fa9e5-8cf3-435f-ad33-04bc706a37a5");
    for (uint32_t i = 0U; i < MAX_SUBSCRIBERS; ++i)
    {
        std::string service = "service" + convert::toString(i);
        std::string instance = "instance" + convert::toString(i);
        RuntimeName_t applicationName = into<lossy<RuntimeName_t>>("AppName" + convert::toString(i));


        auto subscriberPort =
            sut.addSubscriberPort(m_serviceDescription, applicationName, m_subscriberOptions, m_memoryInfo);

        ASSERT_THAT(subscriberPort.has_error(), Eq(false));
        EXPECT_EQ(subscriberPort.value()->m_serviceDescription, m_serviceDescription);
        EXPECT_EQ(subscriberPort.value()->m_runtimeName, applicationName);
        EXPECT_EQ(subscriberPort.value()->m_chunkReceiverData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
        EXPECT_EQ(subscriberPort.value()->m_chunkReceiverData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
    }
}


TEST_F(PortPool_test, AddSubscriberPortWhenSubscriberListOverflowsReturnsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "f7f13463-84d3-4434-ac43-5ee04e37b57f");
    auto addSubscriberPort = [&](const uint32_t i) -> bool {
        std::string service = "service" + convert::toString(i);
        std::string instance = "instance" + convert::toString(i);
        RuntimeName_t applicationName = into<lossy<RuntimeName_t>>("AppName" + convert::toString(i));


        auto publisherPort =
            sut.addSubscriberPort({into<lossy<IdString_t>>(service), into<lossy<IdString_t>>(instance), "foo"},
                                  applicationName,
                                  m_subscriberOptions);
        return publisherPort.has_error();
    };

    for (uint32_t i = 0U; i < MAX_SUBSCRIBERS; ++i)
    {
        EXPECT_FALSE(addSubscriberPort(i));
    }

    IOX_TESTING_EXPECT_OK();

    EXPECT_TRUE(addSubscriberPort(MAX_SUBSCRIBERS));

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::PORT_POOL__SUBSCRIBERLIST_OVERFLOW);
}

TEST_F(PortPool_test, GetSubscriberPortDataListIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "391bba2f-e6f7-4dec-9ffb-67a69cd9a059");
    auto subscriberPort = sut.addSubscriberPort(m_serviceDescription, m_applicationName, m_subscriberOptions);
    EXPECT_FALSE(subscriberPort.has_error());

    ASSERT_EQ(sut.getSubscriberPortDataList().size(), 1U);
}

TEST_F(PortPool_test, GetSubscriberPortDataListWhenEmptyIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "a525a8b7-98f3-4c01-a85f-c8c7cc741e09");

    ASSERT_EQ(sut.getSubscriberPortDataList().size(), 0U);
}

TEST_F(PortPool_test, GetSubscriberPortDataListCompletelyFilledIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "8c1e32ba-74e2-4c34-ae37-4c0d93b21283");
    for (uint32_t i = 0U; i < MAX_SUBSCRIBERS; ++i)
    {
        std::string service = "service" + convert::toString(i);
        std::string instance = "instance" + convert::toString(i);
        RuntimeName_t applicationName = into<lossy<RuntimeName_t>>("AppName" + convert::toString(i));

        auto publisherPort =
            sut.addSubscriberPort({into<lossy<IdString_t>>(service), into<lossy<IdString_t>>(instance), "foo"},
                                  applicationName,
                                  m_subscriberOptions);
        EXPECT_FALSE(publisherPort.has_error());
    }

    ASSERT_EQ(sut.getSubscriberPortDataList().size(), MAX_SUBSCRIBERS);
}

TEST_F(PortPool_test, RemoveSubscriberPortIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "1f642bba-561c-45bb-bb7a-b0f8b373483a");
    auto subscriberPort = sut.addSubscriberPort(m_serviceDescription, m_applicationName, m_subscriberOptions);

    sut.removeSubscriberPort(subscriberPort.value());

    EXPECT_EQ(sut.getSubscriberPortDataList().size(), 0U);
}

// END SubscriberPort tests

// BEGIN ClientPort tests

TEST_F(PortPool_test, AddClientPortIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "47d9cd34-22a6-480a-8595-d4abf46df428");
    constexpr uint32_t NUMBER_OF_CLIENTS_TO_ADD{1U};
    auto addSuccessful =
        addClientPorts(NUMBER_OF_CLIENTS_TO_ADD, [&](const auto& sd, const auto& runtimeName, const auto& clientPort) {
            EXPECT_EQ(clientPort.m_serviceDescription, sd);
            EXPECT_EQ(clientPort.m_runtimeName, runtimeName);
            EXPECT_EQ(clientPort.m_connectRequested.load(), m_clientOptions.connectOnCreate);
            EXPECT_EQ(clientPort.m_connectionState.load(), ConnectionState::NOT_CONNECTED);
            EXPECT_EQ(clientPort.m_chunkReceiverData.m_queue.capacity(), QUEUE_CAPACITY);
            EXPECT_EQ(clientPort.m_chunkReceiverData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
            EXPECT_EQ(clientPort.m_chunkReceiverData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
            EXPECT_EQ(clientPort.m_chunkSenderData.m_historyCapacity, popo::ClientPortData::HISTORY_CAPACITY_ZERO);
            EXPECT_EQ(clientPort.m_chunkSenderData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
            EXPECT_EQ(clientPort.m_chunkSenderData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
        });

    EXPECT_TRUE(addSuccessful);
}

TEST_F(PortPool_test, AddClientPortToMaxCapacityIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "f8ee6f26-fdac-4bfd-9e28-46362e4359e9");
    constexpr uint32_t NUMBER_OF_CLIENTS_TO_ADD{MAX_CLIENTS};
    auto addSuccessful =
        addClientPorts(NUMBER_OF_CLIENTS_TO_ADD, [&](const auto& sd, const auto&, const auto& clientPort) {
            EXPECT_EQ(clientPort.m_serviceDescription, sd);
        });

    EXPECT_TRUE(addSuccessful);
}


TEST_F(PortPool_test, AddClientPortWhenClientListOverflowsReturnsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "98c47d42-5f75-42a3-84b5-b97e72a17992");
    constexpr uint32_t NUMBER_OF_CLIENTS_TO_ADD{MAX_CLIENTS};
    auto addSuccessful = addClientPorts(NUMBER_OF_CLIENTS_TO_ADD, [&](const auto&, const auto&, const auto&) {});

    EXPECT_TRUE(addSuccessful);
    IOX_TESTING_EXPECT_OK();

    constexpr uint32_t ONE_MORE_CLIENT{1U};
    auto additionalAddSuccessful = addClientPorts(ONE_MORE_CLIENT, [&](const auto&, const auto&, const auto&) {});

    EXPECT_FALSE(additionalAddSuccessful);

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::PORT_POOL__CLIENTLIST_OVERFLOW);
}

TEST_F(PortPool_test, GetClientPortDataListIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "39119f21-ca97-4320-a805-029927a79372");
    constexpr uint32_t NUMBER_OF_CLIENTS_TO_ADD{1U};
    auto addSuccessful = addClientPorts(NUMBER_OF_CLIENTS_TO_ADD, [&](const auto&, const auto&, const auto&) {});
    EXPECT_TRUE(addSuccessful);

    ASSERT_EQ(sut.getClientPortDataList().size(), NUMBER_OF_CLIENTS_TO_ADD);
}

TEST_F(PortPool_test, GetClientPortDataListWhenEmptyIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "6c08ae7d-1eed-46d6-b363-b2dc294d0e0e");

    ASSERT_EQ(sut.getClientPortDataList().size(), 0U);
}

TEST_F(PortPool_test, GetClientPortDataListCompletelyFilledIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "cdac1dca-f438-4816-90f9-ca976b6ccd88");
    constexpr uint32_t NUMBER_OF_CLIENTS_TO_ADD{MAX_CLIENTS};
    auto addSuccessful = addClientPorts(NUMBER_OF_CLIENTS_TO_ADD, [&](const auto&, const auto&, const auto&) {});
    EXPECT_TRUE(addSuccessful);

    ASSERT_EQ(sut.getClientPortDataList().size(), MAX_CLIENTS);
}

TEST_F(PortPool_test, RemoveClientPortIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "d93ecaef-555a-4db4-a49d-390366457f97");
    constexpr uint32_t NUMBER_OF_CLIENTS_TO_ADD{1U};
    auto addSuccessful =
        addClientPorts(NUMBER_OF_CLIENTS_TO_ADD,
                       [&](const auto&, const auto&, const auto& clientPort) { sut.removeClientPort(&clientPort); });
    EXPECT_TRUE(addSuccessful);

    EXPECT_EQ(sut.getClientPortDataList().size(), 0U);
}

// END ClientPort tests

// BEGIN ServerPort tests

TEST_F(PortPool_test, AddServerPortIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "ff0a77a0-5a60-460e-ba3c-f9c5669b7086");
    constexpr uint32_t NUMBER_OF_SERVERS_TO_ADD{1U};
    auto addSuccessful =
        addServerPorts(NUMBER_OF_SERVERS_TO_ADD, [&](const auto& sd, const auto& runtimeName, const auto& serverPort) {
            EXPECT_EQ(serverPort.m_serviceDescription, sd);
            EXPECT_EQ(serverPort.m_runtimeName, runtimeName);
            EXPECT_EQ(serverPort.m_offeringRequested.load(), m_serverOptions.offerOnCreate);
            EXPECT_EQ(serverPort.m_offered.load(), false);
            EXPECT_EQ(serverPort.m_chunkReceiverData.m_queue.capacity(), QUEUE_CAPACITY);
            EXPECT_EQ(serverPort.m_chunkReceiverData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
            EXPECT_EQ(serverPort.m_chunkReceiverData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
            EXPECT_EQ(serverPort.m_chunkSenderData.m_historyCapacity, popo::ServerPortData::HISTORY_REQUEST_OF_ZERO);
            EXPECT_EQ(serverPort.m_chunkSenderData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
            EXPECT_EQ(serverPort.m_chunkSenderData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
        });

    EXPECT_TRUE(addSuccessful);
}

TEST_F(PortPool_test, AddServerPortToMaxCapacityIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "496021f9-5ec3-4b1c-a551-8a0d50d0ac8f");
    constexpr uint32_t NUMBER_OF_SERVERS_TO_ADD{MAX_SERVERS};
    auto addSuccessful =
        addServerPorts(NUMBER_OF_SERVERS_TO_ADD, [&](const auto& sd, const auto&, const auto& serverPort) {
            EXPECT_EQ(serverPort.m_serviceDescription, sd);
        });

    EXPECT_TRUE(addSuccessful);
}


TEST_F(PortPool_test, AddServerPortWhenServerListOverflowsReturnsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "744b3d73-b2d2-49cf-a748-e13dc6f3b06c");
    constexpr uint32_t NUMBER_OF_SERVERS_TO_ADD{MAX_SERVERS};
    auto addSuccessful = addServerPorts(NUMBER_OF_SERVERS_TO_ADD, [&](const auto&, const auto&, const auto&) {});

    EXPECT_TRUE(addSuccessful);

    constexpr uint32_t ONE_MORE_SERVER{1U};
    auto additionalAddSuccessful = addServerPorts(ONE_MORE_SERVER, [&](const auto&, const auto&, const auto&) {});

    EXPECT_FALSE(additionalAddSuccessful);

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::PORT_POOL__SERVERLIST_OVERFLOW);
}

TEST_F(PortPool_test, GetServerPortDataListIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "d30fa67c-7f7d-43f1-a7bc-599e5668ab65");
    constexpr uint32_t NUMBER_OF_SERVERS_TO_ADD{1U};
    auto addSuccessful = addServerPorts(NUMBER_OF_SERVERS_TO_ADD, [&](const auto&, const auto&, const auto&) {});
    EXPECT_TRUE(addSuccessful);

    ASSERT_EQ(sut.getServerPortDataList().size(), NUMBER_OF_SERVERS_TO_ADD);
}

TEST_F(PortPool_test, GetServerPortDataListWhenEmptyIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "d1b32417-caeb-4a5c-ae40-49d651b418cd");

    ASSERT_EQ(sut.getServerPortDataList().size(), 0U);
}

TEST_F(PortPool_test, GetServerPortDataListCompletelyFilledIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "2968e43d-6972-4667-82f6-7762d479a729");
    constexpr uint32_t NUMBER_OF_SERVERS_TO_ADD{MAX_SERVERS};
    auto addSuccessful = addServerPorts(NUMBER_OF_SERVERS_TO_ADD, [&](const auto&, const auto&, const auto&) {});
    EXPECT_TRUE(addSuccessful);

    ASSERT_EQ(sut.getServerPortDataList().size(), MAX_SERVERS);
}

TEST_F(PortPool_test, RemoveServerPortIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "b140e3bf-0ddf-4e1a-824b-a4935596f371");
    constexpr uint32_t NUMBER_OF_SERVERS_TO_ADD{1U};
    auto addSuccessful =
        addServerPorts(NUMBER_OF_SERVERS_TO_ADD,
                       [&](const auto&, const auto&, const auto& serverPort) { sut.removeServerPort(&serverPort); });
    EXPECT_TRUE(addSuccessful);

    EXPECT_EQ(sut.getServerPortDataList().size(), 0U);
}

// END ServerPort tests

// BEGIN InterfacePort tests

TEST_F(PortPool_test, AddInterfacePortIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "28116302-dc19-4927-aab4-6d03c9befd88");
    auto interfacePortData = sut.addInterfacePort(m_applicationName, Interfaces::INTERNAL);

    ASSERT_THAT(interfacePortData.has_error(), Eq(false));
    EXPECT_EQ(interfacePortData.value()->m_runtimeName, m_applicationName);
    EXPECT_EQ(interfacePortData.value()->m_serviceDescription.getSourceInterface(), Interfaces::INTERNAL);
}

TEST_F(PortPool_test, AddInterfacePortWithMaxCapacityIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "8f7690e5-c29e-4e7e-bc9d-f6ea61c3fd6c");
    for (uint32_t i = 1U; i <= MAX_INTERFACE_NUMBER; ++i)
    {
        auto interfacePortData = sut.addInterfacePort(m_applicationName, Interfaces::INTERNAL);
        ASSERT_THAT(interfacePortData.has_error(), Eq(false));
    }

    EXPECT_EQ(sut.getInterfacePortDataList().size(), MAX_INTERFACE_NUMBER);
}

TEST_F(PortPool_test, AddInterfacePortWhenInterfaceListOverflowsReturnsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "a35f7b3b-4b21-4bff-bc40-693a7064ffc5");
    for (uint32_t i = 0U; i < MAX_INTERFACE_NUMBER; ++i)
    {
        EXPECT_FALSE(sut.addInterfacePort(m_applicationName, Interfaces::INTERFACE_END).has_error());
    }

    EXPECT_TRUE(sut.addInterfacePort(m_applicationName, Interfaces::INTERFACE_END).has_error());

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::PORT_POOL__INTERFACELIST_OVERFLOW);
}

TEST_F(PortPool_test, GetInterfacePortDataListIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "0ed6bf52-2ffb-40f4-acab-a9f79532cde1");
    auto interfacePort = sut.addInterfacePort(m_applicationName, Interfaces::INTERNAL);
    EXPECT_FALSE(interfacePort.has_error());

    ASSERT_EQ(sut.getInterfacePortDataList().size(), 1U);
}

TEST_F(PortPool_test, GetInterfacePortDataListWhenEmptyIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "80aab75f-5251-4c2e-9ab6-82b00c728a9c");

    ASSERT_EQ(sut.getInterfacePortDataList().size(), 0U);
}

TEST_F(PortPool_test, GetInterfacePortDataListCompletelyFilledIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "460703f9-72d8-4b72-9c3a-761be22e6c9a");
    for (uint32_t i = 0U; i < MAX_INTERFACE_NUMBER; ++i)
    {
        RuntimeName_t applicationName = into<lossy<RuntimeName_t>>("AppName" + convert::toString(i));
        ASSERT_FALSE(sut.addInterfacePort(applicationName, Interfaces::INTERNAL).has_error());
    }

    ASSERT_EQ(sut.getInterfacePortDataList().size(), MAX_INTERFACE_NUMBER);
}

TEST_F(PortPool_test, RemoveInterfacePortIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "65db995b-46b1-44f5-bef0-09096faa4953");
    auto interfacePort = sut.addInterfacePort(m_applicationName, Interfaces::INTERNAL);

    sut.removeInterfacePort(interfacePort.value());

    ASSERT_EQ(sut.getInterfacePortDataList().size(), 0U);
}

// END InterfacePort tests

// BEGIN ConditionVariable tests

TEST_F(PortPool_test, AddConditionVariableDataIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "08021def-be31-42f2-855f-38cac6120c3f");
    auto conditionVariableData = sut.addConditionVariableData(m_applicationName);

    ASSERT_THAT(conditionVariableData.has_error(), Eq(false));
    EXPECT_EQ(conditionVariableData.value()->m_runtimeName, m_applicationName);
}

TEST_F(PortPool_test, AddConditionVariableDataWithMaxCapacityIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "ee38306e-93b6-4d07-a719-e2e169801f17");
    for (uint32_t i = 1U; i <= MAX_NUMBER_OF_CONDITION_VARIABLES; ++i)
    {
        auto conditionVariableData = sut.addConditionVariableData(m_applicationName);
        ASSERT_THAT(conditionVariableData.has_error(), Eq(false));
    }

    EXPECT_EQ(sut.getConditionVariableDataList().size(), MAX_NUMBER_OF_CONDITION_VARIABLES);
}

TEST_F(PortPool_test, AddConditionVariableDataWhenContainerIsFullReturnsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "6d1d351a-6a5e-47f2-8042-9a5f8d8a650d");
    for (uint32_t i = 0U; i < MAX_NUMBER_OF_CONDITION_VARIABLES; ++i)
    {
        EXPECT_FALSE(sut.addConditionVariableData(m_applicationName).has_error());
    }

    EXPECT_TRUE(sut.addConditionVariableData(m_applicationName).has_error());

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::PORT_POOL__CONDITION_VARIABLE_LIST_OVERFLOW);
}

TEST_F(PortPool_test, GetConditionVariableDataListIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "b128487c-f808-4eef-9c74-7ddeab5415d9");
    ASSERT_FALSE(sut.addConditionVariableData(m_applicationName).has_error());

    ASSERT_EQ(sut.getConditionVariableDataList().size(), 1U);
}

TEST_F(PortPool_test, GetConditionVariableDataListWhenEmptyIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "f70cc08d-9a50-4166-acfc-b2514bd7f571");

    ASSERT_EQ(sut.getConditionVariableDataList().size(), 0U);
}

TEST_F(PortPool_test, GetConditionVariableDataListCompletelyFilledIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "42c58990-4dbe-485f-bbf6-7430cc878118");
    for (uint32_t i = 0U; i < MAX_NUMBER_OF_CONDITION_VARIABLES; ++i)
    {
        RuntimeName_t applicationName = into<lossy<RuntimeName_t>>("AppName" + convert::toString(i));
        ASSERT_FALSE(sut.addConditionVariableData(applicationName).has_error());
    }

    ASSERT_EQ(sut.getConditionVariableDataList().size(), MAX_NUMBER_OF_CONDITION_VARIABLES);
}

TEST_F(PortPool_test, RemoveConditionVariableDataIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "7d876157-4d02-4374-ae16-fbff32ff683a");
    auto conditionVariableData = sut.addConditionVariableData(m_applicationName);

    sut.removeConditionVariableData(conditionVariableData.value());

    ASSERT_EQ(sut.getConditionVariableDataList().size(), 0U);
}

// END ConditionVariable tests

} // namespace
