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

#include "iceoryx_posh/internal/roudi/port_pool_data.hpp"
#include "iceoryx_posh/internal/runtime/node_data.hpp"
#include "iceoryx_posh/popo/typed_subscriber.hpp"
#include "iceoryx_posh/roudi/port_pool.hpp"
#include "test.hpp"

using namespace ::testing;
using ::testing::Return;
using namespace iox::capro;

namespace test
{
static constexpr uint32_t DEFAULT_DEVICE_ID{0u};
static constexpr uint32_t DEFAULT_MEMORY_TYPE{0u};
class PortPool_test : public Test
{
  public:
    iox::roudi::PortPoolData portPoolData;
    iox::roudi::PortPool sut{portPoolData};

    iox::capro::ServiceDescription serviceDescription{"service1", "instance1"};
    iox::ProcessName_t applicationName{"AppNmae"};
    iox::mepoo::MemoryManager memoryManager;
    iox::popo::PublisherOptions publisherOptions;
    iox::popo::SubscriberOptions subscriberOptions;
};

TEST_F(PortPool_test, AddNodeDataSuccessfully)
{
    auto nodeData = sut.addNodeData("processName", "nodeName", 999U);

    ASSERT_THAT(nodeData.value()->m_process, Eq(iox::ProcessName_t("processName")));
    ASSERT_THAT(nodeData.value()->m_node, Eq(iox::ProcessName_t("nodeName")));
    ASSERT_THAT(nodeData.value()->m_nodeDeviceIdentifier, Eq(999U));
}


TEST_F(PortPool_test, AddMaxNodeDataSuccessfully)
{
    iox::cxx::vector<iox::runtime::NodeData*, iox::MAX_NODE_NUMBER> nodeContainer;

    for (uint32_t i = 1U; i <= iox::MAX_NODE_NUMBER; ++i)
    {
        auto nodeData = sut.addNodeData("processName", "nodeName", i);
        if (!nodeData.has_error())
        {
            nodeContainer.push_back(nodeData.value());
        }
    }

    ASSERT_THAT(nodeContainer.size(), Eq(iox::MAX_NODE_NUMBER));
}


TEST_F(PortPool_test, AddNodeDataFailsWhenNodeListIsFull)
{
    auto errorHandlerCalled{false};
    iox::Error errorHandlerType;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&](const iox::Error error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerType = error;
            errorHandlerCalled = true;
        });

    for (uint32_t i = 0U; i <= iox::MAX_NODE_NUMBER; ++i)
    {
        sut.addNodeData("processName", "nodeName", i);
    }

    ASSERT_THAT(errorHandlerCalled, Eq(true));
    EXPECT_EQ(errorHandlerType, iox::Error::kPORT_POOL__NODELIST_OVERFLOW);
}


TEST_F(PortPool_test, GetNodeDataListSuccessfully)
{
    sut.addNodeData("processName", "nodeName", 999U);
    auto nodeDataList = sut.getNodeDataList();

    ASSERT_THAT(nodeDataList.size(), Eq(1U));
}

TEST_F(PortPool_test, GetMaxNodeDataListSuccessfully)
{
    iox::cxx::vector<iox::runtime::NodeData*, iox::MAX_NODE_NUMBER> nodeContainer;

    for (uint32_t i = 1U; i <= iox::MAX_NODE_NUMBER; ++i)
    {
        auto nodeData = sut.addNodeData("processName", "nodeName", i);
        if (!nodeData.has_error())
        {
            nodeContainer.push_back(nodeData.value());
        }
    }

    auto nodeDataList = sut.getNodeDataList();
    ASSERT_THAT(nodeDataList.size(), Eq(iox::MAX_NODE_NUMBER));
}

TEST_F(PortPool_test, RemoveNodeDataSuccessfully)
{
    auto nodeData = sut.addNodeData("processName", "nodeName", 999U);
    sut.removeNodeData(nodeData.value());
    auto nodeDataList = sut.getNodeDataList();

    ASSERT_THAT(nodeDataList.size(), Eq(0U));
}

TEST_F(PortPool_test, AddPublisherPortSuccessfully)
{
    auto publisherPort = sut.addPublisherPort(serviceDescription, &memoryManager, applicationName, publisherOptions);

    EXPECT_THAT(publisherPort.value()->m_serviceDescription, Eq(ServiceDescription{"service1", "instance1"}));
    EXPECT_EQ(publisherPort.value()->m_processName, iox::ProcessName_t{"AppNmae"});
    EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_historyCapacity, 0UL);
    EXPECT_EQ(publisherPort.value()->m_nodeName, iox::NodeName_t{""});
    EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
    EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
}

TEST_F(PortPool_test, AddMaxPublisherPortSuccessfully)
{
    for (uint32_t i = 0; i < iox::MAX_PUBLISHERS; ++i)
    {
        std::string service = "service" + std::to_string(i);
        std::string instance = "instance" + std::to_string(i);
        iox::ProcessName_t applicationName = {iox::cxx::TruncateToCapacity, "AppNmae" + std::to_string(i)};


        auto publisherPort = sut.addPublisherPort({iox::capro::IdString_t(iox::cxx::TruncateToCapacity, service),
                                                   iox::capro::IdString_t(iox::cxx::TruncateToCapacity, instance)},
                                                  &memoryManager,
                                                  applicationName,
                                                  publisherOptions);

        EXPECT_THAT(publisherPort.value()->m_serviceDescription,
                    Eq(ServiceDescription{iox::capro::IdString_t(iox::cxx::TruncateToCapacity, service),
                                          iox::capro::IdString_t(iox::cxx::TruncateToCapacity, instance)}));
        EXPECT_EQ(publisherPort.value()->m_processName, applicationName);
        EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_historyCapacity, 0UL);
        EXPECT_EQ(publisherPort.value()->m_nodeName, iox::NodeName_t{""});
        EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
        EXPECT_EQ(publisherPort.value()->m_chunkSenderData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
    }
}

TEST_F(PortPool_test, AddPublisherPortOverflow)
{
    auto errorHandlerCalled{false};
    iox::Error errorHandlerType;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&](const iox::Error error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerType = error;
            errorHandlerCalled = true;
        });

    for (uint32_t i = 0; i <= iox::MAX_PUBLISHERS; ++i)
    {
        std::string service = "service" + std::to_string(i);
        std::string instance = "instance" + std::to_string(i);
        iox::ProcessName_t applicationName = {iox::cxx::TruncateToCapacity, "AppNmae" + std::to_string(i)};


        auto publisherPort = sut.addPublisherPort({iox::capro::IdString_t(iox::cxx::TruncateToCapacity, service),
                                                   iox::capro::IdString_t(iox::cxx::TruncateToCapacity, instance)},
                                                  &memoryManager,
                                                  applicationName,
                                                  publisherOptions);
    }

    ASSERT_THAT(errorHandlerCalled, Eq(true));
    EXPECT_EQ(errorHandlerType, iox::Error::kPORT_POOL__PUBLISHERLIST_OVERFLOW);
}

TEST_F(PortPool_test, GetPublisherPortDataListSuccessfully)
{
    sut.addPublisherPort(serviceDescription, &memoryManager, applicationName, publisherOptions);
    auto publisherPortDataList = sut.getPublisherPortDataList();

    ASSERT_THAT(publisherPortDataList.size(), Eq(1U));
}

TEST_F(PortPool_test, GetPublisherPortDataListCompletelyFilledSuccessfully)
{
    for (uint32_t i = 0; i < iox::MAX_PUBLISHERS; ++i)
    {
        std::string service = "service" + std::to_string(i);
        std::string instance = "instance" + std::to_string(i);
        iox::ProcessName_t applicationName = {iox::cxx::TruncateToCapacity, "AppNmae" + std::to_string(i)};


        auto publisherPort = sut.addPublisherPort({iox::capro::IdString_t(iox::cxx::TruncateToCapacity, service),
                                                   iox::capro::IdString_t(iox::cxx::TruncateToCapacity, instance)},
                                                  &memoryManager,
                                                  applicationName,
                                                  publisherOptions);
    }

    auto publisherPortDataList = sut.getPublisherPortDataList();

    ASSERT_THAT(publisherPortDataList.size(), Eq(iox::MAX_PUBLISHERS));
}

TEST_F(PortPool_test, RemovePublisherPortSuccessfully)
{
    auto publisherPort = sut.addPublisherPort(serviceDescription, &memoryManager, applicationName, publisherOptions);
    sut.removePublisherPort(publisherPort.value());
    auto publisherPortDataList = sut.getPublisherPortDataList();

    ASSERT_THAT(publisherPortDataList.size(), Eq(0U));
}

TEST_F(PortPool_test, AddSubscriberPortSuccessfully)
{
    auto subscriberPort = sut.addSubscriberPort(serviceDescription, applicationName, subscriberOptions);

    EXPECT_THAT(subscriberPort.value()->m_serviceDescription, Eq(ServiceDescription{"service1", "instance1"}));
    EXPECT_EQ(subscriberPort.value()->m_processName, iox::ProcessName_t{"AppNmae"});
    EXPECT_EQ(subscriberPort.value()->m_nodeName, iox::NodeName_t{""});
    EXPECT_EQ(subscriberPort.value()->m_historyRequest, 0U);
    EXPECT_EQ(subscriberPort.value()->m_chunkReceiverData.m_queue.capacity(), 256U);
    EXPECT_EQ(subscriberPort.value()->m_chunkReceiverData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
    EXPECT_EQ(subscriberPort.value()->m_chunkReceiverData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
}

TEST_F(PortPool_test, AddMaxSubscriberPortSuccessfully)
{
    for (uint32_t i = 0; i < iox::MAX_SUBSCRIBERS; ++i)
    {
        std::string service = "service" + std::to_string(i);
        std::string instance = "instance" + std::to_string(i);
        iox::ProcessName_t applicationName = {iox::cxx::TruncateToCapacity, "AppNmae" + std::to_string(i)};


        auto subscriberPort = sut.addSubscriberPort({iox::capro::IdString_t(iox::cxx::TruncateToCapacity, service),
                                                     iox::capro::IdString_t(iox::cxx::TruncateToCapacity, instance)},
                                                    applicationName,
                                                    subscriberOptions);

        EXPECT_THAT(subscriberPort.value()->m_serviceDescription,
                    Eq(ServiceDescription{iox::capro::IdString_t(iox::cxx::TruncateToCapacity, service),
                                          iox::capro::IdString_t(iox::cxx::TruncateToCapacity, instance)}));
        EXPECT_EQ(subscriberPort.value()->m_processName, applicationName);
        EXPECT_EQ(subscriberPort.value()->m_nodeName, iox::NodeName_t{""});
        EXPECT_EQ(subscriberPort.value()->m_chunkReceiverData.m_memoryInfo.deviceId, DEFAULT_DEVICE_ID);
        EXPECT_EQ(subscriberPort.value()->m_chunkReceiverData.m_memoryInfo.memoryType, DEFAULT_MEMORY_TYPE);
    }
}


TEST_F(PortPool_test, AddSubscriberPortOverflow)
{
    auto errorHandlerCalled{false};
    iox::Error errorHandlerType;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&](const iox::Error error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerType = error;
            errorHandlerCalled = true;
        });

    for (uint32_t i = 0; i <= iox::MAX_SUBSCRIBERS; ++i)
    {
        std::string service = "service" + std::to_string(i);
        std::string instance = "instance" + std::to_string(i);
        iox::ProcessName_t applicationName = {iox::cxx::TruncateToCapacity, "AppNmae" + std::to_string(i)};


        auto publisherPort = sut.addSubscriberPort({iox::capro::IdString_t(iox::cxx::TruncateToCapacity, service),
                                                    iox::capro::IdString_t(iox::cxx::TruncateToCapacity, instance)},
                                                   applicationName,
                                                   subscriberOptions);
    }

    ASSERT_THAT(errorHandlerCalled, Eq(true));
    EXPECT_EQ(errorHandlerType, iox::Error::kPORT_POOL__SUBSCRIBERLIST_OVERFLOW);
}

TEST_F(PortPool_test, GetSubscriberPortDataListSuccessfully)
{
    auto subscriberPort = sut.addSubscriberPort(serviceDescription, applicationName, subscriberOptions);
    auto subscriberPortDataList = sut.getSubscriberPortDataList();

    ASSERT_THAT(subscriberPortDataList.size(), Eq(1U));
}

TEST_F(PortPool_test, GetSubscriberPortDataListCompletelyFilledSuccessfully)
{
    for (uint32_t i = 0; i < iox::MAX_SUBSCRIBERS; ++i)
    {
        std::string service = "service" + std::to_string(i);
        std::string instance = "instance" + std::to_string(i);
        iox::ProcessName_t applicationName = {iox::cxx::TruncateToCapacity, "AppNmae" + std::to_string(i)};


        auto publisherPort = sut.addSubscriberPort({iox::capro::IdString_t(iox::cxx::TruncateToCapacity, service),
                                                    iox::capro::IdString_t(iox::cxx::TruncateToCapacity, instance)},
                                                   applicationName,
                                                   subscriberOptions);
    }
    auto subscriberPortDataList = sut.getSubscriberPortDataList();

    ASSERT_THAT(subscriberPortDataList.size(), Eq(iox::MAX_SUBSCRIBERS));
}

TEST_F(PortPool_test, RemoveSubscriberPortSuccessfully)
{
    auto subscriberPort = sut.addSubscriberPort(serviceDescription, applicationName, subscriberOptions);

    sut.removeSubscriberPort(subscriberPort.value());
    auto subscriberPortDataList = sut.getSubscriberPortDataList();

    ASSERT_THAT(subscriberPortDataList.size(), Eq(0U));
}

} // namespace test
