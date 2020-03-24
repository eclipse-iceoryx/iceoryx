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

#if !defined(_WIN32) && !defined(__APPLE__)
#include "iceoryx_utils/internal/posix_wrapper/message_queue.hpp"
#include "iceoryx_utils/internal/posix_wrapper/unix_domain_socket.hpp"

#include "test.hpp"

#include <chrono>

using namespace ::testing;
using namespace iox;
using namespace iox::posix;

using IpcChannel = MessageQueue;

constexpr char goodName[] = "/channel_test";
constexpr char anotherGoodName[] = "/horst";
constexpr char theUnknown[] = "/WhoeverYouAre";
constexpr char badName[] = "skdhnskähug";

class MessageQueue_test : public Test
{
  public:
    void SetUp()
    {
        auto serverResult =
            IpcChannel::create(goodName, IpcChannelMode::BLOCKING, IpcChannelSide::SERVER, MaxMsgSize, MaxMsgNumber);
        ASSERT_THAT(serverResult.has_error(), Eq(false));
        server = std::move(serverResult.get_value());
        internal::CaptureStderr();

        auto clientResult =
            IpcChannel::create(goodName, IpcChannelMode::BLOCKING, IpcChannelSide::CLIENT, MaxMsgSize, MaxMsgNumber);
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

    static constexpr size_t MaxMsgSize = IpcChannel::MAX_MESSAGE_SIZE;
    static constexpr uint64_t MaxMsgNumber = 10u;
    IpcChannel server;
    IpcChannel client;
};

constexpr size_t MessageQueue_test::MaxMsgSize;
constexpr uint64_t MessageQueue_test::MaxMsgNumber;

TEST_F(MessageQueue_test, createNoName)
{
    auto mq2 = IpcChannel::create("", IpcChannelMode::BLOCKING, IpcChannelSide::SERVER);
    EXPECT_TRUE(mq2.has_error());
    ASSERT_THAT(mq2.get_error(), Eq(IpcChannelError::INVALID_CHANNEL_NAME));
}

TEST_F(MessageQueue_test, createBadName)
{
    auto mq2 = IpcChannel::create(badName, IpcChannelMode::BLOCKING, IpcChannelSide::SERVER);
    EXPECT_TRUE(mq2.has_error());
}

TEST_F(MessageQueue_test, createAgain)
{
    // if there is a leftover from a crashed channel, we can create a new one. This is simulated by creating twice
    auto first = IpcChannel::create(anotherGoodName, IpcChannelMode::BLOCKING, IpcChannelSide::SERVER);
    EXPECT_FALSE(first.has_error());
    auto second = IpcChannel::create(anotherGoodName, IpcChannelMode::BLOCKING, IpcChannelSide::SERVER);
    EXPECT_FALSE(second.has_error());
}


TEST_F(MessageQueue_test, createAgainAndEmpty)
{
    using namespace iox::units;
    using namespace std::chrono;

    auto serverResult = IpcChannel::create(anotherGoodName, IpcChannelMode::BLOCKING, IpcChannelSide::SERVER);
    EXPECT_FALSE(serverResult.has_error());
    auto server = std::move(serverResult.get_value());

    auto clientResult = IpcChannel::create(anotherGoodName, IpcChannelMode::BLOCKING, IpcChannelSide::CLIENT);
    EXPECT_FALSE(clientResult.has_error());
    auto client = std::move(clientResult.get_value());

    // send and receive as usual
    std::string message = "Hey, I'm talking to you";
    bool sent = client.send(message).has_error();
    EXPECT_FALSE(sent);

    auto receivedMessage = server.receive();
    ASSERT_THAT(receivedMessage.has_error(), Eq(false));
    EXPECT_EQ(message, *receivedMessage);

    // send a message, create the server again and there is no message
    std::string newMessage = "I'm still talking to you brother";
    sent = client.send(newMessage).has_error();
    EXPECT_FALSE(sent);

    auto second = IpcChannel::create(anotherGoodName, IpcChannelMode::BLOCKING, IpcChannelSide::SERVER);
    EXPECT_FALSE(second.has_error());
    server = std::move(second.get_value());

    Duration timeout = 100_ms;
    auto received = server.timedReceive(timeout);
    ASSERT_TRUE(received.has_error());
    ASSERT_THAT(received.get_error(), Eq(IpcChannelError::TIMEOUT));
}

TEST_F(MessageQueue_test, clientWithoutServerFails)
{
    auto clientResult = IpcChannel::create(anotherGoodName, IpcChannelMode::BLOCKING, IpcChannelSide::CLIENT);
    EXPECT_TRUE(clientResult.has_error());
    ASSERT_THAT(clientResult.get_error(), Eq(IpcChannelError::NO_SUCH_CHANNEL));
}


TEST_F(MessageQueue_test, NotOutdatedOne)
{
    auto serverResult = IpcChannel::create(anotherGoodName, IpcChannelMode::BLOCKING, IpcChannelSide::SERVER);
    EXPECT_FALSE(serverResult.has_error());
    auto server = std::move(serverResult.get_value());

    auto clientResult = IpcChannel::create(anotherGoodName, IpcChannelMode::BLOCKING, IpcChannelSide::CLIENT);
    EXPECT_FALSE(clientResult.has_error());
    auto client = std::move(clientResult.get_value());

    auto outdated = client.isOutdated();
    EXPECT_FALSE(outdated.has_error());
    EXPECT_FALSE(outdated.get_value());
}


TEST_F(MessageQueue_test, OutdatedOne)
{
    if (std::is_same<IpcChannel, UnixDomainSocket>::value)
    {
        // isOutdated cannot be realized for unix domain sockets
        return;
    }

    auto serverResult = IpcChannel::create(anotherGoodName, IpcChannelMode::BLOCKING, IpcChannelSide::SERVER);
    EXPECT_FALSE(serverResult.has_error());
    auto server = std::move(serverResult.get_value());

    auto clientResult = IpcChannel::create(anotherGoodName, IpcChannelMode::BLOCKING, IpcChannelSide::CLIENT);
    EXPECT_FALSE(clientResult.has_error());
    auto client = std::move(clientResult.get_value());

    // destroy the server and the client is outdated
    auto dest = server.destroy();
    ASSERT_FALSE(dest.has_error());

    auto outdated = client.isOutdated();
    EXPECT_FALSE(outdated.has_error());
    EXPECT_TRUE(outdated.get_value());
}

TEST_F(MessageQueue_test, unlinkExistingOne)
{
    auto first = IpcChannel::create(anotherGoodName, IpcChannelMode::BLOCKING, IpcChannelSide::SERVER);
    EXPECT_FALSE(first.has_error());
    auto ret = IpcChannel::unlinkIfExists(anotherGoodName);
    EXPECT_FALSE(ret.has_error());
    EXPECT_TRUE(ret.get_value());
}

TEST_F(MessageQueue_test, unlinkNonExistingOne)
{
    auto ret = IpcChannel::unlinkIfExists(theUnknown);
    EXPECT_FALSE(ret.has_error());
    EXPECT_FALSE(ret.get_value());
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

TEST_F(MessageQueue_test, sendAfterClientDestroy)
{
    auto dest = client.destroy();
    ASSERT_FALSE(dest.has_error());

    std::string message = "Should never be sent";
    bool sendError = client.send(message).has_error();
    EXPECT_TRUE(sendError);
}

TEST_F(MessageQueue_test, sendAfterServerDestroy)
{
    if (std::is_same<IpcChannel, MessageQueue>::value)
    {
        // We still can send to the message queue is we destroy the server
        // it would be outdated, this is checked in another test
        return;
    }

    auto dest = server.destroy();
    ASSERT_FALSE(dest.has_error());

    std::string message = "Try to send me";
    auto sendResult = client.send(message);
    EXPECT_TRUE(sendResult.has_error());
    EXPECT_THAT(sendResult.get_error(), Eq(IpcChannelError::NO_SUCH_CHANNEL));
}


TEST_F(MessageQueue_test, receiveAfterServerDestroy)
{
    std::string message = "hello world!";
    bool sendError = client.send(message).has_error();
    EXPECT_FALSE(sendError);

    auto dest = server.destroy();
    ASSERT_FALSE(dest.has_error());

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

TEST_F(MessageQueue_test, sendMaxMessageSize)
{
    std::string message(MaxMsgSize - 1, 'x');
    auto clientReturn = client.send(message);
    ASSERT_THAT(clientReturn.has_error(), Eq(false));

    auto receivedMessage = server.receive();
    ASSERT_THAT(receivedMessage.has_error(), Eq(false));
    EXPECT_EQ(message, receivedMessage.get_value());
}

TEST_F(MessageQueue_test, wildCreate)
{
    auto result = IpcChannel::create();
    ASSERT_THAT(result.has_error(), Eq(true));
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
#endif
