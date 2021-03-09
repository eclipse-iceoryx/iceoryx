// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
#include "iceoryx_posh/internal/roudi/process_manager.hpp"
#include "iceoryx_posh/internal/runtime/ipc_interface_creator.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/roudi/memory/iceoryx_roudi_memory_manager.hpp"
#include "iceoryx_posh/roudi/memory/roudi_memory_interface.hpp"
#include "iceoryx_posh/version/compatibility_check_level.hpp"
#include "iceoryx_utils/cxx/string.hpp"
#include "iceoryx_utils/platform/types.hpp"
#include "test.hpp"

using namespace ::testing;
using namespace iox::roudi;
using namespace iox::popo;
using namespace iox::runtime;
using namespace iox::posix;
using namespace iox::version;
using ::testing::Return;

class ProcessManager_test : public Test
{
  public:
    void SetUp() override
    {
        auto config = iox::RouDiConfig_t().setDefaults();
        m_roudiMemoryManager = new IceOryxRouDiMemoryManager(config);
        m_roudiMemoryManager->createAndAnnounceMemory();
        m_portManager = new PortManager(m_roudiMemoryManager);
        m_sut = new ProcessManager(*m_roudiMemoryManager, *m_portManager, m_compLevel);
        m_sut->initIntrospection(&m_processIntrospection);
    }

    void TearDown() override
    {
        delete m_roudiMemoryManager;
        delete m_portManager;
        delete m_sut;
    }

    const iox::ProcessName_t m_processname{"TestProcess"};
    pid_t m_pid{42U};
    PosixUser m_user{1U};
    bool m_isMonitored{true};
    VersionInfo m_versionInfo{42U, 42U, 42U, 42U, "Foo", "Bar"};
    CompatibilityCheckLevel m_compLevel;

    IpcInterfaceCreator m_processIpcInterface{m_processname};
    ProcessIntrospectionType m_processIntrospection;

    IceOryxRouDiMemoryManager* m_roudiMemoryManager{nullptr};
    PortManager* m_portManager{nullptr};
    ProcessManager* m_sut{nullptr};
};


TEST_F(ProcessManager_test, RegisterProcessWorks)
{
    auto result = m_sut->registerProcess(m_processname, m_pid, m_user, m_isMonitored, 1U, 1U, m_versionInfo);

    EXPECT_TRUE(result);
}

TEST_F(ProcessManager_test, RegisterProcessWithoutMonitoringWorks)
{
    bool isNotMonitored{false};
    auto result = m_sut->registerProcess(m_processname, m_pid, m_user, isNotMonitored, 1U, 1U, m_versionInfo);

    EXPECT_TRUE(result);
}

TEST_F(ProcessManager_test, RegisterSameProcessTwiceLeadsToError)
{
    auto result1 = m_sut->registerProcess(m_processname, m_pid, m_user, m_isMonitored, 1U, 1U, m_versionInfo);
    auto result2 = m_sut->registerProcess(m_processname, m_pid, m_user, m_isMonitored, 1U, 1U, m_versionInfo);

    EXPECT_TRUE(result1);
    EXPECT_FALSE(result2);
}

TEST_F(ProcessManager_test, RegisterSameProcessTwiceWithoutMonitoringLeadsToError)
{
    bool isNotMonitored{false};
    auto result1 = m_sut->registerProcess(m_processname, m_pid, m_user, isNotMonitored, 1U, 1U, m_versionInfo);
    auto result2 = m_sut->registerProcess(m_processname, m_pid, m_user, isNotMonitored, 1U, 1U, m_versionInfo);

    EXPECT_TRUE(result1);
    EXPECT_FALSE(result2);
}

TEST_F(ProcessManager_test, UnregisterNonExistentProcessLeadsToError)
{
    auto unregisterResult = m_sut->unregisterProcess(m_processname);

    EXPECT_FALSE(unregisterResult);
}

TEST_F(ProcessManager_test, RegisterAndUnregisterWorks)
{
    auto registerResult = m_sut->registerProcess(m_processname, m_pid, m_user, m_isMonitored, 1U, 1U, m_versionInfo);
    auto unregisterResult = m_sut->unregisterProcess(m_processname);

    EXPECT_TRUE(registerResult);
    EXPECT_TRUE(unregisterResult);
}
