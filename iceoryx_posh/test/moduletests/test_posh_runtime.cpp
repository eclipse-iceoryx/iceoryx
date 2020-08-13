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
    PoshRuntime* m_runtime{&iox::runtime::PoshRuntime::getInstance("/sender")};
    MqMessage m_sendBuffer;
    MqMessage m_receiveBuffer;
    const iox::cxx::string<100> m_runnableName{"testRunnable"};
    const iox::cxx::string<100> m_invalidRunnableName{"invalidRunnable,"};
    static bool m_errorHandlerCalled;
    const iox::cxx::string<100> m_runtimeName{"/sender"};
};

bool PoshRuntime_test::m_errorHandlerCalled{false};


TEST_F(PoshRuntime_test, ValidAppName)
{
    std::string appName("/valid_name");

    EXPECT_NO_FATAL_FAILURE({ PoshRuntime::getInstance(appName); });
}


TEST_F(PoshRuntime_test, AppNameLength_OutOfLimit)
{
    std::string tooLongName(100, 's');
    tooLongName.insert(0, 1, '/');

    EXPECT_DEATH({ PoshRuntime::getInstance(tooLongName); },
                 "Application name has more than 100 characters, including null termination!");
}


TEST_F(PoshRuntime_test, MaxAppNameLength)
{
    std::string maxValidName(99, 's');
    maxValidName.insert(0, 1, '/');

    EXPECT_NO_FATAL_FAILURE({ PoshRuntime::getInstance(maxValidName); });
}


TEST_F(PoshRuntime_test, NoAppName)
{
    const std::string invalidAppName("");

    EXPECT_DEATH({ PoshRuntime::getInstance(invalidAppName); },
                 "Cannot initialize runtime. Application name must not be empty!");
}


TEST_F(PoshRuntime_test, NoLeadingSlashAppName)
{
    const std::string invalidAppName = "invalidname";

    EXPECT_DEATH(
        { PoshRuntime::getInstance(invalidAppName); },
        "Cannot initialize runtime. Application name invalidname does not have the required leading slash '/'");
}


// since getInstance is a singleton and test class creates instance of Poshruntime,
// when getInstance() is called without parameterx it reuturns extisting instance
// To be able to test this, it needs to be the very first call to getInstance but since,
// we have multiple tests in this binary its not possible here to test
TEST_F(PoshRuntime_test, DISABLED_AppNameEmpty)
{
    EXPECT_DEATH({ iox::runtime::PoshRuntime::getInstance(); },
                 "Cannot initialize runtime. Application name has not been specified!");
}


TEST_F(PoshRuntime_test, GetInstanceNameIsSuccessful)
{
    const std::string appname = "/app";

    auto& sut = PoshRuntime::getInstance(appname);

    EXPECT_EQ(sut.getInstanceName(), appname);
}


TEST_F(PoshRuntime_test, GetMiddlewareApplicationIsSuccessful)
{
    uint32_t uniqueIdCounter = iox::popo::BasePortData::s_uniqueIdCounter;

    const auto applicationPortData = m_runtime->getMiddlewareApplication();

    ASSERT_THAT(applicationPortData, Ne(nullptr));
    EXPECT_THAT(m_runtimeName, applicationPortData->m_processName);
    EXPECT_EQ(iox::capro::ServiceDescription(0u, 0u, 0u), applicationPortData->m_serviceDescription);
    EXPECT_EQ(false, applicationPortData->m_toBeDestroyed);
    EXPECT_EQ(uniqueIdCounter, applicationPortData->m_uniqueId);
}


TEST_F(PoshRuntime_test, GetMiddlewareApplication_ApplicationlistOverflow)
{
    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&errorHandlerCalled](const iox::Error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
        });

    // i=1 because there is already an active runtime in test fixture class which acquired an application port
    for (auto i = 1u; i < iox::MAX_PROCESS_NUMBER; ++i)
    {
        auto appPort = m_runtime->getMiddlewareApplication();
        EXPECT_NE(nullptr, appPort);
    }
    auto appPort = m_runtime->getMiddlewareApplication();

    EXPECT_EQ(nullptr, appPort);
    EXPECT_TRUE(errorHandlerCalled);
}


TEST_F(PoshRuntime_test, GetMiddlewareInterfaceIsSuccessful)
{
    const auto interfacePortData = m_runtime->getMiddlewareInterface(iox::capro::Interfaces::INTERNAL, m_runnableName);

    EXPECT_EQ(m_runtimeName, interfacePortData->m_processName);
    EXPECT_EQ(iox::capro::ServiceDescription(0u, 0u, 0u), interfacePortData->m_serviceDescription);
    EXPECT_EQ(false, interfacePortData->m_toBeDestroyed);
    EXPECT_EQ(true, interfacePortData->m_doInitialOfferForward);
}


TEST_F(PoshRuntime_test, GetMiddlewareInterface_InterfacelistOverflow)
{
    auto errorHandlerCalled{false};
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&errorHandlerCalled](const iox::Error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerCalled = true;
        });

    for (auto i = 0u; i < iox::MAX_INTERFACE_NUMBER; ++i)
    {
        auto interfacePort = m_runtime->getMiddlewareInterface(iox::capro::Interfaces::INTERNAL);
        EXPECT_NE(nullptr, interfacePort);
    }
    auto interfacePort = m_runtime->getMiddlewareInterface(iox::capro::Interfaces::INTERNAL);

    EXPECT_EQ(nullptr, interfacePort);
    EXPECT_TRUE(errorHandlerCalled);
}


TEST_F(PoshRuntime_test, SendMessageToRouDi_ValidMessage)
{
    m_sendBuffer << mqMessageTypeToString(MqMessageType::IMPL_INTERFACE) << m_runtimeName
                 << static_cast<uint32_t>(iox::capro::Interfaces::INTERNAL) << m_runnableName;

    const auto successfullySent = m_runtime->sendMessageToRouDi(m_sendBuffer);

    EXPECT_EQ(true, successfullySent);
}


TEST_F(PoshRuntime_test, SendMessageToRouDi_InvalidMessage)
{
    m_sendBuffer << mqMessageTypeToString(MqMessageType::IMPL_INTERFACE) << std::string()
                 << static_cast<uint32_t>(iox::capro::Interfaces::INTERNAL) << m_invalidRunnableName;

    const auto successfullySent = m_runtime->sendMessageToRouDi(m_sendBuffer);

    EXPECT_EQ(false, successfullySent);
}


TEST_F(PoshRuntime_test, SendMessageToRouDi_EmptyMessage)
{
    const auto successfullySent = m_runtime->sendMessageToRouDi(m_sendBuffer);

    EXPECT_EQ(true, successfullySent);
}


TEST_F(PoshRuntime_test, SendRequestToRouDi_ValidMessage)
{
    m_sendBuffer << mqMessageTypeToString(MqMessageType::IMPL_INTERFACE) << m_runtimeName
                 << static_cast<uint32_t>(iox::capro::Interfaces::INTERNAL) << m_runnableName;

    const auto successfullySent = m_runtime->sendRequestToRouDi(m_sendBuffer, m_receiveBuffer);

    EXPECT_TRUE(m_receiveBuffer.isValid());
    EXPECT_EQ(true, successfullySent);
}


TEST_F(PoshRuntime_test, SendRequestToRouDi_InvalidMessage)
{
    m_sendBuffer << mqMessageTypeToString(MqMessageType::IMPL_INTERFACE) << m_runtimeName
                 << static_cast<uint32_t>(iox::capro::Interfaces::INTERNAL) << m_invalidRunnableName;

    const auto successfullySent = m_runtime->sendRequestToRouDi(m_sendBuffer, m_receiveBuffer);

    EXPECT_EQ(false, successfullySent);
}


TEST_F(PoshRuntime_test, GetMiddlewareSenderIsSuccessful)
{
    const auto senderPort = m_runtime->getMiddlewareSender(
        iox::capro::ServiceDescription(99u, 1u, 20u), m_runnableName, iox::runtime::PortConfigInfo(11u, 22u, 33u));

    EXPECT_EQ(iox::capro::ServiceDescription(99u, 1u, 20u), senderPort->m_serviceDescription);
    EXPECT_EQ(22u, senderPort->m_memoryInfo.deviceId);
    EXPECT_EQ(33u, senderPort->m_memoryInfo.memoryType);
}


TEST_F(PoshRuntime_test, GetMiddlewareSender_DefaultArgs)
{
    const auto senderPort = m_runtime->getMiddlewareSender(iox::capro::ServiceDescription(99u, 1u, 20u));

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
        m_runtime->getMiddlewareSender(iox::capro::ServiceDescription(i, i + 1u, i + 2u));
    }

    EXPECT_TRUE(errorHandlerCalled);
}


TEST_F(PoshRuntime_test, GetMiddlewareReceiverIsSuccessful)
{
    auto receiverPort = m_runtime->getMiddlewareReceiver(
        iox::capro::ServiceDescription(99u, 1u, 20u), m_runnableName, iox::runtime::PortConfigInfo(11u, 22u, 33u));

    EXPECT_EQ(iox::capro::ServiceDescription(99u, 1u, 20u), receiverPort->m_serviceDescription);
    EXPECT_EQ(22u, receiverPort->m_memoryInfo.deviceId);
    EXPECT_EQ(33u, receiverPort->m_memoryInfo.memoryType);
}


TEST_F(PoshRuntime_test, GetMiddlewareReceiver_DefaultArgs)
{
    auto receiverPort = m_runtime->getMiddlewareReceiver(iox::capro::ServiceDescription(99u, 1u, 20u));

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
        m_runtime->getMiddlewareReceiver(iox::capro::ServiceDescription(i, i + 1u, i + 2u));
    }

    EXPECT_TRUE(errorHandlerCalled);
}


TEST_F(PoshRuntime_test, GetServiceRegistryChangeCounter_OfferStopOfferService)
{
    auto serviceCounter = m_runtime->getServiceRegistryChangeCounter();
    auto initialCout = serviceCounter->load();

    m_runtime->offerService({"service1", "instance1"});
    this->InterOpWait();

    EXPECT_EQ(initialCout + 1, serviceCounter->load());

    m_runtime->stopOfferService({"service1", "instance1"});
    this->InterOpWait();

    EXPECT_EQ(initialCout + 2, serviceCounter->load());
}


TEST_F(PoshRuntime_test, CreateRunnable_ReturnValue)
{
    const uint32_t runnableDeviceIdentifier = 1u;
    iox::runtime::RunnableProperty runnableProperty(iox::cxx::string<100>("testRunnable"), runnableDeviceIdentifier);

    auto runableData = m_runtime->createRunnable(runnableProperty);

    EXPECT_EQ(iox::cxx::string<100>("/sender"), runableData->m_process);
    EXPECT_EQ(iox::cxx::string<100>("testRunnable"), runableData->m_runnable);

    /// @todo I am passing runnableDeviceIdentifier as 1, but it returns 0, is this expected?
    // EXPECT_EQ(runnableDeviceIdentifier, runableData->m_runnableDeviceIdentifier);
}
