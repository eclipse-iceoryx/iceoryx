// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
#include "test.hpp"

#include "iceoryx_posh/internal/runtime/ipc_message.hpp"
#include "iceoryx_posh/internal/runtime/ipc_runtime_interface.hpp"
#include "iox/atomic.hpp"
#include "iox/duration.hpp"
#include "iox/message_queue.hpp"
#include "iox/posix_call.hpp"
#include "iox/std_string_support.hpp"


#include <chrono>
#include <mutex>
#include <thread>

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::units;
using namespace iox::units::duration_literals;

using iox::runtime::InterfaceName_t;
using iox::runtime::IpcInterfaceBase;
using iox::runtime::IpcMessage;
using iox::runtime::IpcMessageType;
using iox::runtime::IpcRuntimeInterface;


#if !defined(__APPLE__)
constexpr char DeleteRouDiMessageQueue[] = "rm /dev/mqueue/roudi";
#endif

constexpr char MqAppName[] = "racer";

class StringToMessage : public IpcInterfaceBase
{
  public:
    using IpcInterfaceBase::setMessageFromString;
};

class CMqInterfaceStartupRace_test : public Test
{
  public:
    virtual void SetUp()
    {
        platform::IoxIpcChannelType::Builder_t()
            .name(m_roudiIpcChannelName)
            .channelSide(PosixIpcChannelSide::SERVER)
            .create()
            .and_then([this](auto& channel) { this->m_roudiQueue.emplace(std::move(channel)); });
        ASSERT_THAT(m_roudiQueue.has_value(), true);
    }
    virtual void TearDown()
    {
    }

    IpcMessage getIpcMessage(const std::string& request) const
    {
        IpcMessage msg;
        StringToMessage::setMessageFromString(request.c_str(), msg);
        return msg;
    }

    void checkRegRequest(const IpcMessage& msg) const
    {
        ASSERT_THAT(msg.getNumberOfElements(), Eq(6u));

        std::string cmd = msg.getElementAtIndex(0);
        ASSERT_THAT(cmd.c_str(), StrEq(IpcMessageTypeToString(IpcMessageType::REG)));

        std::string name = msg.getElementAtIndex(1);
        ASSERT_THAT(name.c_str(), StrEq(MqAppName));
    }

    void sendRegAck(const IpcMessage& oldMsg)
    {
        std::lock_guard<std::mutex> lock(m_appQueueMutex);
        IpcMessage regAck;
        constexpr uint32_t DUMMY_SHM_SIZE{37};
        constexpr uint32_t DUMMY_SHM_OFFSET{73};
        constexpr uint32_t DUMMY_SEGMENT_ID{13};
        constexpr uint32_t INDEX_OF_TIMESTAMP{4};
        constexpr iox::UntypedRelativePointer::offset_t OFFSET_ADDRESS_HEARTBEAT{
            iox::UntypedRelativePointer::NULL_POINTER_OFFSET};
        regAck << IpcMessageTypeToString(IpcMessageType::REG_ACK) << DUMMY_SHM_SIZE << DUMMY_SHM_OFFSET
               << oldMsg.getElementAtIndex(INDEX_OF_TIMESTAMP) << DUMMY_SEGMENT_ID << OFFSET_ADDRESS_HEARTBEAT;

        if (!m_appQueue.has_value())
        {
            platform::IoxIpcChannelType::Builder_t()
                .name(runtime::ipcChannelNameToInterfaceName(MqAppName, DEFAULT_DOMAIN_ID, ResourceType::USER_DEFINED))
                .channelSide(PosixIpcChannelSide::CLIENT)
                .create()
                .and_then([this](auto& channel) { this->m_appQueue.emplace(std::move(channel)); });
        }
        ASSERT_THAT(m_appQueue.has_value(), true);

        ASSERT_FALSE(m_appQueue->send(regAck.getMessage()).has_error());
    }

    /// @note smart_lock in combination with optional is currently not really usable
    std::mutex m_roudiQueueMutex;
    optional<platform::IoxIpcChannelType> m_roudiQueue;
    std::mutex m_appQueueMutex;
    optional<platform::IoxIpcChannelType> m_appQueue;
    InterfaceName_t m_roudiIpcChannelName{runtime::ipcChannelNameToInterfaceName(
        roudi::IPC_CHANNEL_ROUDI_NAME, DEFAULT_DOMAIN_ID, ResourceType::ICEORYX_DEFINED)};
};

#if !defined(__APPLE__)
TEST_F(CMqInterfaceStartupRace_test, ObsoleteRouDiMq)
{
    ::testing::Test::RecordProperty("TEST_ID", "a94080de-e07d-433b-be0d-6ca748006664");
    GTEST_SKIP() << "@todo iox-#1106 Test is not compatible on all platforms and needs to be refactored or removed";
    /// @note this test checks if the application handles the situation when the roudi mqueue was not properly cleaned
    /// up and tries to use the obsolet mqueue while RouDi gets restarted and cleans its resources up and creates a new
    /// mqueue

    iox::concurrent::Atomic<bool> shutdown;
    shutdown = false;
    auto roudi = std::thread([&] {
        std::lock_guard<std::mutex> lock(m_roudiQueueMutex);
        // ensure that the application already opened the roudi mqueue by waiting until a REG request is sent to the
        // roudi mqueue
        auto request = m_roudiQueue->timedReceive(15_s);
        ASSERT_FALSE(request.has_error());
        auto msg = getIpcMessage(request.value());
        checkRegRequest(msg);

        // simulate the restart of RouDi with the mqueue cleanup
        IOX_POSIX_CALL(system)
        (DeleteRouDiMessageQueue).failureReturnValue(-1).evaluate().or_else([](auto& r) {
            std::cerr << "system call failed with error: " << r.getHumanReadableErrnum();
            exit(EXIT_FAILURE);
        });

        auto m_roudiQueue2 = platform::IoxIpcChannelType::Builder_t()
                                 .name(m_roudiIpcChannelName)
                                 .channelSide(PosixIpcChannelSide::SERVER)
                                 .create();

        // check if the app retries to register at RouDi
        request = m_roudiQueue2->timedReceive(15_s);
        ASSERT_FALSE(request.has_error());
        msg = getIpcMessage(request.value());
        checkRegRequest(msg);

        sendRegAck(msg);

        while (!shutdown)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    auto dut = IpcRuntimeInterface::create(MqAppName, DEFAULT_DOMAIN_ID, 35_s)
                   .expect("Successfully created runtime interface");

    shutdown = true;
    roudi.join();
}

TEST_F(CMqInterfaceStartupRace_test, ObsoleteRouDiMqWithFullMq)
{
    ::testing::Test::RecordProperty("TEST_ID", "e7594a83-d0d1-49fb-8882-9d4dcc0372ef");
    GTEST_SKIP() << "@todo iox-#1106 Test is not compatible on all platforms and needs to be refactored or removed";
    /// @note this test checks if the application handles the situation when the roudi mqueue was not properly cleaned
    /// up and tries to use the obsolet mqueue while RouDi gets restarted and cleans its resources up and creates a new
    /// mqueue, the obsolete mqueue was filled up to the max message size, e.g. by the KEEP_ALIVE messages

    iox::concurrent::Atomic<bool> shutdown;
    shutdown = false;
    auto roudi = std::thread([&] {
        // fill the roudi mqueue
        std::lock_guard<std::mutex> lock(m_roudiQueueMutex);
        while (!m_roudiQueue->timedSend("dummy", 1_s).has_error())
        {
        }

        // wait some time for Runtime::GetInstance to send a REQ request with the full mqueue
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        // simulate the restart of RouDi with the mqueue cleanup
        IOX_POSIX_CALL(system)
        (DeleteRouDiMessageQueue).failureReturnValue(-1).evaluate().or_else([](auto& r) {
            std::cerr << "system call failed with error: " << r.getHumanReadableErrnum();
            exit(EXIT_FAILURE);
        });

        auto newRoudi = platform::IoxIpcChannelType::Builder_t()
                            .name(m_roudiIpcChannelName)
                            .channelSide(PosixIpcChannelSide::SERVER)
                            .create();

        // check if the app retries to register at RouDi
        auto request = newRoudi->timedReceive(15_s);
        if (request.has_error())
        {
            // clear the old mqueue to prevent a deadlock in mq_send to the old roudi mqueue in the app
            while (!m_roudiQueue->timedReceive(1_s).has_error())
            {
            }
        }
        ASSERT_FALSE(request.has_error());
        auto msg = getIpcMessage(request.value());
        checkRegRequest(msg);

        sendRegAck(msg);

        while (!shutdown)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    auto dut = IpcRuntimeInterface::create(MqAppName, DEFAULT_DOMAIN_ID, 35_s)
                   .expect("Successfully created runtime interface");

    shutdown = true;
    roudi.join();
}
#endif

TEST_F(CMqInterfaceStartupRace_test, ObsoleteRegAck)
{
    ::testing::Test::RecordProperty("TEST_ID", "16eb0dff-ef66-4943-b7a4-c0c0f079a0ae");
    /// @note this test checks if the application handles the situation when it sends an REG request to RouDi,
    /// terminates, gets restarted and sends a new REG request while RouDi has not yet processed the first REG request;
    /// this results in a message in the application mqueue which will be read with the next command and results in a
    /// wrong response

    iox::concurrent::Atomic<bool> shutdown;
    shutdown = false;
    auto roudi = std::thread([&] {
        std::lock_guard<std::mutex> lock(m_roudiQueueMutex);
        // wait for the REG request
        auto request = m_roudiQueue->timedReceive(5_s);
        ASSERT_FALSE(request.has_error());
        auto msg = getIpcMessage(request.value());
        checkRegRequest(msg);

        IpcMessage obsoleteMsg;
        for (uint32_t i = 0; i < 4; ++i)
        {
            obsoleteMsg << msg.getElementAtIndex(i);
        }
        // set an invalid timestamp
        obsoleteMsg << 0;
        sendRegAck(obsoleteMsg);
        sendRegAck(msg);

        while (!shutdown)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    auto dut = IpcRuntimeInterface::create(MqAppName, DEFAULT_DOMAIN_ID, 35_s)
                   .expect("Successfully created runtime interface");

    shutdown = true;
    roudi.join();

    std::lock_guard<std::mutex> lock(m_appQueueMutex);
    // the app IPC channel should be empty after registration
    auto response = m_appQueue->timedReceive(10_ms);
    EXPECT_THAT(response.has_error(), Eq(true));
}

} // namespace
