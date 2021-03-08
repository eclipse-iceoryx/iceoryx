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
#include "test.hpp"

using namespace ::testing;
using namespace iox::roudi;
using namespace iox::popo;
using namespace iox::runtime;
using ::testing::Return;

class IpcInterfaceUser_Mock : public iox::roudi::Process
{
  public:
    IpcInterfaceUser_Mock()
        : iox::roudi::Process("TestProcess", 200, nullptr, true, 0x654321, 255)
    {
    }
    MOCK_METHOD1(sendViaIpcChannel, void(IpcMessage));
};

class ProcessManager_test : public Test
{
  public:
    const iox::ProcessName_t processname = {"TestProcess"};
    pid_t pid{200U};
    iox::mepoo::MemoryManager* payloadMemoryManager{nullptr};
    bool isMonitored = true;
    const uint64_t payloadSegmentId{0x654321U};
    const uint64_t sessionId{255U};
    IpcInterfaceUser_Mock ipcInterfaceUserMock;
};

TEST_F(ProcessManager_test, foo)
{
}
