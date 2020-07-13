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

#include "iceoryx_posh/internal/roudi_environment/roudi_environment.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "test.hpp"

using namespace ::testing;
using namespace iox::runtime;
using iox::roudi::RouDiEnvironment;

class PoshRuntime_test : public Test
{
  public:
    PoshRuntime_test()
    {
    }

    virtual ~PoshRuntime_test()
    {
    }

    virtual void SetUp(){};
    virtual void TearDown(){};

    void InterOpWait()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    RouDiEnvironment m_roudiEnv{iox::RouDiConfig_t().setDefaults()};
    const std::string m_appName = "/sender";
    iox::runtime::PoshRuntime* m_senderRuntime{&iox::runtime::PoshRuntime::getInstance("/sender")};
    iox::runtime::PoshRuntime* m_receiverRuntime{&iox::runtime::PoshRuntime::getInstance("/receiver")};
    iox::runtime::MqMessage m_sendBuffer;
    iox::runtime::MqMessage m_receiveBuffer;
    const iox::cxx::CString100& m_runnableName = iox::cxx::CString100("testRunnable");
    const iox::cxx::CString100& m_invalidRunnableName = iox::cxx::CString100("invalidRunnable,");
};


TEST_F(PoshRuntime_test, SendMessageToRouDi)
{
    m_sendBuffer << mqMessageTypeToString(MqMessageType::IMPL_INTERFACE) << m_appName
                 << static_cast<uint32_t>(iox::capro::Interfaces::INTERNAL) << m_runnableName;

    auto status = m_senderRuntime->sendMessageToRouDi(m_sendBuffer);

    EXPECT_EQ(true, status);
}


TEST_F(PoshRuntime_test, SendMessageToRouDiInvalidMessage)
{
    m_sendBuffer << mqMessageTypeToString(MqMessageType::IMPL_INTERFACE) << std::string()
                 << static_cast<uint32_t>(iox::capro::Interfaces::INTERNAL) << m_invalidRunnableName;

    auto status = m_senderRuntime->sendMessageToRouDi(m_sendBuffer);

    EXPECT_EQ(false, status);
}


TEST_F(PoshRuntime_test, SendMessageToRouDiEmptyMessage)
{
    auto status = m_senderRuntime->sendMessageToRouDi(m_sendBuffer);

    EXPECT_EQ(true, status);
}


TEST_F(PoshRuntime_test, SendRequestToRouDi)
{
    m_sendBuffer << mqMessageTypeToString(MqMessageType::IMPL_INTERFACE) << m_appName
                 << static_cast<uint32_t>(iox::capro::Interfaces::INTERNAL) << m_runnableName;

    auto status = m_senderRuntime->sendRequestToRouDi(m_sendBuffer, m_receiveBuffer);

    EXPECT_EQ(true, status);
}


TEST_F(PoshRuntime_test, SendRequestToRouDiInvalidMessage)
{
    m_sendBuffer << mqMessageTypeToString(MqMessageType::IMPL_INTERFACE) << m_appName
                 << static_cast<uint32_t>(iox::capro::Interfaces::INTERNAL) << m_invalidRunnableName;

    auto status = m_senderRuntime->sendRequestToRouDi(m_sendBuffer, m_receiveBuffer);

    EXPECT_EQ(false, status);
}


TEST_F(PoshRuntime_test, GetMiddlewareSender)
{
    iox::capro::ServiceDescription serviceDescription{99, 1, 20};
    iox::runtime::PortConfigInfo portConfg(11, 22, 33);

    auto senderPort = m_senderRuntime->getMiddlewareSender(serviceDescription, m_runnableName, portConfg);

    EXPECT_EQ(99, senderPort->m_serviceDescription.getServiceID());
    EXPECT_EQ(1, senderPort->m_serviceDescription.getEventID());
    EXPECT_EQ(20, senderPort->m_serviceDescription.getInstanceID());
    EXPECT_EQ(22, senderPort->m_memoryInfo.deviceId);
    EXPECT_EQ(33, senderPort->m_memoryInfo.memoryType);
}


TEST_F(PoshRuntime_test, GetMiddlewareSenderDefaultArgs)
{
    iox::capro::ServiceDescription serviceDescription{99, 1, 20};

    auto senderPort = m_senderRuntime->getMiddlewareSender(serviceDescription);

    EXPECT_EQ(0, senderPort->m_memoryInfo.deviceId);
    EXPECT_EQ(0, senderPort->m_memoryInfo.memoryType);
}


TEST_F(PoshRuntime_test, GetMiddlewareReceiver)
{
    iox::capro::ServiceDescription serviceDescription{99, 1, 20};
    iox::runtime::PortConfigInfo portConfg(11, 22, 33);

    auto receiverPort = m_receiverRuntime->getMiddlewareReceiver(serviceDescription, m_runnableName, portConfg);

    EXPECT_EQ(99, receiverPort->m_serviceDescription.getServiceID());
    EXPECT_EQ(1, receiverPort->m_serviceDescription.getEventID());
    EXPECT_EQ(20, receiverPort->m_serviceDescription.getInstanceID());
    EXPECT_EQ(22, receiverPort->m_memoryInfo.deviceId);
    EXPECT_EQ(33, receiverPort->m_memoryInfo.memoryType);
}


TEST_F(PoshRuntime_test, GetMiddlewareReceiverDefaultArgs)
{
    iox::capro::ServiceDescription serviceDescription{99, 1, 20};

    auto receiverPort = m_receiverRuntime->getMiddlewareReceiver(serviceDescription);

    EXPECT_EQ(0, receiverPort->m_memoryInfo.deviceId);
    EXPECT_EQ(0, receiverPort->m_memoryInfo.memoryType);
}


TEST_F(PoshRuntime_test, CreateRunnable)
{
    uint64_t runnableDeviceIdentifier = 0u;
    iox::runtime::RunnableProperty runnableProperty(iox::cxx::CString100("testRunnable"), runnableDeviceIdentifier);

    auto runableData = m_senderRuntime->createRunnable(runnableProperty);

    EXPECT_EQ(runnableDeviceIdentifier, runableData->m_runnableDeviceIdentifier);
    EXPECT_EQ(m_appName, runableData->m_process);
    EXPECT_EQ(iox::cxx::CString100("testRunnable"), runableData->m_runnable);
}


TEST_F(PoshRuntime_test, DISABLED_GetServiceRegistryChangeCounter)
{
    uint64_t runnableDeviceIdentifier = 0u;
    iox::runtime::RunnableProperty runnableProperty(iox::cxx::CString100("testRunnable"), runnableDeviceIdentifier);

    auto runableData = m_senderRuntime->getServiceRegistryChangeCounter();

    std::cout << *runableData;
}

TEST_F(PoshRuntime_test, DISABLED_InterfacePort)
{
    m_senderRuntime->offerService({"service1", "instance1"});
    this->InterOpWait();
    auto interfacePortData = m_receiverRuntime->getMiddlewareInterface(iox::capro::Interfaces::INTERNAL);
    iox::popo::InterfacePort interfacePort(interfacePortData);
    iox::capro::CaproMessage caproMessage;
    this->InterOpWait();

    bool serviceFound = false;
    while (interfacePort.getCaProMessage(caproMessage))
    {
        if ((caproMessage.m_serviceDescription.getServiceIDString() == IdString("service1"))
            && (caproMessage.m_serviceDescription.getInstanceIDString() == IdString("instance1"))
            && ((caproMessage.m_serviceDescription.getEventIDString() == IdString(iox::capro::AnyEventString))))
        {
            serviceFound = true;
            break;
        }
    }

    EXPECT_THAT(serviceFound, Eq(true));
}


TEST_F(PoshRuntime_test, DISABLED_ApplicationPort)
{
    m_senderRuntime->offerService({"service1", "instance1"});
    this->InterOpWait();
    auto applicationPortData = m_receiverRuntime->getMiddlewareApplication();
    iox::popo::ApplicationPort applicationPort(applicationPortData);
    iox::capro::CaproMessage caproMessage;
    this->InterOpWait();

    bool serviceFound = false;
    while (applicationPort.getCaProMessage(caproMessage))
    {
        if ((caproMessage.m_serviceDescription.getServiceIDString() == IdString("service1"))
            && (caproMessage.m_serviceDescription.getInstanceIDString() == IdString("instance1"))
            && ((caproMessage.m_serviceDescription.getEventIDString() == IdString(iox::capro::AnyEventString))))
        {
            serviceFound = true;
            break;
        }
    }

    EXPECT_THAT(serviceFound, Eq(true));
}
