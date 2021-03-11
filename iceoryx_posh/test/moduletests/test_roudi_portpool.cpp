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
#include "iceoryx_posh/popo/subscriber_options.hpp"
#include "iceoryx_posh/roudi/port_pool.hpp"
#include "test.hpp"

using namespace ::testing;
using ::testing::Return;
using namespace iox::capro;

namespace iox
{
namespace test
{
static constexpr uint32_t DEFAULT_DEVICE_ID{20u};
static constexpr uint32_t DEFAULT_MEMORY_TYPE{100u};
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
    popo::PublisherOptions m_publisherOptions{10, "nodeName"};
    popo::SubscriberOptions m_subscriberOptions{
        iox::popo::SubscriberPortData::ChunkQueueData_t::MAX_CAPACITY, 10, "nodeName"};
    iox::mepoo::MemoryInfo m_memoryInfo{DEFAULT_DEVICE_ID, DEFAULT_MEMORY_TYPE};
};

TEST_F(PortPool_test, AddNodeDataIsSuccessful)
{
    auto nodeData = sut.addNodeData(m_processName, m_nodeName, m_nodeDeviceId);

    ASSERT_EQ(nodeData.value()->m_process, m_processName);
    ASSERT_EQ(nodeData.value()->m_node, m_nodeName);
    ASSERT_EQ(nodeData.value()->m_nodeDeviceIdentifier, m_nodeDeviceId);
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

    ASSERT_EQ(nodeContainer.size(), MAX_NODE_NUMBER);
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

    ASSERT_TRUE(errorHandlerCalled);
    ASSERT_EQ(errorHandlerType, Error::kPORT_POOL__NODELIST_OVERFLOW);
}

TEST_F(PortPool_test, GetNodeDataListIsSuccessful)
{
    sut.addNodeData(m_processName, m_nodeName, m_nodeDeviceId);
    auto nodeDataList = sut.getNodeDataList();

    ASSERT_EQ(nodeDataList.size(), 1U);

    for (const auto& nodeData : nodeDataList)
    {
        ASSERT_EQ(nodeData->m_process, m_processName);
        ASSERT_EQ(nodeData->m_node, m_nodeName);
        ASSERT_EQ(nodeData->m_nodeDeviceIdentifier, m_nodeDeviceId);
    }
}

TEST_F(PortPool_test, GetNodeDataListWhenEmptyIsSuccessful)
{
    auto nodeDataList = sut.getNodeDataList();

    ASSERT_EQ(nodeDataList.size(), 0U);
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

    ASSERT_EQ(nodeDataList.size(), MAX_NODE_NUMBER);
}

TEST_F(PortPool_test, RemoveNodeDataIsSuccessful)
{
    auto nodeData = sut.addNodeData(m_processName, m_nodeName, m_nodeDeviceId);

    sut.removeNodeData(nodeData.value());
    auto nodeDataList = sut.getNodeDataList();

    ASSERT_EQ(nodeDataList.size(), 0U);
}

TEST_F(PortPool_test, AddPublisherPortIsSuccessful)
{
    auto publisherPort = sut.addPublisherPort(
        m_serviceDescription, &m_memoryManager, m_applicationName, m_publisherOptions, m_memoryInfo);

    ASSERT_THAT(publisherPort.value()->m_serviceDescription, Eq(ServiceDescription{"service1", "instance1"}));
    ASSERT_EQ(publisherPort.value()->m_processName, m_applicationName);
    ASSERT_EQ(publisherPort.value()->m_chunkSenderData.m_historyCapacity, m_publisherOptions.historyCapacity);
    ASSERT_EQ(publisherPort.value()->m_nodeName, m_publisherOptions.nodeName);
    ASSERT_EQ(publisherPort.value()->m_chunkSenderData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
    ASSERT_EQ(publisherPort.value()->m_chunkSenderData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
}

TEST_F(PortPool_test, AddPublisherPortWithMaxCapacityIsSuccessful)
{
    for (uint32_t i = 0U; i < MAX_PUBLISHERS; ++i)
    {
        std::string service = "service" + std::to_string(i);
        std::string instance = "instance" + std::to_string(i);
        ProcessName_t applicationName = {cxx::TruncateToCapacity, "AppName" + std::to_string(i)};

        auto publisherPort = sut.addPublisherPort(
            {IdString_t(cxx::TruncateToCapacity, service), IdString_t(cxx::TruncateToCapacity, instance)},
            &m_memoryManager,
            applicationName,
            m_publisherOptions,
            m_memoryInfo);

        ASSERT_THAT(publisherPort.value()->m_serviceDescription,
                    Eq(ServiceDescription{IdString_t(cxx::TruncateToCapacity, service),
                                          IdString_t(cxx::TruncateToCapacity, instance)}));
        ASSERT_EQ(publisherPort.value()->m_processName, applicationName);
        ASSERT_EQ(publisherPort.value()->m_chunkSenderData.m_historyCapacity, m_publisherOptions.historyCapacity);
        ASSERT_EQ(publisherPort.value()->m_nodeName, m_publisherOptions.nodeName);
        ASSERT_EQ(publisherPort.value()->m_chunkSenderData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
        ASSERT_EQ(publisherPort.value()->m_chunkSenderData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
    }
}

TEST_F(PortPool_test, AddPublisherPortWhenPublisherListOverflowsReurnsError)
{
    auto errorHandlerCalled{false};
    Error errorHandlerType;
    auto errorHandlerGuard =
        ErrorHandler::SetTemporaryErrorHandler([&](const Error error, const std::function<void()>, const ErrorLevel) {
            errorHandlerType = error;
            errorHandlerCalled = true;
        });

    for (uint32_t i = 0U; i <= MAX_PUBLISHERS; ++i)
    {
        std::string service = "service" + std::to_string(i);
        std::string instance = "instance" + std::to_string(i);
        ProcessName_t applicationName = {cxx::TruncateToCapacity, "AppName" + std::to_string(i)};

        sut.addPublisherPort(
            {IdString_t(cxx::TruncateToCapacity, service), IdString_t(cxx::TruncateToCapacity, instance)},
            &m_memoryManager,
            applicationName,
            m_publisherOptions);
    }

    ASSERT_TRUE(errorHandlerCalled);
    ASSERT_EQ(errorHandlerType, Error::kPORT_POOL__PUBLISHERLIST_OVERFLOW);
}

TEST_F(PortPool_test, GetPublisherPortDataListIsSuccessful)
{
    auto publisherPortDataList = sut.getPublisherPortDataList();

    ASSERT_EQ(publisherPortDataList.size(), 0U);

    sut.addPublisherPort(m_serviceDescription, &m_memoryManager, m_applicationName, m_publisherOptions);
    publisherPortDataList = sut.getPublisherPortDataList();

    ASSERT_EQ(publisherPortDataList.size(), 1U);
}

TEST_F(PortPool_test, GetPublisherPortDataListWhenEmptyIsSuccessful)
{
    auto nodeDataList = sut.getPublisherPortDataList();

    ASSERT_EQ(nodeDataList.size(), 0U);
}

TEST_F(PortPool_test, GetPublisherPortDataListCompletelyFilledSuccessfully)
{
    for (uint32_t i = 0U; i < MAX_PUBLISHERS; ++i)
    {
        std::string service = "service" + std::to_string(i);
        std::string instance = "instance" + std::to_string(i);
        ProcessName_t applicationName = {cxx::TruncateToCapacity, "AppName" + std::to_string(i)};

        sut.addPublisherPort(
            {IdString_t(cxx::TruncateToCapacity, service), IdString_t(cxx::TruncateToCapacity, instance)},
            &m_memoryManager,
            applicationName,
            m_publisherOptions);
    }

    auto publisherPortDataList = sut.getPublisherPortDataList();

    ASSERT_EQ(publisherPortDataList.size(), MAX_PUBLISHERS);
}

TEST_F(PortPool_test, RemovePublisherPortIsSuccessful)
{
    auto publisherPort =
        sut.addPublisherPort(m_serviceDescription, &m_memoryManager, m_applicationName, m_publisherOptions);
    sut.removePublisherPort(publisherPort.value());
    auto publisherPortDataList = sut.getPublisherPortDataList();

    ASSERT_EQ(publisherPortDataList.size(), 0U);
}

TEST_F(PortPool_test, AddSubscriberPortIsSuccessful)
{
    auto subscriberPort =
        sut.addSubscriberPort(m_serviceDescription, m_applicationName, m_subscriberOptions, m_memoryInfo);

    ASSERT_EQ(subscriberPort.value()->m_serviceDescription, m_serviceDescription);
    ASSERT_EQ(subscriberPort.value()->m_processName, m_applicationName);
    ASSERT_EQ(subscriberPort.value()->m_nodeName, m_subscriberOptions.nodeName);
    ASSERT_EQ(subscriberPort.value()->m_historyRequest, m_subscriberOptions.historyRequest);
    ASSERT_EQ(subscriberPort.value()->m_chunkReceiverData.m_queue.capacity(), 256U);
    ASSERT_EQ(subscriberPort.value()->m_chunkReceiverData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
    ASSERT_EQ(subscriberPort.value()->m_chunkReceiverData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
}

TEST_F(PortPool_test, AddSubscriberPortToMaxCapacityIsSuccessful)
{
    for (uint32_t i = 0U; i < MAX_SUBSCRIBERS; ++i)
    {
        std::string service = "service" + std::to_string(i);
        std::string instance = "instance" + std::to_string(i);
        ProcessName_t applicationName = {cxx::TruncateToCapacity, "AppName" + std::to_string(i)};


        auto subscriberPort = sut.addSubscriberPort(
            {IdString_t(cxx::TruncateToCapacity, service), IdString_t(cxx::TruncateToCapacity, instance)},
            applicationName,
            m_subscriberOptions,
            m_memoryInfo);

        ASSERT_THAT(subscriberPort.value()->m_serviceDescription,
                    Eq(ServiceDescription{IdString_t(cxx::TruncateToCapacity, service),
                                          IdString_t(cxx::TruncateToCapacity, instance)}));
        ASSERT_EQ(subscriberPort.value()->m_processName, applicationName);
        ASSERT_EQ(subscriberPort.value()->m_nodeName, m_subscriberOptions.nodeName);
        ASSERT_EQ(subscriberPort.value()->m_chunkReceiverData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
        ASSERT_EQ(subscriberPort.value()->m_chunkReceiverData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
    }
}


TEST_F(PortPool_test, AddSubscriberPortWhenSubscriberListOverflowsReurnsError)
{
    auto errorHandlerCalled{false};
    Error errorHandlerType;
    auto errorHandlerGuard =
        ErrorHandler::SetTemporaryErrorHandler([&](const Error error, const std::function<void()>, const ErrorLevel) {
            errorHandlerType = error;
            errorHandlerCalled = true;
        });

    for (uint32_t i = 0U; i <= MAX_SUBSCRIBERS; ++i)
    {
        std::string service = "service" + std::to_string(i);
        std::string instance = "instance" + std::to_string(i);
        ProcessName_t applicationName = {cxx::TruncateToCapacity, "AppName" + std::to_string(i)};


        auto publisherPort = sut.addSubscriberPort(
            {IdString_t(cxx::TruncateToCapacity, service), IdString_t(cxx::TruncateToCapacity, instance)},
            applicationName,
            m_subscriberOptions);
    }

    ASSERT_TRUE(errorHandlerCalled);
    ASSERT_EQ(errorHandlerType, Error::kPORT_POOL__SUBSCRIBERLIST_OVERFLOW);
}

TEST_F(PortPool_test, GetSubscriberPortDataListIsSuccessful)
{
    auto subscriberPort = sut.addSubscriberPort(m_serviceDescription, m_applicationName, m_subscriberOptions);
    auto subscriberPortDataList = sut.getSubscriberPortDataList();

    ASSERT_EQ(subscriberPortDataList.size(), 1U);
}

TEST_F(PortPool_test, GetSubscriberPortDataListWhenEmptyIsSuccessful)
{
    auto nodeDataList = sut.getSubscriberPortDataList();

    ASSERT_EQ(nodeDataList.size(), 0U);
}

TEST_F(PortPool_test, GetSubscriberPortDataListCompletelyFilledIsSuccessful)
{
    for (uint32_t i = 0U; i < MAX_SUBSCRIBERS; ++i)
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

    ASSERT_EQ(subscriberPortDataList.size(), MAX_SUBSCRIBERS);
}

TEST_F(PortPool_test, RemoveSubscriberPortIsSuccessful)
{
    auto subscriberPort = sut.addSubscriberPort(m_serviceDescription, m_applicationName, m_subscriberOptions);

    sut.removeSubscriberPort(subscriberPort.value());
    auto subscriberPortDataList = sut.getSubscriberPortDataList();

    ASSERT_EQ(subscriberPortDataList.size(), 0U);
}

TEST_F(PortPool_test, AddInterfacePortIsSuccessful)
{
    auto interfacePortData = sut.addInterfacePort(m_applicationName, Interfaces::INTERNAL);

    ASSERT_EQ(interfacePortData.value()->m_processName, m_applicationName);
    ASSERT_EQ(interfacePortData.value()->m_serviceDescription.getSourceInterface(), Interfaces::INTERNAL);
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

    ASSERT_EQ(interfacePortContainer.size(), MAX_INTERFACE_NUMBER);
}

TEST_F(PortPool_test, AddInterfacePortWhenInterfaceListOverflowsReturnsError)
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

    ASSERT_TRUE(errorHandlerCalled);
    ASSERT_EQ(errorHandlerType, Error::kPORT_POOL__INTERFACELIST_OVERFLOW);
}

TEST_F(PortPool_test, GetInterfacePortDataListIsSuccessful)
{
    auto interfacePort = sut.addInterfacePort(m_applicationName, Interfaces::INTERNAL);
    auto interfacePortDataList = sut.getInterfacePortDataList();

    ASSERT_EQ(interfacePortDataList.size(), 1U);
}

TEST_F(PortPool_test, GetInterfacePortDataListWhenEmptyIsSuccessful)
{
    auto interfacePortDataList = sut.getInterfacePortDataList();

    ASSERT_EQ(interfacePortDataList.size(), 0U);
}

TEST_F(PortPool_test, GetInterfacePortDataListCompletelyFilledIsSuccessful)
{
    for (uint32_t i = 0U; i < MAX_INTERFACE_NUMBER; ++i)
    {
        ProcessName_t applicationName = {cxx::TruncateToCapacity, "AppName" + std::to_string(i)};
        sut.addInterfacePort(applicationName, Interfaces::INTERNAL);
    }
    auto interfacePortDataList = sut.getInterfacePortDataList();

    ASSERT_EQ(interfacePortDataList.size(), MAX_INTERFACE_NUMBER);
}

TEST_F(PortPool_test, RemoveInterfacePortIsSuccessful)
{
    auto interfacePort = sut.addInterfacePort(m_applicationName, Interfaces::INTERNAL);

    sut.removeInterfacePort(interfacePort.value());
    auto interfacePortDataList = sut.getInterfacePortDataList();

    ASSERT_EQ(interfacePortDataList.size(), 0U);
}

TEST_F(PortPool_test, AddApplicationPortIsSuccessful)
{
    auto applicationPortData = sut.addApplicationPort(m_applicationName);

    ASSERT_EQ(applicationPortData.value()->m_processName, m_applicationName);
}

TEST_F(PortPool_test, AddApplicationPortWithMaxCapacityIsSuccessful)
{
    cxx::vector<popo::ApplicationPortData*, MAX_PROCESS_NUMBER> applicationContainer;

    for (uint32_t i = 1U; i <= MAX_PROCESS_NUMBER; ++i)
    {
        auto applicationPortData = sut.addApplicationPort(m_applicationName);
        if (!applicationPortData.has_error())
        {
            applicationContainer.push_back(applicationPortData.value());
        }
    }

    ASSERT_EQ(applicationContainer.size(), MAX_PROCESS_NUMBER);
}

TEST_F(PortPool_test, AddApplicationPortWhenApplicationListOverflowsReturnsError)
{
    auto errorHandlerCalled{false};
    Error errorHandlerType;
    auto errorHandlerGuard =
        ErrorHandler::SetTemporaryErrorHandler([&](const Error error, const std::function<void()>, const ErrorLevel) {
            errorHandlerType = error;
            errorHandlerCalled = true;
        });

    for (uint32_t i = 0U; i <= MAX_PROCESS_NUMBER; ++i)
    {
        auto applicationData = sut.addApplicationPort(m_applicationName);
    }

    ASSERT_TRUE(errorHandlerCalled);
    ASSERT_EQ(errorHandlerType, Error::kPORT_POOL__APPLICATIONLIST_OVERFLOW);
}

TEST_F(PortPool_test, GetApplicationPortDataListWhenEmptyIsSuccessful)
{
    auto applicationPortDataList = sut.getApplicationPortDataList();

    ASSERT_EQ(applicationPortDataList.size(), 0U);
}

TEST_F(PortPool_test, GetApplicationPortDataListIsSuccessful)
{
    sut.addApplicationPort(m_applicationName);
    auto applicationPortDataList = sut.getApplicationPortDataList();

    ASSERT_EQ(applicationPortDataList.size(), 1U);
}

TEST_F(PortPool_test, GetApplicationPortDataListCompletelyFilledIsSuccessful)
{
    for (uint32_t i = 0U; i < MAX_PROCESS_NUMBER; ++i)
    {
        ProcessName_t applicationName = {cxx::TruncateToCapacity, "AppName" + std::to_string(i)};
        sut.addApplicationPort(applicationName);
    }
    auto applicationPortDataList = sut.getApplicationPortDataList();

    ASSERT_EQ(applicationPortDataList.size(), MAX_PROCESS_NUMBER);
}

TEST_F(PortPool_test, RemoveApplicationPortIsSuccessful)
{
    auto applicationPortData = sut.addApplicationPort(m_applicationName);

    sut.removeApplicationPort(applicationPortData.value());
    auto applicationPortDataList = sut.getApplicationPortDataList();

    ASSERT_EQ(applicationPortDataList.size(), 0U);
}

TEST_F(PortPool_test, AddConditionVariableDataIsSuccessful)
{
    auto conditionVariableData = sut.addConditionVariableData(m_applicationName);

    ASSERT_EQ(conditionVariableData.value()->m_process, m_applicationName);
}

TEST_F(PortPool_test, AddConditionVariableDataWithMaxCapacityIsSuccessful)
{
    cxx::vector<popo::ConditionVariableData*, MAX_NUMBER_OF_CONDITION_VARIABLES> conditionVariableContainer;

    for (uint32_t i = 1U; i <= MAX_NUMBER_OF_CONDITION_VARIABLES; ++i)
    {
        auto conditionVariableData = sut.addConditionVariableData(m_applicationName);
        if (!conditionVariableData.has_error())
        {
            conditionVariableContainer.push_back(conditionVariableData.value());
        }
    }

    ASSERT_EQ(conditionVariableContainer.size(), MAX_NUMBER_OF_CONDITION_VARIABLES);
}

TEST_F(PortPool_test, AddConditionVariableDataWhenContainerIsFullReturnsError)
{
    auto errorHandlerCalled{false};
    Error errorHandlerType;
    auto errorHandlerGuard =
        ErrorHandler::SetTemporaryErrorHandler([&](const Error error, const std::function<void()>, const ErrorLevel) {
            errorHandlerType = error;
            errorHandlerCalled = true;
        });

    for (uint32_t i = 0U; i <= MAX_NUMBER_OF_CONDITION_VARIABLES; ++i)
    {
        auto conditionVariableData = sut.addConditionVariableData(m_applicationName);
    }

    ASSERT_TRUE(errorHandlerCalled);
    ASSERT_EQ(errorHandlerType, Error::kPORT_POOL__CONDITION_VARIABLE_LIST_OVERFLOW);
}

TEST_F(PortPool_test, GetConditionVariableDataListIsSuccessful)
{
    sut.addConditionVariableData(m_applicationName);
    auto condtionalVariableData = sut.getConditionVariableDataList();

    ASSERT_EQ(condtionalVariableData.size(), 1U);
}

TEST_F(PortPool_test, GetConditionVariableDataListWhenEmptyIsSuccessful)
{
    auto condtionalVariableData = sut.getConditionVariableDataList();

    ASSERT_EQ(condtionalVariableData.size(), 0U);
}

TEST_F(PortPool_test, GetConditionVariableDataListCompletelyFilledIsSuccessful)
{
    for (uint32_t i = 0U; i < MAX_NUMBER_OF_CONDITION_VARIABLES; ++i)
    {
        ProcessName_t applicationName = {cxx::TruncateToCapacity, "AppName" + std::to_string(i)};
        sut.addConditionVariableData(applicationName);
    }
    auto condtionalVariableData = sut.getConditionVariableDataList();

    ASSERT_EQ(condtionalVariableData.size(), MAX_NUMBER_OF_CONDITION_VARIABLES);
}

TEST_F(PortPool_test, RemoveConditionVariableDataIsSuccessful)
{
    auto conditionVariableData = sut.addConditionVariableData(m_applicationName);

    sut.removeConditionVariableData(conditionVariableData.value());
    auto condtionalVariableData = sut.getConditionVariableDataList();

    ASSERT_EQ(condtionalVariableData.size(), 0U);
}

TEST_F(PortPool_test, GetServiceRegistryChangeCounterReturnsZeroAsInitialValue)
{
    auto serviceCounter = sut.serviceRegistryChangeCounter();

    ASSERT_EQ(serviceCounter->load(), 0U);
}


TEST_F(PortPool_test, GetServiceRegistryChangeCounterIsSuccessfull)
{
    sut.serviceRegistryChangeCounter()->fetch_add(1, std::memory_order_relaxed);
    auto serviceCounter = sut.serviceRegistryChangeCounter();

    ASSERT_EQ(serviceCounter->load(), 1U);
}
} // namespace test
} // namespace iox
