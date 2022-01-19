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

#include "iceoryx_hoofs/cxx/convert.hpp"
#include "iceoryx_posh/internal/roudi/port_pool_data.hpp"
#include "iceoryx_posh/internal/runtime/node_data.hpp"
#include "iceoryx_posh/popo/subscriber_options.hpp"
#include "iceoryx_posh/roudi/port_pool.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::capro;
using namespace iox;

static constexpr uint32_t DEFAULT_DEVICE_ID{20U};
static constexpr uint32_t DEFAULT_MEMORY_TYPE{100U};
class PortPool_test : public Test
{
  public:
    roudi::PortPoolData m_portPoolData;
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
    iox::mepoo::MemoryInfo m_memoryInfo{DEFAULT_DEVICE_ID, DEFAULT_MEMORY_TYPE};
};

TEST_F(PortPool_test, AddNodeDataIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "a917fe3d-08a4-4c8f-83a5-4b99b915c0dd");
    auto nodeData = sut.addNodeData(m_runtimeName, m_nodeName, m_nodeDeviceId);

    ASSERT_THAT(nodeData.has_error(), Eq(false));
    EXPECT_EQ(nodeData.value()->m_runtimeName, m_runtimeName);
    EXPECT_EQ(nodeData.value()->m_nodeName, m_nodeName);
    EXPECT_EQ(nodeData.value()->m_nodeDeviceIdentifier, m_nodeDeviceId);
}

TEST_F(PortPool_test, AddNodeDataWithMaxCapacityIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "1553d03e-4154-49e8-9f38-db6f52e6fa29");
    for (uint32_t i = 1U; i <= MAX_NODE_NUMBER; ++i)
    {
        auto nodeData = sut.addNodeData(m_runtimeName, m_nodeName, i);

        ASSERT_THAT(nodeData.has_error(), Eq(false));
    }

    EXPECT_EQ(sut.getNodeDataList().size(), MAX_NODE_NUMBER);
}


TEST_F(PortPool_test, AddNodeDataWhenNodeListIsFullReturnsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "23ff8250-6c1b-4a4e-b3e6-207883386edc");
    for (uint32_t i = 0U; i < MAX_NODE_NUMBER; ++i)
    {
        ASSERT_FALSE(sut.addNodeData(m_runtimeName, m_nodeName, i).has_error());
    }

    auto errorHandlerCalled{false};
    Error errorHandlerType;
    auto errorHandlerGuard = ErrorHandler<iox::Error>::setTemporaryErrorHandler(
        [&](const Error error, const std::function<void()>, const ErrorLevel) {
            errorHandlerType = error;
            errorHandlerCalled = true;
        });

    ASSERT_TRUE(sut.addNodeData(m_runtimeName, m_nodeName, MAX_NODE_NUMBER).has_error());

    EXPECT_TRUE(errorHandlerCalled);
    EXPECT_EQ(errorHandlerType, Error::kPORT_POOL__NODELIST_OVERFLOW);
}

TEST_F(PortPool_test, GetNodeDataListIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "5a86e0ed-e61a-4f45-9aab-2b38a22730a9");
    ASSERT_FALSE(sut.addNodeData(m_runtimeName, m_nodeName, m_nodeDeviceId).has_error());

    auto nodeDataList = sut.getNodeDataList();

    EXPECT_EQ(nodeDataList.size(), 1U);
    EXPECT_EQ(nodeDataList[0]->m_runtimeName, m_runtimeName);
    EXPECT_EQ(nodeDataList[0]->m_nodeName, m_nodeName);
    EXPECT_EQ(nodeDataList[0]->m_nodeDeviceIdentifier, m_nodeDeviceId);
}

TEST_F(PortPool_test, GetNodeDataListWhenEmptyIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "c5f629bd-b9ea-4d41-b991-5654e20dae3b");
    auto nodeDataList = sut.getNodeDataList();

    EXPECT_EQ(nodeDataList.size(), 0U);
}

TEST_F(PortPool_test, GetNodeDataListWithMaxCapacityIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "6a43182d-f0fd-4cbf-931e-f803a0236180");
    for (uint32_t i = 1U; i <= MAX_NODE_NUMBER; ++i)
    {
        auto nodeData = sut.addNodeData(m_runtimeName, m_nodeName, i);

        ASSERT_THAT(nodeData.has_error(), Eq(false));
    }

    auto nodeDataList = sut.getNodeDataList();

    EXPECT_EQ(nodeDataList.size(), MAX_NODE_NUMBER);
}

TEST_F(PortPool_test, RemoveNodeDataIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "b300e7a6-df97-417d-9f7a-df3b55dace41");
    auto nodeData = sut.addNodeData(m_runtimeName, m_nodeName, m_nodeDeviceId);

    sut.removeNodeData(nodeData.value());
    auto nodeDataList = sut.getNodeDataList();

    EXPECT_EQ(nodeDataList.size(), 0U);
}

TEST_F(PortPool_test, AddPublisherPortIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "8e2271f5-65a9-41ea-bffa-7f1a55321cc0");
    auto publisherPort = sut.addPublisherPort(
        m_serviceDescription, &m_memoryManager, m_applicationName, m_publisherOptions, m_memoryInfo);

    ASSERT_THAT(publisherPort.has_error(), Eq(false));
    EXPECT_EQ(publisherPort.value()->m_serviceDescription, m_serviceDescription);
    EXPECT_EQ(publisherPort.value()->m_runtimeName, m_applicationName);
    EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_historyCapacity, m_publisherOptions.historyCapacity);
    EXPECT_EQ(publisherPort.value()->m_nodeName, m_publisherOptions.nodeName);
    EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
    EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
}

TEST_F(PortPool_test, AddPublisherPortWithMaxCapacityIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "3328692a-77a7-42d4-8ec2-154e1e89f8cd");
    for (uint32_t i = 0U; i < MAX_PUBLISHERS; ++i)
    {
        RuntimeName_t applicationName = {cxx::TruncateToCapacity, "AppName" + cxx::convert::toString(i)};

        auto publisherPort = sut.addPublisherPort(
            m_serviceDescription, &m_memoryManager, applicationName, m_publisherOptions, m_memoryInfo);

        ASSERT_THAT(publisherPort.has_error(), Eq(false));
        EXPECT_EQ(publisherPort.value()->m_serviceDescription, m_serviceDescription);
        EXPECT_EQ(publisherPort.value()->m_runtimeName, applicationName);
        EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_historyCapacity, m_publisherOptions.historyCapacity);
        EXPECT_EQ(publisherPort.value()->m_nodeName, m_publisherOptions.nodeName);
        EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
        EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
    }
}

TEST_F(PortPool_test, AddPublisherPortWhenPublisherListOverflowsReturnsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "a0dcb81c-d7cf-448a-bb6d-5c7feaa7da6c");
    auto addPublisherPort = [&](const uint32_t i) -> bool {
        std::string service = "service" + cxx::convert::toString(i);
        std::string instance = "instance" + cxx::convert::toString(i);
        RuntimeName_t applicationName = {cxx::TruncateToCapacity, "AppName" + cxx::convert::toString(i)};

        return sut
            .addPublisherPort(
                {IdString_t(cxx::TruncateToCapacity, service), IdString_t(cxx::TruncateToCapacity, instance), "foo"},
                &m_memoryManager,
                applicationName,
                m_publisherOptions)
            .has_error();
    };

    for (uint32_t i = 0U; i < MAX_PUBLISHERS; ++i)
    {
        EXPECT_FALSE(addPublisherPort(i));
    }

    auto errorHandlerCalled{false};
    Error errorHandlerType;
    auto errorHandlerGuard = ErrorHandler<iox::Error>::setTemporaryErrorHandler(
        [&](const Error error, const std::function<void()>, const ErrorLevel) {
            errorHandlerType = error;
            errorHandlerCalled = true;
        });

    EXPECT_TRUE(addPublisherPort(MAX_PUBLISHERS));

    EXPECT_TRUE(errorHandlerCalled);
    EXPECT_EQ(errorHandlerType, Error::kPORT_POOL__PUBLISHERLIST_OVERFLOW);
}

TEST_F(PortPool_test, GetPublisherPortDataListIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "1650a6e0-8079-4ac4-ad03-723a7fc70217");
    auto publisherPortDataList = sut.getPublisherPortDataList();

    EXPECT_EQ(publisherPortDataList.size(), 0U);

    ASSERT_FALSE(sut.addPublisherPort(m_serviceDescription, &m_memoryManager, m_applicationName, m_publisherOptions)
                     .has_error());
    publisherPortDataList = sut.getPublisherPortDataList();

    EXPECT_EQ(publisherPortDataList.size(), 1U);
}

TEST_F(PortPool_test, GetPublisherPortDataListWhenEmptyIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "01fc41aa-4961-4bb6-98b7-a35ca3f93c1d");
    auto nodeDataList = sut.getPublisherPortDataList();

    EXPECT_EQ(nodeDataList.size(), 0U);
}

TEST_F(PortPool_test, GetPublisherPortDataListCompletelyFilledSuccessfully)
{
    ::testing::Test::RecordProperty("TEST_ID", "1e0c7c72-6a67-4481-8873-afba04850d03");
    for (uint32_t i = 0U; i < MAX_PUBLISHERS; ++i)
    {
        std::string service = "service" + cxx::convert::toString(i);
        std::string instance = "instance" + cxx::convert::toString(i);
        RuntimeName_t applicationName = {cxx::TruncateToCapacity, "AppName" + cxx::convert::toString(i)};

        ASSERT_FALSE(
            sut.addPublisherPort(
                   {IdString_t(cxx::TruncateToCapacity, service), IdString_t(cxx::TruncateToCapacity, instance), "foo"},
                   &m_memoryManager,
                   applicationName,
                   m_publisherOptions)
                .has_error());
    }

    auto publisherPortDataList = sut.getPublisherPortDataList();

    EXPECT_EQ(publisherPortDataList.size(), MAX_PUBLISHERS);
}

TEST_F(PortPool_test, RemovePublisherPortIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "7c0e06c2-e07b-468e-bb95-1dcad99e29b4");
    auto publisherPort =
        sut.addPublisherPort(m_serviceDescription, &m_memoryManager, m_applicationName, m_publisherOptions);
    sut.removePublisherPort(publisherPort.value());
    auto publisherPortDataList = sut.getPublisherPortDataList();

    EXPECT_EQ(publisherPortDataList.size(), 0U);
}

TEST_F(PortPool_test, AddSubscriberPortIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "b4703d69-bec1-49cf-8f7b-00e805577d8f");
    auto subscriberPort =
        sut.addSubscriberPort(m_serviceDescription, m_applicationName, m_subscriberOptions, m_memoryInfo);

    ASSERT_THAT(subscriberPort.has_error(), Eq(false));
    EXPECT_EQ(subscriberPort.value()->m_serviceDescription, m_serviceDescription);
    EXPECT_EQ(subscriberPort.value()->m_runtimeName, m_applicationName);
    EXPECT_EQ(subscriberPort.value()->m_nodeName, m_subscriberOptions.nodeName);
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
        std::string service = "service" + cxx::convert::toString(i);
        std::string instance = "instance" + cxx::convert::toString(i);
        RuntimeName_t applicationName = {cxx::TruncateToCapacity, "AppName" + cxx::convert::toString(i)};


        auto subscriberPort =
            sut.addSubscriberPort(m_serviceDescription, applicationName, m_subscriberOptions, m_memoryInfo);

        ASSERT_THAT(subscriberPort.has_error(), Eq(false));
        EXPECT_EQ(subscriberPort.value()->m_serviceDescription, m_serviceDescription);
        EXPECT_EQ(subscriberPort.value()->m_runtimeName, applicationName);
        EXPECT_EQ(subscriberPort.value()->m_nodeName, m_subscriberOptions.nodeName);
        EXPECT_EQ(subscriberPort.value()->m_chunkReceiverData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
        EXPECT_EQ(subscriberPort.value()->m_chunkReceiverData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
    }
}


TEST_F(PortPool_test, AddSubscriberPortWhenSubscriberListOverflowsReturnsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "f7f13463-84d3-4434-ac43-5ee04e37b57f");
    auto addSubscriberPort = [&](const uint32_t i) -> bool {
        std::string service = "service" + cxx::convert::toString(i);
        std::string instance = "instance" + cxx::convert::toString(i);
        RuntimeName_t applicationName = {cxx::TruncateToCapacity, "AppName" + cxx::convert::toString(i)};


        auto publisherPort = sut.addSubscriberPort(
            {IdString_t(cxx::TruncateToCapacity, service), IdString_t(cxx::TruncateToCapacity, instance), "foo"},
            applicationName,
            m_subscriberOptions);
        return publisherPort.has_error();
    };

    for (uint32_t i = 0U; i < MAX_SUBSCRIBERS; ++i)
    {
        EXPECT_FALSE(addSubscriberPort(i));
    }

    auto errorHandlerCalled{false};
    Error errorHandlerType;
    auto errorHandlerGuard = ErrorHandler<iox::Error>::setTemporaryErrorHandler(
        [&](const Error error, const std::function<void()>, const ErrorLevel) {
            errorHandlerType = error;
            errorHandlerCalled = true;
        });
    EXPECT_TRUE(addSubscriberPort(MAX_SUBSCRIBERS));

    EXPECT_TRUE(errorHandlerCalled);
    EXPECT_EQ(errorHandlerType, Error::kPORT_POOL__SUBSCRIBERLIST_OVERFLOW);
}

TEST_F(PortPool_test, GetSubscriberPortDataListIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "391bba2f-e6f7-4dec-9ffb-67a69cd9a059");
    auto subscriberPort = sut.addSubscriberPort(m_serviceDescription, m_applicationName, m_subscriberOptions);
    EXPECT_FALSE(subscriberPort.has_error());
    auto subscriberPortDataList = sut.getSubscriberPortDataList();

    ASSERT_EQ(subscriberPortDataList.size(), 1U);
}

TEST_F(PortPool_test, GetSubscriberPortDataListWhenEmptyIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "a525a8b7-98f3-4c01-a85f-c8c7cc741e09");
    auto nodeDataList = sut.getSubscriberPortDataList();

    ASSERT_EQ(nodeDataList.size(), 0U);
}

TEST_F(PortPool_test, GetSubscriberPortDataListCompletelyFilledIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "8c1e32ba-74e2-4c34-ae37-4c0d93b21283");
    for (uint32_t i = 0U; i < MAX_SUBSCRIBERS; ++i)
    {
        std::string service = "service" + cxx::convert::toString(i);
        std::string instance = "instance" + cxx::convert::toString(i);
        RuntimeName_t applicationName = {cxx::TruncateToCapacity, "AppName" + cxx::convert::toString(i)};

        auto publisherPort = sut.addSubscriberPort(
            {IdString_t(cxx::TruncateToCapacity, service), IdString_t(cxx::TruncateToCapacity, instance), "foo"},
            applicationName,
            m_subscriberOptions);
        EXPECT_FALSE(publisherPort.has_error());
    }
    auto subscriberPortDataList = sut.getSubscriberPortDataList();

    ASSERT_EQ(subscriberPortDataList.size(), MAX_SUBSCRIBERS);
}

TEST_F(PortPool_test, RemoveSubscriberPortIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "1f642bba-561c-45bb-bb7a-b0f8b373483a");
    auto subscriberPort = sut.addSubscriberPort(m_serviceDescription, m_applicationName, m_subscriberOptions);

    sut.removeSubscriberPort(subscriberPort.value());
    auto subscriberPortDataList = sut.getSubscriberPortDataList();

    EXPECT_EQ(subscriberPortDataList.size(), 0U);
}

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

    auto errorHandlerCalled{false};
    Error errorHandlerType;
    auto errorHandlerGuard = ErrorHandler<iox::Error>::setTemporaryErrorHandler(
        [&](const Error error, const std::function<void()>, const ErrorLevel) {
            errorHandlerType = error;
            errorHandlerCalled = true;
        });
    EXPECT_TRUE(sut.addInterfacePort(m_applicationName, Interfaces::INTERFACE_END).has_error());

    EXPECT_TRUE(errorHandlerCalled);
    EXPECT_EQ(errorHandlerType, Error::kPORT_POOL__INTERFACELIST_OVERFLOW);
}

TEST_F(PortPool_test, GetInterfacePortDataListIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "0ed6bf52-2ffb-40f4-acab-a9f79532cde1");
    auto interfacePort = sut.addInterfacePort(m_applicationName, Interfaces::INTERNAL);
    EXPECT_FALSE(interfacePort.has_error());
    auto interfacePortDataList = sut.getInterfacePortDataList();

    ASSERT_EQ(interfacePortDataList.size(), 1U);
}

TEST_F(PortPool_test, GetInterfacePortDataListWhenEmptyIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "80aab75f-5251-4c2e-9ab6-82b00c728a9c");
    auto interfacePortDataList = sut.getInterfacePortDataList();

    ASSERT_EQ(interfacePortDataList.size(), 0U);
}

TEST_F(PortPool_test, GetInterfacePortDataListCompletelyFilledIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "460703f9-72d8-4b72-9c3a-761be22e6c9a");
    for (uint32_t i = 0U; i < MAX_INTERFACE_NUMBER; ++i)
    {
        RuntimeName_t applicationName = {cxx::TruncateToCapacity, "AppName" + cxx::convert::toString(i)};
        ASSERT_FALSE(sut.addInterfacePort(applicationName, Interfaces::INTERNAL).has_error());
    }
    auto interfacePortDataList = sut.getInterfacePortDataList();

    ASSERT_EQ(interfacePortDataList.size(), MAX_INTERFACE_NUMBER);
}

TEST_F(PortPool_test, RemoveInterfacePortIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "65db995b-46b1-44f5-bef0-09096faa4953");
    auto interfacePort = sut.addInterfacePort(m_applicationName, Interfaces::INTERNAL);

    sut.removeInterfacePort(interfacePort.value());
    auto interfacePortDataList = sut.getInterfacePortDataList();

    ASSERT_EQ(interfacePortDataList.size(), 0U);
}

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

    auto errorHandlerCalled{false};
    Error errorHandlerType;
    auto errorHandlerGuard = ErrorHandler<iox::Error>::setTemporaryErrorHandler(
        [&](const Error error, const std::function<void()>, const ErrorLevel) {
            errorHandlerType = error;
            errorHandlerCalled = true;
        });
    EXPECT_TRUE(sut.addConditionVariableData(m_applicationName).has_error());

    EXPECT_TRUE(errorHandlerCalled);
    EXPECT_EQ(errorHandlerType, Error::kPORT_POOL__CONDITION_VARIABLE_LIST_OVERFLOW);
}

TEST_F(PortPool_test, GetConditionVariableDataListIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "b128487c-f808-4eef-9c74-7ddeab5415d9");
    ASSERT_FALSE(sut.addConditionVariableData(m_applicationName).has_error());
    auto condtionalVariableData = sut.getConditionVariableDataList();

    ASSERT_EQ(condtionalVariableData.size(), 1U);
}

TEST_F(PortPool_test, GetConditionVariableDataListWhenEmptyIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "f70cc08d-9a50-4166-acfc-b2514bd7f571");
    auto condtionalVariableData = sut.getConditionVariableDataList();

    ASSERT_EQ(condtionalVariableData.size(), 0U);
}

TEST_F(PortPool_test, GetConditionVariableDataListCompletelyFilledIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "42c58990-4dbe-485f-bbf6-7430cc878118");
    for (uint32_t i = 0U; i < MAX_NUMBER_OF_CONDITION_VARIABLES; ++i)
    {
        RuntimeName_t applicationName = {cxx::TruncateToCapacity, "AppName" + cxx::convert::toString(i)};
        ASSERT_FALSE(sut.addConditionVariableData(applicationName).has_error());
    }
    auto condtionalVariableData = sut.getConditionVariableDataList();

    ASSERT_EQ(condtionalVariableData.size(), MAX_NUMBER_OF_CONDITION_VARIABLES);
}

TEST_F(PortPool_test, RemoveConditionVariableDataIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "7d876157-4d02-4374-ae16-fbff32ff683a");
    auto conditionVariableData = sut.addConditionVariableData(m_applicationName);

    sut.removeConditionVariableData(conditionVariableData.value());
    auto condtionalVariableData = sut.getConditionVariableDataList();

    ASSERT_EQ(condtionalVariableData.size(), 0U);
}

TEST_F(PortPool_test, GetServiceRegistryChangeCounterReturnsZeroAsInitialValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "5c028500-6589-4947-9e9c-68b691028244");
    auto serviceCounter = sut.serviceRegistryChangeCounter();

    EXPECT_EQ(serviceCounter->load(), 0U);
}

TEST_F(PortPool_test, GetServiceRegistryChangeCounterIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "12c20e8b-f819-435c-b9c0-ca0acabacb8d");
    sut.serviceRegistryChangeCounter()->fetch_add(1, std::memory_order_relaxed);
    auto serviceCounter = sut.serviceRegistryChangeCounter();

    ASSERT_EQ(serviceCounter->load(), 1U);
}

} // namespace
