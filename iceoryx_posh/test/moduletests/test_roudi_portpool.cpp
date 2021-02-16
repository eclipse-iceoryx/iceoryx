// Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/roudi/port_pool_data.hpp"
#include "iceoryx_posh/internal/runtime/node_data.hpp"
#include "iceoryx_posh/popo/typed_subscriber.hpp"
#include "iceoryx_posh/roudi/port_pool.hpp"
#include "test.hpp"

using namespace ::testing;
using ::testing::Return;
using namespace iox::capro;

namespace iox
{
namespace test
{
static constexpr uint32_t DEFAULT_DEVICE_ID{0u};
static constexpr uint32_t DEFAULT_MEMORY_TYPE{0u};
class PortPool_test : public Test
{
  public:
    roudi::PortPoolData m_portPoolData;
    roudi::PortPool sut{m_portPoolData};

    ServiceDescription m_serviceDescription{"service1", "instance1"};
    ProcessName_t m_applicationName{"AppName"};
    ProcessName_t m_processName{"processName"};
    ProcessName_t m_nodeName{"nodeName"};
    const uint64_t m_nodeDeviceId = 999U;
    mepoo::MemoryManager m_memoryManager;
    popo::PublisherOptions m_publisherOptions;
    popo::SubscriberOptions m_subscriberOptions;
};

TEST_F(PortPool_test, AddNodeDataIsSuccessful)
{
    auto nodeData = sut.addNodeData(m_processName, m_nodeName, m_nodeDeviceId);

    ASSERT_THAT(nodeData.value()->m_process, Eq(m_processName));
    ASSERT_THAT(nodeData.value()->m_node, Eq(m_nodeName));
    ASSERT_THAT(nodeData.value()->m_nodeDeviceIdentifier, Eq(m_nodeDeviceId));
}

TEST_F(PortPool_test, AddNodeDataWithMaxCapacityIsSuccessful)
{
    cxx::vector<runtime::NodeData*, MAX_NODE_NUMBER> nodeContainer;

    for (uint32_t i = 1U; i <= MAX_NODE_NUMBER; ++i)
    {
        auto nodeData = sut.addNodeData(m_processName, m_nodeName, i);
        if (!nodeData.has_error())
        {
            nodeContainer.push_back(nodeData.value());
        }
    }

    ASSERT_THAT(nodeContainer.size(), Eq(MAX_NODE_NUMBER));
}


TEST_F(PortPool_test, AddNodeDataWhenNodeListIsFullReturnsError)
{
    auto errorHandlerCalled{false};
    Error errorHandlerType;
    auto errorHandlerGuard =
        ErrorHandler::SetTemporaryErrorHandler([&](const Error error, const std::function<void()>, const ErrorLevel) {
            errorHandlerType = error;
            errorHandlerCalled = true;
        });

    for (uint32_t i = 0U; i <= MAX_NODE_NUMBER; ++i)
    {
        sut.addNodeData(m_processName, m_nodeName, i);
    }

    ASSERT_EQ(errorHandlerCalled, true);
    EXPECT_EQ(errorHandlerType, Error::kPORT_POOL__NODELIST_OVERFLOW);
}

TEST_F(PortPool_test, GetNodeDataListWhenEmptyIsSuccessful)
{
    auto nodeDataList = sut.getNodeDataList();

    ASSERT_THAT(nodeDataList.size(), Eq(0U));
}

TEST_F(PortPool_test, GetNodeDataListIsSuccessful)
{
    sut.addNodeData(m_processName, m_nodeName, m_nodeDeviceId);
    auto nodeDataList = sut.getNodeDataList();

    ASSERT_THAT(nodeDataList.size(), Eq(1U));
}

TEST_F(PortPool_test, GetNodeDataListWithMaxCapacityIsSuccessful)
{
    cxx::vector<runtime::NodeData*, MAX_NODE_NUMBER> nodeContainer;

    for (uint32_t i = 1U; i <= MAX_NODE_NUMBER; ++i)
    {
        auto nodeData = sut.addNodeData(m_processName, m_nodeName, i);
        if (!nodeData.has_error())
        {
            nodeContainer.push_back(nodeData.value());
        }
    }

    auto nodeDataList = sut.getNodeDataList();
    ASSERT_THAT(nodeDataList.size(), Eq(MAX_NODE_NUMBER));
}

TEST_F(PortPool_test, RemoveNodeDataIsSuccessful)
{
    auto nodeData = sut.addNodeData(m_processName, m_nodeName, m_nodeDeviceId);
    sut.removeNodeData(nodeData.value());
    auto nodeDataList = sut.getNodeDataList();

    ASSERT_THAT(nodeDataList.size(), Eq(0U));
}

TEST_F(PortPool_test, AddPublisherPortIsSuccessful)
{
    auto publisherPort =
        sut.addPublisherPort(m_serviceDescription, &m_memoryManager, m_applicationName, m_publisherOptions);

    EXPECT_THAT(publisherPort.value()->m_serviceDescription, Eq(ServiceDescription{"service1", "instance1"}));
    EXPECT_EQ(publisherPort.value()->m_processName, m_applicationName);
    EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_historyCapacity, 0UL);
    EXPECT_EQ(publisherPort.value()->m_nodeName, NodeName_t{""});
    EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
    EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
}

TEST_F(PortPool_test, AddMaxPublisherPortWithMaxCapacityIsSuccessful)
{
    for (uint32_t i = 0; i < MAX_PUBLISHERS; ++i)
    {
        std::string service = "service" + std::to_string(i);
        std::string instance = "instance" + std::to_string(i);
        ProcessName_t applicationName = {cxx::TruncateToCapacity, "AppName" + std::to_string(i)};


        auto publisherPort = sut.addPublisherPort(
            {IdString_t(cxx::TruncateToCapacity, service), IdString_t(cxx::TruncateToCapacity, instance)},
            &m_memoryManager,
            applicationName,
            m_publisherOptions);

        EXPECT_THAT(publisherPort.value()->m_serviceDescription,
                    Eq(ServiceDescription{IdString_t(cxx::TruncateToCapacity, service),
                                          IdString_t(cxx::TruncateToCapacity, instance)}));
        EXPECT_EQ(publisherPort.value()->m_processName, applicationName);
        EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_historyCapacity, 0UL);
        EXPECT_EQ(publisherPort.value()->m_nodeName, NodeName_t{""});
        EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
        EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
    }
}

TEST_F(PortPool_test, AddPublisherPortWhenOverflowsReurnsError)
{
    auto errorHandlerCalled{false};
    Error errorHandlerType;
    auto errorHandlerGuard =
        ErrorHandler::SetTemporaryErrorHandler([&](const Error error, const std::function<void()>, const ErrorLevel) {
            errorHandlerType = error;
            errorHandlerCalled = true;
        });

    for (uint32_t i = 0; i <= MAX_PUBLISHERS; ++i)
    {
        std::string service = "service" + std::to_string(i);
        std::string instance = "instance" + std::to_string(i);
        ProcessName_t applicationName = {cxx::TruncateToCapacity, "AppName" + std::to_string(i)};


        auto publisherPort = sut.addPublisherPort(
            {IdString_t(cxx::TruncateToCapacity, service), IdString_t(cxx::TruncateToCapacity, instance)},
            &m_memoryManager,
            applicationName,
            m_publisherOptions);
    }

    ASSERT_THAT(errorHandlerCalled, Eq(true));
    EXPECT_EQ(errorHandlerType, Error::kPORT_POOL__PUBLISHERLIST_OVERFLOW);
}

TEST_F(PortPool_test, GetPublisherPortDataListIsSuccessful)
{
    auto publisherPortDataList = sut.getPublisherPortDataList();

    EXPECT_THAT(publisherPortDataList.size(), Eq(0U));

    sut.addPublisherPort(m_serviceDescription, &m_memoryManager, m_applicationName, m_publisherOptions);
    publisherPortDataList = sut.getPublisherPortDataList();

    ASSERT_THAT(publisherPortDataList.size(), Eq(1U));
}

TEST_F(PortPool_test, GetPublisherPortDataListWhenEmptyIsSuccessful)
{
    auto nodeDataList = sut.getPublisherPortDataList();

    ASSERT_THAT(nodeDataList.size(), Eq(0U));
}

TEST_F(PortPool_test, GetPublisherPortDataListCompletelyFilledSuccessfully)
{
    for (uint32_t i = 0; i < MAX_PUBLISHERS; ++i)
    {
        std::string service = "service" + std::to_string(i);
        std::string instance = "instance" + std::to_string(i);
        ProcessName_t applicationName = {cxx::TruncateToCapacity, "AppName" + std::to_string(i)};


        auto publisherPort = sut.addPublisherPort(
            {IdString_t(cxx::TruncateToCapacity, service), IdString_t(cxx::TruncateToCapacity, instance)},
            &m_memoryManager,
            applicationName,
            m_publisherOptions);
    }

    auto publisherPortDataList = sut.getPublisherPortDataList();

    ASSERT_THAT(publisherPortDataList.size(), Eq(MAX_PUBLISHERS));
}

TEST_F(PortPool_test, RemovePublisherPortIsSuccessful)
{
    auto publisherPort =
        sut.addPublisherPort(m_serviceDescription, &m_memoryManager, m_applicationName, m_publisherOptions);
    sut.removePublisherPort(publisherPort.value());
    auto publisherPortDataList = sut.getPublisherPortDataList();

    ASSERT_THAT(publisherPortDataList.size(), Eq(0U));
}

TEST_F(PortPool_test, AddSubscriberPortIsSuccessful)
{
    auto subscriberPort = sut.addSubscriberPort(m_serviceDescription, m_applicationName, m_subscriberOptions);

    EXPECT_THAT(subscriberPort.value()->m_serviceDescription, Eq(m_serviceDescription));
    EXPECT_EQ(subscriberPort.value()->m_processName, m_applicationName);
    EXPECT_EQ(subscriberPort.value()->m_nodeName, NodeName_t{""});
    EXPECT_EQ(subscriberPort.value()->m_historyRequest, 0U);
    EXPECT_EQ(subscriberPort.value()->m_chunkReceiverData.m_queue.capacity(), 256U);
    EXPECT_EQ(subscriberPort.value()->m_chunkReceiverData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
    EXPECT_EQ(subscriberPort.value()->m_chunkReceiverData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
}

TEST_F(PortPool_test, AddSubscriberPortToMaxCapacityIsSuccessful)
{
    for (uint32_t i = 0; i < MAX_SUBSCRIBERS; ++i)
    {
        std::string service = "service" + std::to_string(i);
        std::string instance = "instance" + std::to_string(i);
        ProcessName_t applicationName = {cxx::TruncateToCapacity, "AppName" + std::to_string(i)};


        auto subscriberPort = sut.addSubscriberPort(
            {IdString_t(cxx::TruncateToCapacity, service), IdString_t(cxx::TruncateToCapacity, instance)},
            applicationName,
            m_subscriberOptions);

        EXPECT_THAT(subscriberPort.value()->m_serviceDescription,
                    Eq(ServiceDescription{IdString_t(cxx::TruncateToCapacity, service),
                                          IdString_t(cxx::TruncateToCapacity, instance)}));
        EXPECT_EQ(subscriberPort.value()->m_processName, applicationName);
        EXPECT_EQ(subscriberPort.value()->m_nodeName, NodeName_t{""});
        EXPECT_EQ(subscriberPort.value()->m_chunkReceiverData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
        EXPECT_EQ(subscriberPort.value()->m_chunkReceiverData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
    }
}


TEST_F(PortPool_test, AddSubscriberPortWhenOverflowsReurnsError)
{
    auto errorHandlerCalled{false};
    Error errorHandlerType;
    auto errorHandlerGuard =
        ErrorHandler::SetTemporaryErrorHandler([&](const Error error, const std::function<void()>, const ErrorLevel) {
            errorHandlerType = error;
            errorHandlerCalled = true;
        });

    for (uint32_t i = 0; i <= MAX_SUBSCRIBERS; ++i)
    {
        std::string service = "service" + std::to_string(i);
        std::string instance = "instance" + std::to_string(i);
        ProcessName_t applicationName = {cxx::TruncateToCapacity, "AppName" + std::to_string(i)};


        auto publisherPort = sut.addSubscriberPort(
            {IdString_t(cxx::TruncateToCapacity, service), IdString_t(cxx::TruncateToCapacity, instance)},
            applicationName,
            m_subscriberOptions);
    }

    ASSERT_THAT(errorHandlerCalled, Eq(true));
    EXPECT_EQ(errorHandlerType, Error::kPORT_POOL__SUBSCRIBERLIST_OVERFLOW);
}

TEST_F(PortPool_test, GetSubscriberPortDataListWhenEmptyIsSuccessful)
{
    auto nodeDataList = sut.getSubscriberPortDataList();

    ASSERT_THAT(nodeDataList.size(), Eq(0U));
}


TEST_F(PortPool_test, GetSubscriberPortDataListIsSuccessful)
{
    auto subscriberPort = sut.addSubscriberPort(m_serviceDescription, m_applicationName, m_subscriberOptions);
    auto subscriberPortDataList = sut.getSubscriberPortDataList();

    ASSERT_THAT(subscriberPortDataList.size(), Eq(1U));
}

TEST_F(PortPool_test, GetSubscriberPortDataListCompletelyFilledIsSuccessful)
{
    for (uint32_t i = 0; i < MAX_SUBSCRIBERS; ++i)
    {
        std::string service = "service" + std::to_string(i);
        std::string instance = "instance" + std::to_string(i);
        ProcessName_t applicationName = {cxx::TruncateToCapacity, "AppName" + std::to_string(i)};


        auto publisherPort = sut.addSubscriberPort(
            {IdString_t(cxx::TruncateToCapacity, service), IdString_t(cxx::TruncateToCapacity, instance)},
            applicationName,
            m_subscriberOptions);
    }
    auto subscriberPortDataList = sut.getSubscriberPortDataList();

    ASSERT_THAT(subscriberPortDataList.size(), Eq(MAX_SUBSCRIBERS));
}

TEST_F(PortPool_test, RemoveSubscriberPortIsSuccessful)
{
    auto subscriberPort = sut.addSubscriberPort(m_serviceDescription, m_applicationName, m_subscriberOptions);

    sut.removeSubscriberPort(subscriberPort.value());
    auto subscriberPortDataList = sut.getSubscriberPortDataList();

    ASSERT_THAT(subscriberPortDataList.size(), Eq(0U));
}

TEST_F(PortPool_test, AddInterfacePortIsSuccessful)
{
    auto interfacePortData = sut.addInterfacePort(m_applicationName, Interfaces::INTERNAL);
    EXPECT_THAT(interfacePortData.value()->m_processName, Eq(m_applicationName));
    EXPECT_THAT(interfacePortData.value()->m_serviceDescription.getSourceInterface(), Eq(Interfaces::INTERNAL));
}

TEST_F(PortPool_test, AddInterfacePortWithMaxCapacityIsSuccessful)
{
    cxx::vector<popo::InterfacePortData*, MAX_INTERFACE_NUMBER> interfacePortContainer;

    for (uint32_t i = 1U; i <= MAX_INTERFACE_NUMBER; ++i)
    {
        auto interfacePortData = sut.addInterfacePort(m_applicationName, Interfaces::INTERNAL);
        if (!interfacePortData.has_error())
        {
            interfacePortContainer.push_back(interfacePortData.value());
        }
    }

    ASSERT_THAT(interfacePortContainer.size(), Eq(MAX_INTERFACE_NUMBER));
}

TEST_F(PortPool_test, AddInterfacePortWhenContainerIsFullReturnsError)
{
    auto errorHandlerCalled{false};
    Error errorHandlerType;
    auto errorHandlerGuard =
        ErrorHandler::SetTemporaryErrorHandler([&](const Error error, const std::function<void()>, const ErrorLevel) {
            errorHandlerType = error;
            errorHandlerCalled = true;
        });

    for (uint32_t i = 0U; i <= MAX_INTERFACE_NUMBER; ++i)
    {
        auto interfacePortData = sut.addInterfacePort(m_applicationName, Interfaces::INTERNAL);
    }

    ASSERT_EQ(errorHandlerCalled, true);
    EXPECT_EQ(errorHandlerType, Error::kPORT_POOL__INTERFACELIST_OVERFLOW);
}

TEST_F(PortPool_test, GetInterfacePortDataListWhenEmptyIsSuccessful)
{
    auto interfacePortDataList = sut.getInterfacePortDataList();

    EXPECT_THAT(interfacePortDataList.size(), Eq(0U));
}

TEST_F(PortPool_test, GetInterfacePortDataListIsSuccessful)
{
    auto interfacePort = sut.addInterfacePort(m_applicationName, Interfaces::INTERNAL);
    auto interfacePortDataList = sut.getInterfacePortDataList();

    ASSERT_THAT(interfacePortDataList.size(), Eq(1U));
}

TEST_F(PortPool_test, GetInterfacePortDataListCompletelyFilledIsSuccessful)
{
    for (uint32_t i = 0; i < MAX_INTERFACE_NUMBER; ++i)
    {
        ProcessName_t applicationName = {cxx::TruncateToCapacity, "AppName" + std::to_string(i)};

        auto interfacePort = sut.addInterfacePort(applicationName, Interfaces::INTERNAL);
    }
    auto interfacePortDataList = sut.getInterfacePortDataList();

    ASSERT_THAT(interfacePortDataList.size(), Eq(MAX_INTERFACE_NUMBER));
}

TEST_F(PortPool_test, RemoveInterfacePortIsSuccessful)
{
    auto interfacePort = sut.addInterfacePort(m_applicationName, Interfaces::INTERNAL);

    sut.removeInterfacePort(interfacePort.value());
    auto interfacePortDataList = sut.getInterfacePortDataList();

    ASSERT_THAT(interfacePortDataList.size(), Eq(0U));
}

} // namespace test
} // namespace iox