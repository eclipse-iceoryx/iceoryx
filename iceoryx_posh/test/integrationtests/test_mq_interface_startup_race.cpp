// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/runtime/message_queue_interface.hpp"
#include "iceoryx_posh/internal/runtime/message_queue_message.hpp"
#include "iceoryx_utils/internal/posix_wrapper/message_queue.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"

#include <chrono>
#include <mutex>
#include <thread>

using namespace ::testing;
using namespace iox;
using namespace iox::units;
using namespace iox::posix;

using iox::runtime::MqBase;
using iox::runtime::MqMessage;
using iox::runtime::MqMessageType;
using iox::runtime::MqRuntimeInterface;

using MQueue = iox::posix::MessageQueue;

constexpr char MqRouDiName[] = "/roudi";
constexpr char MqAppName[] = "/racer";

constexpr char DeleteRouDiMessageQueue[] = "rm /dev/mqueue/roudi";

class StringToMessage : public MqBase
{
  public:
    using MqBase::setMessageFromString;
};

class CMqInterfaceStartupRace_test : public Test
{
  public:
    CMqInterfaceStartupRace_test()
        : m_appQueue{MQueue::create()}
    {
    }

    virtual void SetUp()
    {
        ASSERT_THAT(m_roudiQueue.has_error(), false);
    }
    virtual void TearDown()
    {
    }

    MqMessage getMqMessage(const std::string& request) const
    {
        MqMessage msg;
        StringToMessage::setMessageFromString(request.c_str(), msg);
        return msg;
    }

    void checkRegRequest(const MqMessage& msg) const
    {
        ASSERT_THAT(msg.getNumberOfElements(), Eq(5u));

        std::string cmd = msg.getElementAtIndex(0);
        ASSERT_THAT(cmd.c_str(), StrEq(mqMessageTypeToString(MqMessageType::REG)));

        std::string name = msg.getElementAtIndex(1);
        ASSERT_THAT(name.c_str(), StrEq(MqAppName));
    }

    void sendRegAck(const MqMessage& oldMsg)
    {
        std::lock_guard<std::mutex> lock(m_appQueueMutex);
        MqMessage regAck;
        constexpr uint32_t DUMMY_SHM_ADDRESS{42};
        constexpr uint32_t DUMMY_SHM_SIZE{37};
        constexpr uint32_t DUMMY_SHM_OFFSET{73};
        constexpr uint32_t DUMMY_SEGMENT_ID{13};
        constexpr uint32_t INDEX_OF_TIMESTAMP{4};
        regAck << mqMessageTypeToString(MqMessageType::REG_ACK) << DUMMY_SHM_ADDRESS << DUMMY_SHM_SIZE
               << DUMMY_SHM_OFFSET << oldMsg.getElementAtIndex(INDEX_OF_TIMESTAMP) << DUMMY_SEGMENT_ID;

        if (m_appQueue.has_error())
        {
            m_appQueue = MQueue::create(MqAppName, MessageQueueMode::Blocking, MessageQueueOwnership::CreateNew);
        }
        ASSERT_THAT(m_appQueue.has_error(), false);

        m_appQueue->send(regAck.getMessage());
    }

    /// @note smart_lock in combination with optional is currently not really usable
    std::mutex m_roudiQueueMutex;
    MQueue::result_t m_roudiQueue{
        MQueue::create(MqRouDiName, MessageQueueMode::Blocking, MessageQueueOwnership::CreateNew)};
    std::mutex m_appQueueMutex;
    MQueue::result_t m_appQueue;
};

TEST_F(CMqInterfaceStartupRace_test, DISABLED_ObsoleteRouDiMq_PERFORMANCETEST42)
{
    /// @note this test checks if the application handles the situation when the roudi mqueue was not properly cleaned
    /// up and tries to use the obsolet mqueue while RouDi gets restarted and cleans its resources up and creates a new
    /// mqueue

    auto roudi = std::thread([&] {
        std::lock_guard<std::mutex> lock(m_roudiQueueMutex);
        // ensure that the application already opened the roudi mqueue by waiting until a REG request is sent to the
        // roudi mqueue
        auto request = m_roudiQueue->timedReceive(15_s);
        ASSERT_FALSE(request.has_error());
        auto msg = getMqMessage(request.get_value());
        checkRegRequest(msg);

        // simulate the restart of RouDi with the mqueue cleanup
        system(DeleteRouDiMessageQueue);
        auto m_roudiQueue2 = MQueue::create(MqRouDiName, MessageQueueMode::Blocking, MessageQueueOwnership::CreateNew);

        // check if the app retries to register at RouDi
        request = m_roudiQueue2->timedReceive(15_s);
        ASSERT_FALSE(request.has_error());
        msg = getMqMessage(request.get_value());
        checkRegRequest(msg);

        sendRegAck(msg);
    });

    MqRuntimeInterface dut(MqRouDiName, MqAppName, 35_s);

    roudi.join();
}

TEST_F(CMqInterfaceStartupRace_test, ObsoleteRouDiMqWithFullMq)
{
    /// @note this test checks if the application handles the situation when the roudi mqueue was not properly cleaned
    /// up and tries to use the obsolet mqueue while RouDi gets restarted and cleans its resources up and creates a new
    /// mqueue, the obsolete mqueue was filled up to the max message size, e.g. by the KEEP_ALIVE messages

    auto roudi = std::thread([&] {
        // fill the roudi mqueue
        std::lock_guard<std::mutex> lock(m_roudiQueueMutex);
        while (!m_roudiQueue->timedSend("dummy", 10_ms).has_error())
        {
        }

        // wait some time for Runtime::GetInstance to send a REQ request with the full mqueue
        std::this_thread::sleep_for(std::chrono::milliseconds(666));

        // simulate the restart of RouDi with the mqueue cleanup
        system(DeleteRouDiMessageQueue);
        auto newRoudi = MQueue::create(MqRouDiName, MessageQueueMode::Blocking, MessageQueueOwnership::CreateNew);

        // check if the app retries to register at RouDi
        auto request = newRoudi->timedReceive(5_s);
        if (request.has_error())
        {
            // clear the old mqueue to prevent a deadlock in mq_send to the old roudi mqueue in the app
            while (!m_roudiQueue->timedReceive(10_ms).has_error())
            {
            }
        }
        ASSERT_FALSE(request.has_error());
        auto msg = getMqMessage(request.get_value());
        checkRegRequest(msg);

        sendRegAck(msg);
    });

    MqRuntimeInterface dut(MqRouDiName, MqAppName, 10000_ms);

    roudi.join();
}

TEST_F(CMqInterfaceStartupRace_test, ObsoleteRegAck)
{
    /// @note this test checks if the application handles the situation when it sends an REG request to RouDi,
    /// terminates, gets restarted and sends a new REG request while RouDi has not yet processed the first REG request;
    /// this results in a message in the application mqueue which will be read with the next command and results in a
    /// wrong response

    auto roudi = std::thread([&] {
        std::lock_guard<std::mutex> lock(m_roudiQueueMutex);
        // wait for the REG request
        auto request = m_roudiQueue->timedReceive(5_s);
        ASSERT_FALSE(request.has_error());
        auto msg = getMqMessage(request.get_value());
        checkRegRequest(msg);

        MqMessage obsoleteMsg;
        for (int i = 0; i < 4; ++i)
        {
            obsoleteMsg << msg.getElementAtIndex(i);
        }
        // set an invalid timestamp
        obsoleteMsg << 0;
        sendRegAck(obsoleteMsg);
        sendRegAck(msg);
    });

    MqRuntimeInterface dut(MqRouDiName, MqAppName, 10000_ms);

    roudi.join();

    std::lock_guard<std::mutex> lock(m_appQueueMutex);
    // the app message queue should be empty after registration
    auto response = m_appQueue->timedReceive(10_ms);
    EXPECT_THAT(response.has_error(), Eq(true));
}
