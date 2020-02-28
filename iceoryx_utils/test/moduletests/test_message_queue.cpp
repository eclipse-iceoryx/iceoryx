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

#include "iceoryx_utils/internal/posix_wrapper/message_queue.hpp"
#include "iceoryx_utils/internal/posix_wrapper/unix_domain_socket.hpp"

#include "test.hpp"

#include <chrono>

using namespace ::testing;
using namespace iox;
using namespace iox::posix;

using IpcChannel = MessageQueue;


class MessageQueue_test : public Test
{
  public:
    void SetUp()
    {
        auto serverResult = IpcChannel::create(
            "/channel_test", IpcChannelMode::BLOCKING, IpcChannelSide::SERVER, MaxMsgSize, MaxMsgNumber);
        ASSERT_THAT(serverResult.has_error(), Eq(false));
        server = std::move(serverResult.get_value());
        internal::CaptureStderr();

        auto clientResult = IpcChannel::create("/channel_test", IpcChannelMode::BLOCKING, IpcChannelSide::CLIENT);
        ASSERT_THAT(clientResult.has_error(), Eq(false));
        client = std::move(clientResult.get_value());
    }

    void TearDown()
    {
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }

    ~MessageQueue_test()
    {
    }

    static constexpr size_t MaxMsgSize = 512;
    static constexpr uint64_t MaxMsgNumber = 10;
    IpcChannel server;
    IpcChannel client;
};

constexpr size_t MessageQueue_test::MaxMsgSize;
constexpr uint64_t MessageQueue_test::MaxMsgNumber;

TEST_F(MessageQueue_test, create)
{
    auto mq2 = IpcChannel::create("Silly name", IpcChannelMode::BLOCKING, IpcChannelSide::SERVER);
    EXPECT_TRUE(mq2.has_error());
}

TEST_F(MessageQueue_test, sendAndReceive)
{
    std::string message = "Hey, I'm talking to you";
    bool sent = client.send(message).has_error();
    EXPECT_FALSE(sent);

    std::string anotherMessage = "This is a message";
    sent = client.send(anotherMessage).has_error();
    EXPECT_FALSE(sent);

    auto receivedMessage = server.receive();
    ASSERT_THAT(receivedMessage.has_error(), Eq(false));
    EXPECT_EQ(message, *receivedMessage);

    receivedMessage = server.receive();
    ASSERT_THAT(receivedMessage.has_error(), Eq(false));
    EXPECT_EQ(anotherMessage, *receivedMessage);
}

TEST_F(MessageQueue_test, sendAfterDestroy)
{
    client.destroy();

    std::string message = "Should never be sent";
    bool sendError = client.send(message).has_error();
    EXPECT_TRUE(sendError);
}

TEST_F(MessageQueue_test, receiveAfterDestroy)
{
    std::string message = "hello world!";
    bool sendError = client.send(message).has_error();
    EXPECT_FALSE(sendError);

    server.destroy();

    bool receiveError = server.receive().has_error();
    EXPECT_THAT(receiveError, Eq(true));
}

TEST_F(MessageQueue_test, sendMoreThanAllowed)
{
    std::string shortMessage = "Iceoryx rules.";
    ASSERT_THAT(client.send(shortMessage).has_error(), Eq(false));

    std::string longMessage(server.MAX_MESSAGE_SIZE + 8, 'x');
    ASSERT_THAT(client.send(longMessage).has_error(), Eq(true));

    auto receivedMessage = server.receive();
    ASSERT_THAT(receivedMessage.has_error(), Eq(false));
    EXPECT_EQ(shortMessage, receivedMessage.get_value());
}

TEST_F(MessageQueue_test, wildCreate)
{
    return;
    auto result = IpcChannel::create();
    ASSERT_THAT(result.has_error(), Eq(true));
    result = IpcChannel::create(std::string("/blafu").c_str(), IpcChannelMode::BLOCKING, IpcChannelSide::SERVER);
}

TEST_F(MessageQueue_test, timedSend)
{
    using namespace iox::units;
    using namespace std::chrono;

    std::string msg = "ISG rules. And some more                                                                        "
                      "data to have a bit                                                                              "
                      "longer message";

    Duration maxTimeout = 100_ms;
    Duration minTimeoutTolerance = 10_ms;
    Duration maxTimeoutTolerance = 20_ms;

    // send till it breaks
    for (;;)
    {
        auto before = system_clock::now();
        auto result = client.timedSend(msg, maxTimeout);
        auto after = system_clock::now();
        if (result.has_error())
        {
            ASSERT_THAT(result.get_error(), Eq(IpcChannelError::TIMEOUT));
            // Do not exceed timeout
            auto timeDiff_ms = duration_cast<milliseconds>(after - before);
            EXPECT_LT(timeDiff_ms.count(), (maxTimeout + maxTimeoutTolerance).milliSeconds<int64_t>());

            // Check if timedSend has blocked for ~maxTimeout and has not returned immediately
            EXPECT_GT(timeDiff_ms.count(), (maxTimeout - minTimeoutTolerance).milliSeconds<int64_t>());

            break;
        }
    }
}

TEST_F(MessageQueue_test, timedReceive)
{
    using namespace iox::units;
    using namespace std::chrono;

    std::string msg = "very useful text for tranmission";
    Duration timeout = 100_ms;
    Duration minTimeoutTolerance = 10_ms;
    Duration maxTimeoutTolerance = 20_ms;

    client.send(msg);

    auto received = server.timedReceive(timeout);
    ASSERT_FALSE(received.has_error());

    EXPECT_EQ(received.get_value(), msg);

    auto before = system_clock::now();
    received = server.timedReceive(timeout);
    auto after = system_clock::now();

    ASSERT_TRUE(received.has_error());
    ASSERT_THAT(received.get_error(), Eq(IpcChannelError::TIMEOUT));

    // Do not exceed timeout
    auto timeDiff_ms = duration_cast<milliseconds>(after - before);
    EXPECT_LT(timeDiff_ms.count(), (timeout + maxTimeoutTolerance).milliSeconds<int64_t>());

    // Check if timedReceive has blocked for ~timeout and has not returned immediately
    EXPECT_GT(timeDiff_ms.count(), (timeout - minTimeoutTolerance).milliSeconds<int64_t>());
}
