// Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
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
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/roudi/memory/roudi_memory_interface.hpp"
#include "iceoryx_posh/version/compatibility_check_level.hpp"
#include "iceoryx_utils/cxx/string.hpp"
#include "iceoryx_utils/platform/types.hpp"
#include "mocks/roudi_memory_interface_mock.hpp"
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
    const iox::ProcessName_t m_processname = {"TestProcess"};
    pid_t m_pid{42U};
    PosixUser m_user{73};
    iox::mepoo::MemoryManager* m_payloadMemoryManager{nullptr};
    bool m_isMonitored{true};
    const uint64_t m_payloadSegmentId{0x654321U};
    const uint64_t m_sessionId{255U};
    RouDiMemoryInterfaceMock m_memoryInterfaceMock;
    PortManager m_portManager{&m_memoryInterfaceMock};
    VersionInfo m_versionInfo{42, 42, 42, 42, "Foo", "Bar"};
    CompatibilityCheckLevel m_compLevel;
    ProcessManager m_sut{m_memoryInterfaceMock, m_portManager, m_compLevel};
};

TEST_F(ProcessManager_test, RegisterProcessWithMonitoringWorks)
{
    auto result = m_sut.registerProcess(m_processname, m_pid, m_user, true, 1U, 1U, m_versionInfo);

    EXPECT_TRUE(result);
}

TEST_F(ProcessManager_test, RegisterProcessWithoutMonitoringWorks)
{
    auto result = m_sut.registerProcess(m_processname, m_pid, m_user, false, 1U, 1U, m_versionInfo);

    EXPECT_TRUE(result);
}
