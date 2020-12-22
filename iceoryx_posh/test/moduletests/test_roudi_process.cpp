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

#include "test.hpp"
#define private public
#define protected public
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/roudi/roudi_process.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/roudi/memory/roudi_memory_interface.hpp"
#include "iceoryx_posh/version/compatibility_check_level.hpp"
#include "iceoryx_utils/cxx/string.hpp"
#undef protected
#undef private

using namespace ::testing;
using namespace iox::roudi;
using namespace iox::popo;
using namespace iox::runtime;
using ::testing::Return;

/*class MqInterfaceUser_Mock : public iox::runtime::MqInterfaceUser
{
  public:
    MqInterfaceUser_Mock()
        : iox::runtime::MqInterfaceUser("MockTest", 5L, 512L)
    {
    }
    MOCK_METHOD1(send, bool(MqMessage));
};*/
class MqInterfaceUser_Mock : public iox::roudi::RouDiProcess
{
  public:
    MqInterfaceUser_Mock()
        : iox::roudi::RouDiProcess("TestRoudiProcess", 200, nullptr, true, 0x654321, 255)
    {
    }
    MOCK_METHOD1(sendToMQ, void(MqMessage));
};

class RouDiProcess_test : public Test
{
  public:
    const iox::ProcessName_t processname = "TestRoudiProcess";
    int32_t pid = 200;
    iox::mepoo::MemoryManager* payloadMemoryManager{nullptr};
    bool isMonitored = true;
    const uint64_t payloadSegmentId = 0x654321;
    const uint64_t sessionId = 255;
    MqInterfaceUser_Mock mqIntrfceusermock;
};

TEST_F(RouDiProcess_test, CTor)
{
    RouDiProcess roudiproc(processname, pid, payloadMemoryManager, isMonitored, payloadSegmentId, sessionId);
    EXPECT_THAT(roudiproc.getPid(), Eq(200));
}

TEST_F(RouDiProcess_test, getPid)
{
    RouDiProcess roudiproc(processname, pid, payloadMemoryManager, isMonitored, payloadSegmentId, sessionId);
    EXPECT_THAT(roudiproc.getPid(), Eq(200));
}

TEST_F(RouDiProcess_test, getName)
{
    RouDiProcess roudiproc(processname, pid, payloadMemoryManager, isMonitored, payloadSegmentId, sessionId);
    EXPECT_THAT(roudiproc.getName(), Eq(std::string("TestRoudiProcess")));
}

TEST_F(RouDiProcess_test, isMonitored)
{
    RouDiProcess roudiproc(processname, pid, payloadMemoryManager, isMonitored, payloadSegmentId, sessionId);
    EXPECT_THAT(roudiproc.isMonitored(), Eq(true));
}

TEST_F(RouDiProcess_test, getPayloadSegId)
{
    RouDiProcess roudiproc(processname, pid, payloadMemoryManager, isMonitored, payloadSegmentId, sessionId);
    EXPECT_THAT(roudiproc.getPayloadSegmentId(), Eq(0x654321));
}

TEST_F(RouDiProcess_test, getSessionId)
{
    RouDiProcess roudiproc(processname, pid, payloadMemoryManager, isMonitored, payloadSegmentId, sessionId);
    EXPECT_THAT(roudiproc.getSessionId(), Eq(255));
}

TEST_F(RouDiProcess_test, getPayloadMemoryManager)
{
    RouDiProcess roudiproc(processname, pid, payloadMemoryManager, isMonitored, payloadSegmentId, sessionId);
    EXPECT_THAT(roudiproc.getPayloadMemoryManager(), Eq(nullptr));
}

// Not able to test sendToMQ as there is no return value and mocking is not currently done

TEST_F(RouDiProcess_test, sendToMQ)
{
    iox::runtime::MqMessage data{"MESSAGE_NOT_SUPPORTED"};
    //    RouDiProcess roudiproc(processname, pid, payloadMemoryManager, isMonitored, payloadSegmentId, sessionId);
    EXPECT_CALL(mqIntrfceusermock, sendToMQ(_)).Times(1);
    mqIntrfceusermock.sendToMQ(data);
}

TEST_F(RouDiProcess_test, TimeStamp)
{
    auto timestmp = iox::mepoo::BaseClock_t::now();
    RouDiProcess roudiproc(processname, pid, payloadMemoryManager, isMonitored, payloadSegmentId, sessionId);
    roudiproc.setTimestamp(timestmp);
    EXPECT_THAT(roudiproc.getTimestamp(), Eq(timestmp));
}
