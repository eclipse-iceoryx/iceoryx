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

#include "iceoryx_platform/types.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iceoryx_posh/internal/roudi/process.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/roudi/memory/roudi_memory_interface.hpp"
#include "iceoryx_posh/version/compatibility_check_level.hpp"
#include "iox/string.hpp"

#include "iceoryx_hoofs/testing/error_reporting/testing_support.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::roudi;
using namespace iox::popo;
using namespace iox::runtime;

class IpcInterfaceUser_Mock : public iox::roudi::Process
{
  public:
    IpcInterfaceUser_Mock()
        : iox::roudi::Process(
            "TestProcess", DEFAULT_DOMAIN_ID, 200, PosixUser("foo"), HeartbeatPool::Index::INVALID, 255)
    {
    }
    MOCK_METHOD1(sendViaIpcChannel, void(IpcMessage));
    iox::mepoo::MemoryManager m_payloadDataSegmentMemoryManager;
};

class Process_test : public Test
{
  public:
    const iox::RuntimeName_t processname = {"TestProcess"};
    uint32_t pid{200U};
    PosixUser user{"foo"};
    bool isMonitored = true;
    HeartbeatPool heartbeatPool;
    HeartbeatPoolIndexType heartbeatPoolIndex{heartbeatPool.emplace().to_index()};
    const uint64_t dataSegmentId{0x654321U};
    const uint64_t sessionId{255U};
    IpcInterfaceUser_Mock ipcInterfaceUserMock;
};

TEST_F(Process_test, getPid)
{
    ::testing::Test::RecordProperty("TEST_ID", "fbe9ea27-9e23-4ec7-bfe6-e2563d42c5e7");
    Process roudiproc(processname, DEFAULT_DOMAIN_ID, pid, user, heartbeatPoolIndex, sessionId);
    EXPECT_THAT(roudiproc.getPid(), Eq(pid));
}

TEST_F(Process_test, getName)
{
    ::testing::Test::RecordProperty("TEST_ID", "c2f3df1d-0aa9-480e-8c2e-dd76960a7717");
    Process roudiproc(processname, DEFAULT_DOMAIN_ID, pid, user, heartbeatPoolIndex, sessionId);
    EXPECT_THAT(roudiproc.getName(), Eq(processname));
}

TEST_F(Process_test, isMonitored)
{
    ::testing::Test::RecordProperty("TEST_ID", "6d926282-c8f4-4b9c-a086-acc62e102c72");
    Process roudiproc(processname, DEFAULT_DOMAIN_ID, pid, user, heartbeatPoolIndex, sessionId);
    EXPECT_THAT(roudiproc.isMonitored(), Eq(isMonitored));
}

TEST_F(Process_test, getSessionId)
{
    ::testing::Test::RecordProperty("TEST_ID", "6986a49c-e23b-4cd6-ab63-269b32ff8d92");
    Process roudiproc(processname, DEFAULT_DOMAIN_ID, pid, user, heartbeatPoolIndex, sessionId);
    EXPECT_THAT(roudiproc.getSessionId(), Eq(sessionId));
}

TEST_F(Process_test, sendViaIpcChannelPass)
{
    ::testing::Test::RecordProperty("TEST_ID", "478cb320-7f4c-420c-a0d2-4a24e0db691c");
    iox::runtime::IpcMessage data{"MESSAGE_NOT_SUPPORTED"};
    EXPECT_CALL(ipcInterfaceUserMock, sendViaIpcChannel(_)).Times(1);
    ipcInterfaceUserMock.sendViaIpcChannel(data);
}
TEST_F(Process_test, sendViaIpcChannelFail)
{
    ::testing::Test::RecordProperty("TEST_ID", "c4d5c133-bf93-45a4-aa4f-9c3c2a50f91a");
    iox::runtime::IpcMessage data{""};

    Process roudiproc(processname, DEFAULT_DOMAIN_ID, pid, user, heartbeatPoolIndex, sessionId);
    roudiproc.sendViaIpcChannel(data);

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::POSH__ROUDI_PROCESS_SEND_VIA_IPC_CHANNEL_FAILED);
}

TEST_F(Process_test, Heartbeat)
{
    ::testing::Test::RecordProperty("TEST_ID", "5b527de2-699e-4d35-86ee-10ed28498e88");
    Process roudiproc(processname, DEFAULT_DOMAIN_ID, pid, user, heartbeatPoolIndex, sessionId);
    EXPECT_THAT(roudiproc.getHeartbeatPoolIndex(), Eq(heartbeatPoolIndex));
}

} // namespace
