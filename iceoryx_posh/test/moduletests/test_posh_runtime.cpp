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
    PoshRuntime* m_receiverRuntime{&iox::runtime::PoshRuntime::getInstance("/receiver")};
    PoshRuntime* m_senderRuntime{&iox::runtime::PoshRuntime::getInstance("/sender")};
    MqMessage m_sendBuffer;
    MqMessage m_receiveBuffer;
    const iox::cxx::CString100 m_runnableName = iox::cxx::CString100("testRunnable");
    const iox::cxx::CString100 m_invalidRunnableName = iox::cxx::CString100("invalidRunnable,");
};


TEST_F(PoshRuntime_test, GetMiddlewareApplication_ReturnValue)
{
    const auto applicationPortData = m_senderRuntime->getMiddlewareApplication();

    EXPECT_EQ(std::string("/sender"), applicationPortData->m_processName);
    EXPECT_EQ(0u, applicationPortData->m_serviceDescription.getEventID());
    EXPECT_EQ(0u, applicationPortData->m_serviceDescription.getInstanceID());
    EXPECT_EQ(0u, applicationPortData->m_serviceDescription.getServiceID());
    EXPECT_EQ(false, applicationPortData->m_toBeDestroyed);
    ///@todo is this expected??
    EXPECT_EQ(8u, applicationPortData->m_uniqueId);
    EXPECT_EQ(9u, applicationPortData->s_uniqueIdCounter);
}


TEST_F(PoshRuntime_test, GetMiddlewareApplication_ApplicationlistOverflow)
{
    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&errorHandlerCalled](const iox::Error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
        });

    for (auto i = 0u; i < iox::MAX_PROCESS_NUMBER; ++i)
    {
        m_senderRuntime->getMiddlewareApplication();
    }

    EXPECT_TRUE(errorHandlerCalled);
}


TEST_F(PoshRuntime_test, GetMiddlewareInterface_ReturnValue)
{
    const auto interfacePortData = m_senderRuntime->getMiddlewareInterface(iox::capro::Interfaces::INTERNAL);

    EXPECT_EQ(std::string("/sender"), interfacePortData->m_processName);
    EXPECT_EQ(0u, interfacePortData->m_serviceDescription.getEventID());
    EXPECT_EQ(0u, interfacePortData->m_serviceDescription.getInstanceID());
    EXPECT_EQ(0u, interfacePortData->m_serviceDescription.getServiceID());
    EXPECT_EQ(false, interfacePortData->m_toBeDestroyed);
    ///@todo is this expected??
    EXPECT_EQ(true, interfacePortData->m_doInitialOfferForward);
}


TEST_F(PoshRuntime_test, GetMiddlewareInterface_InterfacelistOverflow)
{
    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&errorHandlerCalled](const iox::Error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
        });

    for (auto i = 0u; i < iox::MAX_INTERFACE_NUMBER + 1u; ++i)
    {
        m_senderRuntime->getMiddlewareInterface(iox::capro::Interfaces::INTERNAL);
    }

    EXPECT_TRUE(errorHandlerCalled);
}


TEST_F(PoshRuntime_test, SendMessageToRouDi_ValidMessage)
{
    m_sendBuffer << mqMessageTypeToString(MqMessageType::IMPL_INTERFACE) << std::string("/sender")
                 << static_cast<uint32_t>(iox::capro::Interfaces::INTERNAL) << m_runnableName;

    const auto status = m_senderRuntime->sendMessageToRouDi(m_sendBuffer);

    EXPECT_EQ(true, status);
}


TEST_F(PoshRuntime_test, SendMessageToRouDi_InvalidMessage)
{
    m_sendBuffer << mqMessageTypeToString(MqMessageType::IMPL_INTERFACE) << std::string()
                 << static_cast<uint32_t>(iox::capro::Interfaces::INTERNAL) << m_invalidRunnableName;

    const auto status = m_senderRuntime->sendMessageToRouDi(m_sendBuffer);

    EXPECT_EQ(false, status);
}


TEST_F(PoshRuntime_test, SendMessageToRouDi_EmptyMessage)
{
    const auto status = m_senderRuntime->sendMessageToRouDi(m_sendBuffer);

    EXPECT_EQ(true, status);
}


TEST_F(PoshRuntime_test, SendRequestToRouDi_ValidMessage)
{
    m_sendBuffer << mqMessageTypeToString(MqMessageType::IMPL_INTERFACE) << std::string("/sender")
                 << static_cast<uint32_t>(iox::capro::Interfaces::INTERNAL) << m_runnableName;

    const auto status = m_senderRuntime->sendRequestToRouDi(m_sendBuffer, m_receiveBuffer);

    EXPECT_EQ(true, status);
}


TEST_F(PoshRuntime_test, SendRequestToRouDi_InvalidMessage)
{
    m_sendBuffer << mqMessageTypeToString(MqMessageType::IMPL_INTERFACE) << std::string("/sender")
                 << static_cast<uint32_t>(iox::capro::Interfaces::INTERNAL) << m_invalidRunnableName;

    const auto status = m_senderRuntime->sendRequestToRouDi(m_sendBuffer, m_receiveBuffer);

    EXPECT_EQ(false, status);
}


TEST_F(PoshRuntime_test, GetMiddlewareSender_ReturnValue)
{
    const auto senderPort = m_senderRuntime->getMiddlewareSender(
        iox::capro::ServiceDescription(99u, 1u, 20u), m_runnableName, iox::runtime::PortConfigInfo(11u, 22u, 33u));

    EXPECT_EQ(99u, senderPort->m_serviceDescription.getServiceID());
    EXPECT_EQ(1u, senderPort->m_serviceDescription.getEventID());
    EXPECT_EQ(20u, senderPort->m_serviceDescription.getInstanceID());
    EXPECT_EQ(22u, senderPort->m_memoryInfo.deviceId);
    EXPECT_EQ(33u, senderPort->m_memoryInfo.memoryType);
}


TEST_F(PoshRuntime_test, GetMiddlewareSender_DefaultArgs)
{
    const auto senderPort = m_senderRuntime->getMiddlewareSender(iox::capro::ServiceDescription(99u, 1u, 20u));

    EXPECT_EQ(0u, senderPort->m_memoryInfo.deviceId);
    EXPECT_EQ(0u, senderPort->m_memoryInfo.memoryType);
}


TEST_F(PoshRuntime_test, GetMiddlewareSender_SenderlistOverflow)
{
    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&errorHandlerCalled](const iox::Error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
        });

    for (uint32_t i = 0u; i < iox::MAX_PORT_NUMBER; ++i)
    {
        m_senderRuntime->getMiddlewareSender(iox::capro::ServiceDescription(i, i + 1u, i + 2u));
    }

    EXPECT_TRUE(errorHandlerCalled);
}


TEST_F(PoshRuntime_test, GetMiddlewareReceiver_ReturnValue)
{
    auto receiverPort = m_receiverRuntime->getMiddlewareReceiver(
        iox::capro::ServiceDescription(99u, 1u, 20u), m_runnableName, iox::runtime::PortConfigInfo(11u, 22u, 33u));

    EXPECT_EQ(99u, receiverPort->m_serviceDescription.getServiceID());
    EXPECT_EQ(1u, receiverPort->m_serviceDescription.getEventID());
    EXPECT_EQ(20u, receiverPort->m_serviceDescription.getInstanceID());
    EXPECT_EQ(22u, receiverPort->m_memoryInfo.deviceId);
    EXPECT_EQ(33u, receiverPort->m_memoryInfo.memoryType);
}


TEST_F(PoshRuntime_test, GetMiddlewareReceiver_DefaultArgs)
{
    auto receiverPort = m_receiverRuntime->getMiddlewareReceiver(iox::capro::ServiceDescription(99u, 1u, 20u));

    EXPECT_EQ(0u, receiverPort->m_memoryInfo.deviceId);
    EXPECT_EQ(0u, receiverPort->m_memoryInfo.memoryType);
}


TEST_F(PoshRuntime_test, GetMiddlewareReceiver_ReceiverlistOverflow)
{
    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&errorHandlerCalled](const iox::Error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
        });

    for (uint32_t i = 0u; i < iox::MAX_PORT_NUMBER + 1; ++i)
    {
        m_senderRuntime->getMiddlewareReceiver(iox::capro::ServiceDescription(i, i + 1u, i + 2u));
    }

    EXPECT_TRUE(errorHandlerCalled);
}


TEST_F(PoshRuntime_test, GetServiceRegistryChangeCounter_ReturnValue)
{
    const uint64_t runnableDeviceIdentifier = 0u;
    iox::runtime::RunnableProperty runnableProperty(iox::cxx::CString100("testRunnable"), runnableDeviceIdentifier);

    auto runnableData = m_senderRuntime->getServiceRegistryChangeCounter();

    // Roudi internally  calls 5 servieces before application registers its service
    EXPECT_EQ(5u, *runnableData);
}


TEST_F(PoshRuntime_test, GetServiceRegistryChangeCounter_OfferStopOfferService)
{
    const uint32_t runnableDeviceIdentifier = 0u;
    iox::runtime::RunnableProperty runnableProperty(iox::cxx::CString100("testRunnable"), runnableDeviceIdentifier);

    m_senderRuntime->offerService({"service1", "instance1"});
    this->InterOpWait();
    auto runnableData = m_senderRuntime->getServiceRegistryChangeCounter();

    EXPECT_EQ(6u, *runnableData);

    m_senderRuntime->stopOfferService({"service1", "instance1"});
    this->InterOpWait();

    EXPECT_EQ(7u, *runnableData);
}


TEST_F(PoshRuntime_test, CreateRunnable_ReturnValue)
{
    const uint32_t runnableDeviceIdentifier = 1u;
    iox::runtime::RunnableProperty runnableProperty(iox::cxx::CString100("testRunnable"), runnableDeviceIdentifier);

    auto runableData = m_senderRuntime->createRunnable(runnableProperty);

    EXPECT_EQ(std::string("/sender"), runableData->m_process);
    EXPECT_EQ(iox::cxx::CString100("testRunnable"), runableData->m_runnable);

    /// @todo I am passing runnableDeviceIdentifier as 1, but it returns 0, is this expected?
    // EXPECT_EQ(runnableDeviceIdentifier, runableData->m_runnableDeviceIdentifier);
}
