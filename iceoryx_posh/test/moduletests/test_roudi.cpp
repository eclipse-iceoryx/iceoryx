// Copyright (c) 2021 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/internal/roudi/roudi.hpp"
#include "iceoryx_posh/internal/roudi_environment/roudi_environment.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/roudi/iceoryx_roudi_components.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/runtime/message_queue_message.hpp"

#include <chrono>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;
using ::testing::Return;

using namespace iox::roudi;
using namespace iox::runtime;

namespace iox
{
namespace test
{
/// @req IOX_SWRS_112, IOX_SWRS_200
/// @brief Test goal: "This test suite verifies that the BaseClass function is verified"
/// @pre describe what needs to be done in setup()
/// @post describe what needs to be done in teardown()
/// @note name of the Testfixture should match to the Class you want to test

class RoudiClassTest : public RouDi
{
public:
    RoudiClassTest(iox::roudi::RouDiMemoryInterface& roudiMemoryInterface,
             iox::roudi::PortManager& portManager,
             iox::roudi::RouDi::RoudiStartupParameters roudiStartupParameters): RouDi(roudiMemoryInterface, portManager, roudiStartupParameters)
    {   
    }

    void startMQThreadTest()
    {
        this->startMQThread();
    }

    void shutDownTest()
    {
        this->shutdown();
    }

    void processMessageTest(const runtime::MqMessage& message,
                                const iox::runtime::MqMessageType& cmd,
                                const ProcessName_t& processName)
    {
        this->processMessage(message,
                                cmd,
                                processName);
    }

    const PortManager* getPortManager()
    {
        return m_portManager;
    }
};

class RouDiBaseClass_test : public Test
{
  public:


};

TEST_F(RouDiBaseClass_test, ConstructorIsSuccessfull)
{
    std::unique_ptr<IceOryxRouDiComponents> m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(iox::RouDiConfig_t().setDefaults()));
    std::unique_ptr<RoudiClassTest> m_roudiApp = std::unique_ptr<RoudiClassTest>(new RoudiClassTest(m_roudiComponents->m_rouDiMemoryManager,
                                                  m_roudiComponents->m_portManager,
                                                  RouDi::RoudiStartupParameters{roudi::MonitoringMode::ON, false}));

    EXPECT_THAT(m_roudiApp, Not(Eq(nullptr)));

    m_roudiApp->shutDownTest();
}

TEST_F(RouDiBaseClass_test, ConstructorThreadStartNotImmediateIsSuccessfull)
{
    std::unique_ptr<IceOryxRouDiComponents> m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(iox::RouDiConfig_t().setDefaults()));
    std::unique_ptr<RoudiClassTest> m_roudiApp = std::unique_ptr<RoudiClassTest>(new RoudiClassTest(m_roudiComponents->m_rouDiMemoryManager,
                                                  m_roudiComponents->m_portManager,
                                                  RouDi::RoudiStartupParameters{roudi::MonitoringMode::ON, false, RouDi::MQThreadStart::DEFER_START}));

    m_roudiApp->startMQThreadTest();

    EXPECT_DEATH(m_roudiApp->startMQThreadTest(), ".*");
    
    m_roudiApp->shutDownTest();
}

TEST_F(RouDiBaseClass_test, ShutDownKillProcessInDestructorTrueIsSuccessfull)
{
    std::unique_ptr<IceOryxRouDiComponents> m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(iox::RouDiConfig_t().setDefaults()));
    std::unique_ptr<RoudiClassTest> m_roudiApp = std::unique_ptr<RoudiClassTest>(new RoudiClassTest(m_roudiComponents->m_rouDiMemoryManager,
                                                  m_roudiComponents->m_portManager,
                                                  RouDi::RoudiStartupParameters{roudi::MonitoringMode::ON, true}));
    MqMessage message;
    message << mqMessageTypeToString(MqMessageType::CREATE_PUBLISHER) << "AppName" << "123123" << "456" << "789" << "123";
    auto cmd = runtime::stringToMqMessageType(message.getElementAtIndex(0).c_str());
    std::string processName = message.getElementAtIndex(1);        

    m_roudiApp->processMessageTest(message,cmd, ProcessName_t(cxx::TruncateToCapacity, processName));

    m_roudiApp->shutDownTest();
}

TEST_F(RouDiBaseClass_test, ShutDownCalledTwoTimesIsSuccessfull)
{
    std::unique_ptr<IceOryxRouDiComponents> m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(iox::RouDiConfig_t().setDefaults()));
    std::unique_ptr<RoudiClassTest> m_roudiApp = std::unique_ptr<RoudiClassTest>(new RoudiClassTest(m_roudiComponents->m_rouDiMemoryManager,
                                                  m_roudiComponents->m_portManager,
                                                  RouDi::RoudiStartupParameters{roudi::MonitoringMode::ON, true}));
    
    m_roudiApp->shutDownTest();

    m_roudiApp->shutDownTest();
}

TEST_F(RouDiBaseClass_test, SendCorrectRegMessagetoRoudiReturnSuccessfullySent)
{
    const iox::ProcessName_t m_runtimeName{"App"};
    RouDiEnvironment m_roudiEnv{iox::RouDiConfig_t().setDefaults()};
    PoshRuntime* m_runtime{&iox::runtime::PoshRuntime::initRuntime(m_runtimeName)};

    MqMessage message;
    MqMessage m_receiveBuffer;
    const iox::NodeName_t m_nodeName{"testNode"};
    message << mqMessageTypeToString(MqMessageType::REG) << "App" << std::to_string(getpid())
                       << std::to_string(posix::PosixUser::getUserOfCurrentProcess().getID())
                       << std::to_string(0)
                       << static_cast<cxx::Serialization>(version::VersionInfo::getCurrentVersion()).toString();    

    const auto successfullySent = m_runtime->sendRequestToRouDi(message, m_receiveBuffer);

    EXPECT_TRUE(m_receiveBuffer.isValid());
    EXPECT_TRUE(successfullySent);
}

TEST_F(RouDiBaseClass_test, SendWrongRegMessagetoRoudiReturnError)
{
    std::unique_ptr<IceOryxRouDiComponents> m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(iox::RouDiConfig_t().setDefaults()));
    std::unique_ptr<RoudiClassTest> m_roudiApp = std::unique_ptr<RoudiClassTest>(new RoudiClassTest(m_roudiComponents->m_rouDiMemoryManager,
                                                  m_roudiComponents->m_portManager,
                                                  RouDi::RoudiStartupParameters{roudi::MonitoringMode::ON, true}));

    MqMessage message;
    message << mqMessageTypeToString(MqMessageType::REG)
               << "App" << "123123";
    auto cmd = runtime::stringToMqMessageType(message.getElementAtIndex(0).c_str());
    std::string processName = message.getElementAtIndex(1);    

    m_roudiApp->processMessageTest(message,cmd, ProcessName_t(cxx::TruncateToCapacity, processName));
}

TEST_F(RouDiBaseClass_test, ProcessMessageChangeCounter)
{
    std::unique_ptr<IceOryxRouDiComponents> m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(iox::RouDiConfig_t().setDefaults()));
    std::unique_ptr<RoudiClassTest> m_roudiApp = std::unique_ptr<RoudiClassTest>(new RoudiClassTest(m_roudiComponents->m_rouDiMemoryManager,
                                                  m_roudiComponents->m_portManager,
                                                  RouDi::RoudiStartupParameters{roudi::MonitoringMode::ON, true}));
    
    MqMessage message;
    message << mqMessageTypeToString(MqMessageType::SERVICE_REGISTRY_CHANGE_COUNTER)
               << "AppName" << "123123";
    auto cmd = runtime::stringToMqMessageType(message.getElementAtIndex(0).c_str());
    std::string processName = message.getElementAtIndex(1);

    m_roudiApp->processMessageTest(message,cmd, ProcessName_t(cxx::TruncateToCapacity, processName));

    m_roudiApp->shutDownTest();
}

TEST_F(RouDiBaseClass_test, ProcessMessageCreatePublisher)
{
    std::unique_ptr<IceOryxRouDiComponents> m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(iox::RouDiConfig_t().setDefaults()));
    std::unique_ptr<RoudiClassTest> m_roudiApp = std::unique_ptr<RoudiClassTest>(new RoudiClassTest(m_roudiComponents->m_rouDiMemoryManager,
                                                  m_roudiComponents->m_portManager,
                                                  RouDi::RoudiStartupParameters{roudi::MonitoringMode::ON, true}));

    MqMessage message;
    const iox::NodeName_t m_nodeName{"testNode"};
    message << mqMessageTypeToString(MqMessageType::CREATE_PUBLISHER) << "m_appName"
               << static_cast<cxx::Serialization>(iox::capro::ServiceDescription(99U, 1U, 20U)).toString() << std::to_string(13U)
               << m_nodeName << static_cast<cxx::Serialization>(iox::runtime::PortConfigInfo(11U, 22U, 33U)).toString();
    auto cmd = runtime::stringToMqMessageType(message.getElementAtIndex(0).c_str());
    std::string processName = message.getElementAtIndex(1);

    m_roudiApp->processMessageTest(message,cmd, ProcessName_t(cxx::TruncateToCapacity, processName));  

    m_roudiApp->shutDownTest();
}

/// @note name of the Testcase shall describe the test case in detail to avoid additional comments
TEST_F(RouDiBaseClass_test, ProcessMessageCreatePublisherError)
{
    std::unique_ptr<IceOryxRouDiComponents> m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(iox::RouDiConfig_t().setDefaults()));
    std::unique_ptr<RoudiClassTest> m_roudiApp = std::unique_ptr<RoudiClassTest>(new RoudiClassTest(m_roudiComponents->m_rouDiMemoryManager,
                                                  m_roudiComponents->m_portManager,
                                                  RouDi::RoudiStartupParameters{roudi::MonitoringMode::ON, true}));
    
    MqMessage message;
    message << mqMessageTypeToString(MqMessageType::CREATE_PUBLISHER) << "AppName" << "123123";
    auto cmd = runtime::stringToMqMessageType(message.getElementAtIndex(0).c_str());
    std::string processName = message.getElementAtIndex(1);

    m_roudiApp->processMessageTest(message,cmd, ProcessName_t(cxx::TruncateToCapacity, processName));

    m_roudiApp->shutDownTest();
}

/// @note name of the Testcase shall describe the test case in detail to avoid additional comments
TEST_F(RouDiBaseClass_test, ProcessMessageCreateSubscriber)
{
    std::unique_ptr<IceOryxRouDiComponents> m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(iox::RouDiConfig_t().setDefaults()));
    std::unique_ptr<RoudiClassTest> m_roudiApp = std::unique_ptr<RoudiClassTest>(new RoudiClassTest(m_roudiComponents->m_rouDiMemoryManager,
                                                  m_roudiComponents->m_portManager,
                                                  RouDi::RoudiStartupParameters{roudi::MonitoringMode::ON, true}));
    
    MqMessage message;
    message << mqMessageTypeToString(MqMessageType::CREATE_SUBSCRIBER) << "AppName" << "123123" << "456" << "789" << "123" << "abc";
    auto cmd = runtime::stringToMqMessageType(message.getElementAtIndex(0).c_str());
    std::string processName = message.getElementAtIndex(1);

    m_roudiApp->processMessageTest(message,cmd, ProcessName_t(cxx::TruncateToCapacity, processName));

    m_roudiApp->shutDownTest();
}

/// @note name of the Testcase shall describe the test case in detail to avoid additional comments
TEST_F(RouDiBaseClass_test, ProcessMessageCreateSubscriberError)
{
    std::unique_ptr<IceOryxRouDiComponents> m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(iox::RouDiConfig_t().setDefaults()));
    std::unique_ptr<RoudiClassTest> m_roudiApp = std::unique_ptr<RoudiClassTest>(new RoudiClassTest(m_roudiComponents->m_rouDiMemoryManager,
                                                  m_roudiComponents->m_portManager,
                                                  RouDi::RoudiStartupParameters{roudi::MonitoringMode::ON, true}));
    
    MqMessage message;
    message << mqMessageTypeToString(MqMessageType::CREATE_SUBSCRIBER) << "AppName" << "123123";
    auto cmd = runtime::stringToMqMessageType(message.getElementAtIndex(0).c_str());
    std::string processName = message.getElementAtIndex(1);

    m_roudiApp->processMessageTest(message,cmd, ProcessName_t(cxx::TruncateToCapacity, processName));

    m_roudiApp->shutDownTest();
}

/// @note name of the Testcase shall describe the test case in detail to avoid additional comments
TEST_F(RouDiBaseClass_test, ProcessMessageCreateConditionVariable)
{
    std::unique_ptr<IceOryxRouDiComponents> m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(iox::RouDiConfig_t().setDefaults()));
    std::unique_ptr<RoudiClassTest> m_roudiApp = std::unique_ptr<RoudiClassTest>(new RoudiClassTest(m_roudiComponents->m_rouDiMemoryManager,
                                                  m_roudiComponents->m_portManager,
                                                  RouDi::RoudiStartupParameters{roudi::MonitoringMode::ON, true}));
    
    MqMessage message;
    message << mqMessageTypeToString(MqMessageType::CREATE_CONDITION_VARIABLE) <<  "AppName";
    auto cmd = runtime::stringToMqMessageType(message.getElementAtIndex(0).c_str());
    std::string processName = message.getElementAtIndex(1);

    m_roudiApp->processMessageTest(message,cmd, ProcessName_t(cxx::TruncateToCapacity, processName));

    m_roudiApp->shutDownTest();
}

/// @note name of the Testcase shall describe the test case in detail to avoid additional comments
TEST_F(RouDiBaseClass_test, ProcessMessageCreateConditionVariableError)
{
    std::unique_ptr<IceOryxRouDiComponents> m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(iox::RouDiConfig_t().setDefaults()));
    std::unique_ptr<RoudiClassTest> m_roudiApp = std::unique_ptr<RoudiClassTest>(new RoudiClassTest(m_roudiComponents->m_rouDiMemoryManager,
                                                  m_roudiComponents->m_portManager,
                                                  RouDi::RoudiStartupParameters{roudi::MonitoringMode::ON, true}));
    
    MqMessage message;
    message << mqMessageTypeToString(MqMessageType::CREATE_CONDITION_VARIABLE) << "AppName" << "123123";
    auto cmd = runtime::stringToMqMessageType(message.getElementAtIndex(0).c_str());
    std::string processName = message.getElementAtIndex(1);

    m_roudiApp->processMessageTest(message,cmd, ProcessName_t(cxx::TruncateToCapacity, processName));

    m_roudiApp->shutDownTest();
}

/// @note name of the Testcase shall describe the test case in detail to avoid additional comments
TEST_F(RouDiBaseClass_test, ProcessMessageCreateInterface)
{
    std::unique_ptr<IceOryxRouDiComponents> m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(iox::RouDiConfig_t().setDefaults()));
    std::unique_ptr<RoudiClassTest> m_roudiApp = std::unique_ptr<RoudiClassTest>(new RoudiClassTest(m_roudiComponents->m_rouDiMemoryManager,
                                                  m_roudiComponents->m_portManager,
                                                  RouDi::RoudiStartupParameters{roudi::MonitoringMode::ON, true}));
    
    MqMessage message;
    message << mqMessageTypeToString(MqMessageType::CREATE_INTERFACE) << "AppName" << "123123" << "456";
    auto cmd = runtime::stringToMqMessageType(message.getElementAtIndex(0).c_str());
    std::string processName = message.getElementAtIndex(1);

    m_roudiApp->processMessageTest(message,cmd, ProcessName_t(cxx::TruncateToCapacity, processName));

    m_roudiApp->shutDownTest();
}

/// @note name of the Testcase shall describe the test case in detail to avoid additional comments
TEST_F(RouDiBaseClass_test, ProcessMessageCreateInterfaceError)
{
    std::unique_ptr<IceOryxRouDiComponents> m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(iox::RouDiConfig_t().setDefaults()));
    std::unique_ptr<RoudiClassTest> m_roudiApp = std::unique_ptr<RoudiClassTest>(new RoudiClassTest(m_roudiComponents->m_rouDiMemoryManager,
                                                  m_roudiComponents->m_portManager,
                                                  RouDi::RoudiStartupParameters{roudi::MonitoringMode::ON, true}));
    
    MqMessage message;
    message << mqMessageTypeToString(MqMessageType::CREATE_INTERFACE) << "AppName" << "123123";
    auto cmd = runtime::stringToMqMessageType(message.getElementAtIndex(0).c_str());
    std::string processName = message.getElementAtIndex(1);

    m_roudiApp->processMessageTest(message,cmd, ProcessName_t(cxx::TruncateToCapacity, processName));

    m_roudiApp->shutDownTest();
}

/// @note name of the Testcase shall describe the test case in detail to avoid additional comments
TEST_F(RouDiBaseClass_test, ProcessMessageCreateApplication)
{
    std::unique_ptr<IceOryxRouDiComponents> m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(iox::RouDiConfig_t().setDefaults()));
    std::unique_ptr<RoudiClassTest> m_roudiApp = std::unique_ptr<RoudiClassTest>(new RoudiClassTest(m_roudiComponents->m_rouDiMemoryManager,
                                                  m_roudiComponents->m_portManager,
                                                  RouDi::RoudiStartupParameters{roudi::MonitoringMode::ON, true}));
    
    MqMessage message;
    message << mqMessageTypeToString(MqMessageType::CREATE_APPLICATION) << "AppName";
    auto cmd = runtime::stringToMqMessageType(message.getElementAtIndex(0).c_str());
    std::string processName = message.getElementAtIndex(1);

    m_roudiApp->processMessageTest(message,cmd, ProcessName_t(cxx::TruncateToCapacity, processName));

    m_roudiApp->shutDownTest();
}

/// @note name of the Testcase shall describe the test case in detail to avoid additional comments
TEST_F(RouDiBaseClass_test, ProcessMessageCreateApplicationError)
{
    std::unique_ptr<IceOryxRouDiComponents> m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(iox::RouDiConfig_t().setDefaults()));
    std::unique_ptr<RoudiClassTest> m_roudiApp = std::unique_ptr<RoudiClassTest>(new RoudiClassTest(m_roudiComponents->m_rouDiMemoryManager,
                                                  m_roudiComponents->m_portManager,
                                                  RouDi::RoudiStartupParameters{roudi::MonitoringMode::ON, true}));
    
    MqMessage message;
    message << mqMessageTypeToString(MqMessageType::CREATE_APPLICATION) << "AppName" << "123123";
    auto cmd = runtime::stringToMqMessageType(message.getElementAtIndex(0).c_str());
    std::string processName = message.getElementAtIndex(1);

    m_roudiApp->processMessageTest(message,cmd, ProcessName_t(cxx::TruncateToCapacity, processName));

    m_roudiApp->shutDownTest();
}

/// @note name of the Testcase shall describe the test case in detail to avoid additional comments
TEST_F(RouDiBaseClass_test, ProcessMessageCreateNode)
{
    std::unique_ptr<IceOryxRouDiComponents> m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(iox::RouDiConfig_t().setDefaults()));
    std::unique_ptr<RoudiClassTest> m_roudiApp = std::unique_ptr<RoudiClassTest>(new RoudiClassTest(m_roudiComponents->m_rouDiMemoryManager,
                                                  m_roudiComponents->m_portManager,
                                                  RouDi::RoudiStartupParameters{roudi::MonitoringMode::ON, true}));
    
    MqMessage message;
    message << mqMessageTypeToString(MqMessageType::CREATE_NODE) << "AppName" << "123123" << "456";
    auto cmd = runtime::stringToMqMessageType(message.getElementAtIndex(0).c_str());
    std::string processName = message.getElementAtIndex(1);

    m_roudiApp->processMessageTest(message,cmd, ProcessName_t(cxx::TruncateToCapacity, processName));

    m_roudiApp->shutDownTest();
}

/// @note name of the Testcase shall describe the test case in detail to avoid additional comments
TEST_F(RouDiBaseClass_test, ProcessMessageCreateNodeError)
{
    std::unique_ptr<IceOryxRouDiComponents> m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(iox::RouDiConfig_t().setDefaults()));
    std::unique_ptr<RoudiClassTest> m_roudiApp = std::unique_ptr<RoudiClassTest>(new RoudiClassTest(m_roudiComponents->m_rouDiMemoryManager,
                                                  m_roudiComponents->m_portManager,
                                                  RouDi::RoudiStartupParameters{roudi::MonitoringMode::ON, true}));
    
    MqMessage message;
    message << mqMessageTypeToString(MqMessageType::CREATE_NODE) << "AppName" << "123123";
    auto cmd = runtime::stringToMqMessageType(message.getElementAtIndex(0).c_str());
    std::string processName = message.getElementAtIndex(1);

    m_roudiApp->processMessageTest(message,cmd, ProcessName_t(cxx::TruncateToCapacity, processName));

    m_roudiApp->shutDownTest();
}

/// @note name of the Testcase shall describe the test case in detail to avoid additional comments
TEST_F(RouDiBaseClass_test, ProcessMessageFindService)
{
    std::unique_ptr<IceOryxRouDiComponents> m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(iox::RouDiConfig_t().setDefaults()));
    std::unique_ptr<RoudiClassTest> m_roudiApp = std::unique_ptr<RoudiClassTest>(new RoudiClassTest(m_roudiComponents->m_rouDiMemoryManager,
                                                  m_roudiComponents->m_portManager,
                                                  RouDi::RoudiStartupParameters{roudi::MonitoringMode::ON, true}));
    
    MqMessage message;
    message << mqMessageTypeToString(MqMessageType::FIND_SERVICE) << "AppName" << "123123";
    auto cmd = runtime::stringToMqMessageType(message.getElementAtIndex(0).c_str());
    std::string processName = message.getElementAtIndex(1);

    m_roudiApp->processMessageTest(message,cmd, ProcessName_t(cxx::TruncateToCapacity, processName));

    m_roudiApp->shutDownTest();
}

/// @note name of the Testcase shall describe the test case in detail to avoid additional comments
TEST_F(RouDiBaseClass_test, ProcessMessageFindServiceError)
{
    std::unique_ptr<IceOryxRouDiComponents> m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(iox::RouDiConfig_t().setDefaults()));
    std::unique_ptr<RoudiClassTest> m_roudiApp = std::unique_ptr<RoudiClassTest>(new RoudiClassTest(m_roudiComponents->m_rouDiMemoryManager,
                                                  m_roudiComponents->m_portManager,
                                                  RouDi::RoudiStartupParameters{roudi::MonitoringMode::ON, true}));
    
    MqMessage message;
    message << mqMessageTypeToString(MqMessageType::FIND_SERVICE) << "AppName";
    auto cmd = runtime::stringToMqMessageType(message.getElementAtIndex(0).c_str());
    std::string processName = message.getElementAtIndex(1);

    m_roudiApp->processMessageTest(message,cmd, ProcessName_t(cxx::TruncateToCapacity, processName));

    m_roudiApp->shutDownTest();
}

/// @note name of the Testcase shall describe the test case in detail to avoid additional comments
TEST_F(RouDiBaseClass_test, ProcessMessageKeepAlive)
{
    std::unique_ptr<IceOryxRouDiComponents> m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(iox::RouDiConfig_t().setDefaults()));
    std::unique_ptr<RoudiClassTest> m_roudiApp = std::unique_ptr<RoudiClassTest>(new RoudiClassTest(m_roudiComponents->m_rouDiMemoryManager,
                                                  m_roudiComponents->m_portManager,
                                                  RouDi::RoudiStartupParameters{roudi::MonitoringMode::ON, true}));
    
    MqMessage message;
    message << mqMessageTypeToString(MqMessageType::KEEPALIVE) << "AppName" << "123123";
    auto cmd = runtime::stringToMqMessageType(message.getElementAtIndex(0).c_str());
    std::string processName = message.getElementAtIndex(1);

    m_roudiApp->processMessageTest(message,cmd, ProcessName_t(cxx::TruncateToCapacity, processName));

    m_roudiApp->shutDownTest();
}

/// @note name of the Testcase shall describe the test case in detail to avoid additional comments
TEST_F(RouDiBaseClass_test, ProcessMessageError)
{
    std::unique_ptr<IceOryxRouDiComponents> m_roudiComponents = std::unique_ptr<IceOryxRouDiComponents>(new IceOryxRouDiComponents(iox::RouDiConfig_t().setDefaults()));
    std::unique_ptr<RoudiClassTest> m_roudiApp = std::unique_ptr<RoudiClassTest>(new RoudiClassTest(m_roudiComponents->m_rouDiMemoryManager,
                                                  m_roudiComponents->m_portManager,
                                                  RouDi::RoudiStartupParameters{roudi::MonitoringMode::ON, true}));
    
    MqMessage message;
    message << mqMessageTypeToString(MqMessageType::ERROR) << "AppName" << "123123";
    auto cmd = runtime::stringToMqMessageType(message.getElementAtIndex(0).c_str());
    std::string processName = message.getElementAtIndex(1);

    m_roudiApp->processMessageTest(message,cmd, ProcessName_t(cxx::TruncateToCapacity, processName));

    m_roudiApp->shutDownTest();
}

} // namespace test
} // namespace iox
