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
#include "iceoryx_utils/internal/posix_wrapper/message_queue.hpp"

#include <chrono>

using namespace ::testing;
using namespace iox;
using namespace iox::posix;


class MessageQueue_test : public Test
{
  public:
    void SetUp()
    {
        auto mqResult =
            MessageQueue::create("/testQueue", MessageQueueMode::Blocking, MessageQueueOwnership::CreateNew);
        ASSERT_THAT(mqResult.has_error(), Eq(false));
        sut = std::move(mqResult.get_value());
        internal::CaptureStderr();
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

    MessageQueue sut;
};

TEST_F(MessageQueue_test, create)
{
    auto mq2 = MessageQueue::create("Silly name", MessageQueueMode::Blocking, MessageQueueOwnership::CreateNew);
    EXPECT_TRUE(mq2.has_error());
}

TEST_F(MessageQueue_test, sendAndReceive)
{
    std::string message = "Hey, I'm talking to you";
    bool sent = sut.send(message).has_error();
    EXPECT_FALSE(sent);

    std::string anotherMessage = "This is a message";
    sent = sut.send(anotherMessage).has_error();
    EXPECT_FALSE(sent);

    auto receivedMessage = sut.receive();
    ASSERT_THAT(receivedMessage.has_error(), Eq(false));
    EXPECT_EQ(message, *receivedMessage);

    receivedMessage = sut.receive();
    ASSERT_THAT(receivedMessage.has_error(), Eq(false));
    EXPECT_EQ(anotherMessage, *receivedMessage);
}

TEST_F(MessageQueue_test, sendAfterDestroy)
{
    sut.destroy();

    std::string message = "Should never be sent";
    bool sent = sut.send(message).has_error();
    EXPECT_TRUE(sent);
}

TEST_F(MessageQueue_test, receiveAfterDestroy)
{
    std::string message = "hello world!";
    bool sent = sut.send(message).has_error();
    EXPECT_FALSE(sent);

    sut.destroy();

    bool receivedMessage = sut.receive().has_error();
    EXPECT_THAT(receivedMessage, Eq(true));
}

TEST_F(MessageQueue_test, sendMoreThanAllowed)
{
    std::string shortMessage = "Iceoryx rules.";
    ASSERT_THAT(sut.send(shortMessage).has_error(), Eq(false));

    std::string longMessage(sut.MaxMsgSize + 8, 'x');
    ASSERT_THAT(sut.send(longMessage).has_error(), Eq(true));

    auto receivedMessage = sut.receive();
    ASSERT_THAT(receivedMessage.has_error(), Eq(false));
    EXPECT_EQ(shortMessage, receivedMessage.get_value());
}

TEST_F(MessageQueue_test, wildCreate)
{
    return;
    auto mqResult = MessageQueue::create();
    ASSERT_THAT(mqResult.has_error(), Eq(true));
    mqResult = MessageQueue::create(
        std::string("/blafu").c_str(), MessageQueueMode::Blocking, MessageQueueOwnership::CreateNew);
}

TEST_F(MessageQueue_test, timedSend)
{
    using namespace iox::units;
    using namespace std::chrono;

    std::string msg = "ISG rules.";
    Duration maxTimeout = 100_ms;
    Duration minTimeoutTolerance = 10_ms;
    Duration maxTimeoutTolerance = 20_ms;

    bool sent = false;

    // make sure message queue is full
    for (long i = 0; i < sut.MaxMsgNumber; ++i)
    {
        ASSERT_THAT(sut.timedSend(msg, maxTimeout).has_error(), Eq(false));
    }

    auto before = system_clock::now();
    auto result = sut.timedSend(msg, maxTimeout);
    ASSERT_THAT(result.has_error(), Eq(true));
    ASSERT_THAT(result.get_error(), Eq(MessageQueueError::Timeout));
    auto after = system_clock::now();

    EXPECT_FALSE(sent);

    // Do not exceed timeout
    auto timeDiff_ms = duration_cast<milliseconds>(after - before);
    EXPECT_LT(timeDiff_ms.count(), (maxTimeout + maxTimeoutTolerance).milliSeconds<int64_t>());

    // Check if timedSend has blocked for ~maxTimeout and has not returned immediately
    EXPECT_GT(timeDiff_ms.count(), (maxTimeout - minTimeoutTolerance).milliSeconds<int64_t>());
}

TEST_F(MessageQueue_test, timedReceive)
{
    using namespace iox::units;
    using namespace std::chrono;

    std::string msg = "very useful text for tranmission";
    Duration timeout = 100_ms;
    Duration minTimeoutTolerance = 10_ms;
    Duration maxTimeoutTolerance = 20_ms;

    sut.send(msg);

    auto received = sut.timedReceive(timeout);
    ASSERT_FALSE(received.has_error());

    EXPECT_EQ(received.get_value(), msg);

    auto before = system_clock::now();
    received = sut.timedReceive(timeout);
    auto after = system_clock::now();

    ASSERT_TRUE(received.has_error());

    // Do not exceed timeout
    auto timeDiff_ms = duration_cast<milliseconds>(after - before);
    EXPECT_LT(timeDiff_ms.count(), (timeout + maxTimeoutTolerance).milliSeconds<int64_t>());

    // Check if timedReceive has blocked for ~timeout and has not returned immediately
    EXPECT_GT(timeDiff_ms.count(), (timeout - minTimeoutTolerance).milliSeconds<int64_t>());
}
