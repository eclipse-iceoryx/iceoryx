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

#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "iceoryx_platform/types.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/roudi/process_manager.hpp"
#include "iceoryx_posh/internal/runtime/ipc_interface_creator.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/roudi/memory/iceoryx_roudi_memory_manager.hpp"
#include "iceoryx_posh/roudi/memory/roudi_memory_interface.hpp"
#include "iceoryx_posh/roudi_env/minimal_iceoryx_config.hpp"
#include "iceoryx_posh/version/compatibility_check_level.hpp"
#include "iox/posix_user.hpp"
#include "iox/string.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::roudi;
using namespace iox::popo;
using namespace iox::runtime;
using namespace iox::roudi_env;
using namespace iox::version;

class ProcessManager_test : public Test
{
  public:
    void SetUp() override
    {
        m_roudiMemoryManager = std::make_unique<IceOryxRouDiMemoryManager>(MinimalIceoryxConfigBuilder().create());
        EXPECT_FALSE(m_roudiMemoryManager->createAndAnnounceMemory().has_error());
        m_portManager = std::make_unique<PortManager>(m_roudiMemoryManager.get());
        CompatibilityCheckLevel m_compLevel{CompatibilityCheckLevel::OFF};
        m_sut = std::make_unique<ProcessManager>(*m_roudiMemoryManager, *m_portManager, DEFAULT_DOMAIN_ID, m_compLevel);
        m_sut->initIntrospection(&m_processIntrospection);
    }

    void TearDown() override
    {
    }

    const iox::RuntimeName_t m_processname{"TestProcess"};
    const uint32_t m_pid{42U};
    PosixUser m_user{PosixUser::getUserOfCurrentProcess().getName()};
    const bool m_isMonitored{true};
    VersionInfo m_versionInfo{42U, 42U, 42U, 42U, "Foo", "Bar"};

    IpcInterfaceCreator m_processIpcInterface{
        IpcInterfaceCreator::create(m_processname, DEFAULT_DOMAIN_ID, ResourceType::USER_DEFINED)
            .expect("This should never fail")};
    ProcessIntrospectionType m_processIntrospection;

    std::unique_ptr<IceOryxRouDiMemoryManager> m_roudiMemoryManager{nullptr};
    std::unique_ptr<PortManager> m_portManager{nullptr};
    std::unique_ptr<ProcessManager> m_sut{nullptr};
};


TEST_F(ProcessManager_test, RegisteredProcessCountIsInitiallyZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "54ec8fa2-e2dd-43a0-9e39-a9da967941de");

    EXPECT_THAT(m_sut->registeredProcessCount(), Eq(0));
}

TEST_F(ProcessManager_test, RegisterProcessWithMonitorningWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "57311fb6-f993-4011-bbe9-e42df5e54d5e");
    auto result = m_sut->registerProcess(m_processname, m_pid, m_user, m_isMonitored, 1U, 1U, m_versionInfo);

    EXPECT_TRUE(result);
    EXPECT_THAT(m_sut->registeredProcessCount(), Eq(1));
}

TEST_F(ProcessManager_test, RegisterProcessWithoutMonitoringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "ce0fcf0e-564c-4330-86c8-13b33c2a64c8");
    constexpr bool isNotMonitored{false};
    auto result = m_sut->registerProcess(m_processname, m_pid, m_user, isNotMonitored, 1U, 1U, m_versionInfo);

    EXPECT_TRUE(result);
}

TEST_F(ProcessManager_test, RegisterSameProcessTwiceWithMonitoringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "d449513c-2f8f-4b77-b419-8d1b5743f02d");
    auto result1 = m_sut->registerProcess(m_processname, m_pid, m_user, m_isMonitored, 1U, 1U, m_versionInfo);
    auto result2 = m_sut->registerProcess(m_processname, m_pid, m_user, m_isMonitored, 1U, 1U, m_versionInfo);

    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2);
    EXPECT_THAT(m_sut->registeredProcessCount(), Eq(1));
}

TEST_F(ProcessManager_test, RegisterSameProcessTwiceWithoutMonitoringWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "08d16887-72e5-4934-8447-a3b4760444e1");
    constexpr bool isNotMonitored{false};
    auto result1 = m_sut->registerProcess(m_processname, m_pid, m_user, isNotMonitored, 1U, 1U, m_versionInfo);
    auto result2 = m_sut->registerProcess(m_processname, m_pid, m_user, isNotMonitored, 1U, 1U, m_versionInfo);

    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2);
    EXPECT_THAT(m_sut->registeredProcessCount(), Eq(1));
}

TEST_F(ProcessManager_test, UnregisterNonExistentProcessLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "293cc3d1-727c-40ee-a298-3532a9e111a1");
    auto unregisterResult = m_sut->unregisterProcess(m_processname);

    EXPECT_FALSE(unregisterResult);
}

TEST_F(ProcessManager_test, RegisterAndUnregisterWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "335f1487-38ab-4526-9a83-a4b496139c34");
    m_sut->registerProcess(m_processname, m_pid, m_user, m_isMonitored, 1U, 1U, m_versionInfo);
    auto unregisterResult = m_sut->unregisterProcess(m_processname);

    EXPECT_TRUE(unregisterResult);
    EXPECT_THAT(m_sut->registeredProcessCount(), Eq(0));
}

TEST_F(ProcessManager_test, HandleProcessShutdownPreparationRequestWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "741669ec-111b-494b-b243-d28510b07782");
    m_sut->registerProcess(m_processname, m_pid, m_user, m_isMonitored, 1U, 1U, m_versionInfo);

    auto user = PosixUser::getUserOfCurrentProcess();
    auto payloadDataSegmentMemoryManager = m_roudiMemoryManager->segmentManager()
                                               .value()
                                               ->getSegmentInformationWithWriteAccessForUser(user)
                                               .m_memoryManager;

    ASSERT_TRUE(payloadDataSegmentMemoryManager.has_value());

    // get publisher and subscriber
    PublisherOptions publisherOptions{
        0U, iox::NodeName_t("node"), true, iox::popo::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER};
    PublisherPortUser publisher(m_portManager
                                    ->acquirePublisherPortData({"1", "1", "1"},
                                                               publisherOptions,
                                                               m_processname,
                                                               &payloadDataSegmentMemoryManager.value().get(),
                                                               PortConfigInfo())
                                    .value());

    ASSERT_TRUE(publisher.isOffered());

    m_sut->handleProcessShutdownPreparationRequest(m_processname);

    // we just check if handleProcessShutdownPreparationRequest calls PortManager::unblockProcessShutdown
    // ideally this should be checked by a mock, but since there isn't on for PortManager we just check the side effect
    ASSERT_FALSE(publisher.isOffered());
}

} // namespace
