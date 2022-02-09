// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/internal/posix_wrapper/message_queue.hpp"
#include "iceoryx_hoofs/internal/posix_wrapper/unix_domain_socket.hpp"
#include "iceoryx_hoofs/posix_wrapper/named_pipe.hpp"

#include "test.hpp"

#include <chrono>

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::posix;
using namespace iox::units::duration_literals;

#if defined(__APPLE__)
using IpcChannelTypes = Types<UnixDomainSocket>;
#elif defined(_WIN32)
using IpcChannelTypes = Types<NamedPipe>;
#elif defined(unix) || defined(__unix) || defined(__unix__)
using IpcChannelTypes = Types<UnixDomainSocket, NamedPipe>;
#else
using IpcChannelTypes = Types<MessageQueue, UnixDomainSocket, NamedPipe>;
#endif

constexpr char goodName[] = "channel_test";
constexpr char anotherGoodName[] = "horst";
constexpr char theUnknown[] = "WhoeverYouAre";
constexpr char slashName[] = "/miau";

/// @req
/// @brief This test suite verifies that the abstract interface IpcChannelType is fulfilled by both the UnixDomainSocket
/// class and the MessageQueue class
/// @pre server and client object are allocated and move to the member object of the fixture
/// @post StdErr is capture and outputed to StdCout
/// @note Specific functionality of the underlying implementations of an IpcChannelType are tested in
/// "UnixDomainSocket_test"
template <typename T>
class IpcChannel_test : public Test
{
  public:
    using IpcChannelType = T;

    void SetUp()
    {
        IOX_DISCARD_RESULT(IpcChannelType::unlinkIfExists(goodName));

        auto serverResult = IpcChannelType::create(goodName, IpcChannelSide::SERVER, MaxMsgSize, MaxMsgNumber);
        ASSERT_THAT(serverResult.has_error(), Eq(false));
        server = std::move(serverResult.value());
        internal::CaptureStderr();

        auto clientResult = IpcChannelType::create(goodName, IpcChannelSide::CLIENT, MaxMsgSize, MaxMsgNumber);
        ASSERT_THAT(clientResult.has_error(), Eq(false));
        client = std::move(clientResult.value());
    }

    void TearDown()
    {
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }

    ~IpcChannel_test()
    {
    }

    static const size_t MaxMsgSize;
    static constexpr uint64_t MaxMsgNumber = 10U;
    IpcChannelType server;
    IpcChannelType client;
};

template <typename T>
const size_t IpcChannel_test<T>::MaxMsgSize = IpcChannelType::MAX_MESSAGE_SIZE;
template <typename T>
constexpr uint64_t IpcChannel_test<T>::MaxMsgNumber;

TYPED_TEST_SUITE(IpcChannel_test, IpcChannelTypes);


TYPED_TEST(IpcChannel_test, CreateWithTooLargeMessageSizeLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "c9e63950-e7f1-4efc-9d55-a5c445ee7dee");
    auto serverResult = TestFixture::IpcChannelType::create(
        goodName, IpcChannelSide::SERVER, TestFixture::MaxMsgSize + 1, TestFixture::MaxMsgNumber);
    EXPECT_TRUE(serverResult.has_error());
    ASSERT_THAT(serverResult.get_error(), Eq(IpcChannelError::MAX_MESSAGE_SIZE_EXCEEDED));
}

TYPED_TEST(IpcChannel_test, CreateNoNameLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "3ffe2cf2-26f4-4b93-8baf-d997dc71e610");
    auto serverResult = TestFixture::IpcChannelType::create("", IpcChannelSide::SERVER);
    EXPECT_TRUE(serverResult.has_error());
    ASSERT_THAT(serverResult.get_error(), Eq(IpcChannelError::INVALID_CHANNEL_NAME));
}

TYPED_TEST(IpcChannel_test, CreateWithLeadingSlashWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "89340ebd-f80d-480b-833f-da37dff06cef");
    auto serverResult = TestFixture::IpcChannelType::create(slashName, IpcChannelSide::SERVER);
    EXPECT_FALSE(serverResult.has_error());
}

TYPED_TEST(IpcChannel_test, CreateAgainWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "74f7e785-36fc-418c-9807-3dc95ae8aa91");
    // if there is a leftover from a crashed channel, we can create a new one. This is simulated by creating twice
    auto first = TestFixture::IpcChannelType::create(anotherGoodName, IpcChannelSide::SERVER);
    EXPECT_FALSE(first.has_error());
    auto second = TestFixture::IpcChannelType::create(anotherGoodName, IpcChannelSide::SERVER);
    EXPECT_FALSE(second.has_error());
}

TYPED_TEST(IpcChannel_test, CreateAgainAndEmptyWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "c6bf30dc-661d-47f9-8365-2cda5ca067f9");
    if (std::is_same<typename TestFixture::IpcChannelType, NamedPipe>::value)
    {
        // A NamedPipe server creates and destroys a pipe only when it was created
        // by itself. It is a normal use case that multiple instances can send
        // or receive concurrently via the same named pipe therefore the ctor of
        // the named pipe does not purge the underlying data.
        return;
    }

    using namespace iox::units;
    using namespace std::chrono;

    auto serverResult = TestFixture::IpcChannelType::create(anotherGoodName, IpcChannelSide::SERVER);
    EXPECT_FALSE(serverResult.has_error());
    auto server = std::move(serverResult.value());

    auto clientResult = TestFixture::IpcChannelType::create(anotherGoodName, IpcChannelSide::CLIENT);
    EXPECT_FALSE(clientResult.has_error());
    auto client = std::move(clientResult.value());

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

    auto second = TestFixture::IpcChannelType::create(anotherGoodName, IpcChannelSide::SERVER);
    EXPECT_FALSE(second.has_error());
    server = std::move(second.value());

    Duration timeout = 100_ms;
    auto received = server.timedReceive(timeout);
    ASSERT_TRUE(received.has_error());
    ASSERT_THAT(received.get_error(), Eq(IpcChannelError::TIMEOUT));
}

TYPED_TEST(IpcChannel_test, ClientWithoutServerLeadsToNoSuchChannelError)
{
    ::testing::Test::RecordProperty("TEST_ID", "6a0f75fc-cb6a-4593-a815-ea587c8e9d9e");
    auto clientResult = TestFixture::IpcChannelType::create(anotherGoodName, IpcChannelSide::CLIENT);
    EXPECT_TRUE(clientResult.has_error());
    ASSERT_THAT(clientResult.get_error(), Eq(IpcChannelError::NO_SUCH_CHANNEL));
}


TYPED_TEST(IpcChannel_test, NotDestroyingServerLeadsToNonOutdatedClient)
{
    ::testing::Test::RecordProperty("TEST_ID", "b3eef376-ae04-425b-aa5e-6b0ec4360253");
    auto serverResult = TestFixture::IpcChannelType::create(anotherGoodName, IpcChannelSide::SERVER);
    EXPECT_FALSE(serverResult.has_error());
    auto server = std::move(serverResult.value());

    auto clientResult = TestFixture::IpcChannelType::create(anotherGoodName, IpcChannelSide::CLIENT);
    EXPECT_FALSE(clientResult.has_error());
    auto client = std::move(clientResult.value());

    auto outdated = client.isOutdated();
    EXPECT_FALSE(outdated.has_error());
    EXPECT_FALSE(outdated.value());
}


TYPED_TEST(IpcChannel_test, DestroyingServerLeadsToOutdatedClient)
{
    ::testing::Test::RecordProperty("TEST_ID", "441c4480-57e7-4607-a7e1-df7a9f2f19d0");
    if (std::is_same<typename TestFixture::IpcChannelType, UnixDomainSocket>::value
        || std::is_same<typename TestFixture::IpcChannelType, NamedPipe>::value
        || std::is_same<typename TestFixture::IpcChannelType, MessageQueue>::value)
    {
        // isOutdated cannot be realized for unix domain sockets, named pipes or message queues
        return;
    }

    auto serverResult = TestFixture::IpcChannelType::create(anotherGoodName, IpcChannelSide::SERVER);
    EXPECT_FALSE(serverResult.has_error());
    auto server = std::move(serverResult.value());

    auto clientResult = TestFixture::IpcChannelType::create(anotherGoodName, IpcChannelSide::CLIENT);
    EXPECT_FALSE(clientResult.has_error());
    auto client = std::move(clientResult.value());

    // destroy the server and the client is outdated
    auto dest = server.destroy();
    ASSERT_FALSE(dest.has_error());

    auto outdated = client.isOutdated();
    EXPECT_FALSE(outdated.has_error());
    EXPECT_TRUE(outdated.value());
}

#if !defined(_WIN32)
// From:
// https://docs.microsoft.com/en-us/windows/win32/memory/sharing-files-and-memory
// The shared memory is not destroyed until every process called CloseHandle on
// that shared memory. If a process as an abnormal termination the kernel calls
// CloseHandle on every open handle, therefore shared memory remains should be
// impossible.
TYPED_TEST(IpcChannel_test, UnlinkExistingOneWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "ea22483b-4484-4d6d-b2b7-b574eaeb4b95");
    auto first = TestFixture::IpcChannelType::create(anotherGoodName, IpcChannelSide::SERVER);
    EXPECT_FALSE(first.has_error());
    auto ret = TestFixture::IpcChannelType::unlinkIfExists(anotherGoodName);
    ASSERT_FALSE(ret.has_error());
    EXPECT_TRUE(ret.value());
}

TYPED_TEST(IpcChannel_test, UnlinkNonExistingOneWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "c2ecbedc-be63-4f07-bceb-293d5a82c3cd");
    auto ret = TestFixture::IpcChannelType::unlinkIfExists(theUnknown);
    ASSERT_FALSE(ret.has_error());
    EXPECT_FALSE(ret.value());
}
#endif

TYPED_TEST(IpcChannel_test, SendAndReceiveWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "76a3df9f-736e-4803-af11-912927c352d4");
    std::string message = "Hey, I'm talking to you";
    bool sent = this->client.send(message).has_error();
    EXPECT_FALSE(sent);

    std::string anotherMessage = "This is a message";
    sent = this->client.send(anotherMessage).has_error();
    EXPECT_FALSE(sent);

    auto receivedMessage = this->server.receive();
    ASSERT_THAT(receivedMessage.has_error(), Eq(false));
    EXPECT_EQ(message, *receivedMessage);

    receivedMessage = this->server.receive();
    ASSERT_THAT(receivedMessage.has_error(), Eq(false));
    EXPECT_EQ(anotherMessage, *receivedMessage);
}

TYPED_TEST(IpcChannel_test, InvalidAfterDestroy)
{
    ::testing::Test::RecordProperty("TEST_ID", "4d878a64-95a4-4f15-8e09-f439c5dbd8eb");
    ASSERT_FALSE(this->client.destroy().has_error());
    ASSERT_FALSE(this->client.isInitialized());
    ASSERT_FALSE(this->server.destroy().has_error());
    ASSERT_FALSE(this->server.isInitialized());
}

TYPED_TEST(IpcChannel_test, SendAfterClientDestroyLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "dde79f4d-7bb5-4627-8d26-0bbc3dbf4d7d");
    auto dest = this->client.destroy();
    ASSERT_FALSE(dest.has_error());

    std::string message = "Should never be sent";
    bool sendError = this->client.send(message).has_error();
    EXPECT_TRUE(sendError);
}

TYPED_TEST(IpcChannel_test, SendAfterServerDestroyLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "95919ff0-ffe2-47e1-8a1d-0cb1e1df02df");
    if (std::is_same<typename TestFixture::IpcChannelType, MessageQueue>::value
        || std::is_same<typename TestFixture::IpcChannelType, NamedPipe>::value)
    {
        // NamedPipes are as long opened as long there is one instance
        // We still can send to the message queue is we destroy the server
        // it would be outdated, this is checked in another test
        return;
    }

    auto dest = this->server.destroy();
    ASSERT_FALSE(dest.has_error());

    std::string message = "Try to send me";
    auto sendResult = this->client.send(message);
    EXPECT_TRUE(sendResult.has_error());
}


TYPED_TEST(IpcChannel_test, ReceiveAfterServerDestroyLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "a704f003-097c-486b-92d3-4284f6b5abbe");
    std::string message = "hello world!";
    bool sendError = this->client.send(message).has_error();
    EXPECT_FALSE(sendError);

    auto dest = this->server.destroy();
    ASSERT_FALSE(dest.has_error());

    bool receiveError = this->server.receive().has_error();
    EXPECT_THAT(receiveError, Eq(true));
}

TYPED_TEST(IpcChannel_test, SendMoreThanAllowedLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "4e49c2fe-7278-4bc5-b91f-41777287e452");
    std::string shortMessage = "Iceoryx rules.";
    ASSERT_THAT(this->client.send(shortMessage).has_error(), Eq(false));

    std::string longMessage(this->server.MAX_MESSAGE_SIZE + 8, 'x');
    ASSERT_THAT(this->client.send(longMessage).has_error(), Eq(true));

    auto receivedMessage = this->server.receive();
    ASSERT_THAT(receivedMessage.has_error(), Eq(false));
    EXPECT_EQ(shortMessage, receivedMessage.value());
}

TYPED_TEST(IpcChannel_test, SendMaxMessageSizeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "f471df17-d49a-43cd-85a9-bbe457cfd012");
    std::string message(this->MaxMsgSize - 1, 'x');
    auto clientReturn = this->client.send(message);
    ASSERT_THAT(clientReturn.has_error(), Eq(false));

    auto receivedMessage = this->server.receive();
    ASSERT_THAT(receivedMessage.has_error(), Eq(false));
    EXPECT_EQ(message, receivedMessage.value());
}

TYPED_TEST(IpcChannel_test, wildCreate)
{
    ::testing::Test::RecordProperty("TEST_ID", "98d6003c-3f1a-4a8b-a524-ec1071bc9527");
    auto result = TestFixture::IpcChannelType::create();
    ASSERT_THAT(result.has_error(), Eq(true));
}

#if !(defined(__APPLE__) || defined(unix) || defined(__unix) || defined(__unix__))
TYPED_TEST(IpcChannel_test, TimedSendWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "12fe0ee5-37f8-4c34-ba44-ed50872a5fd9");
    using namespace iox::units;
    using namespace std::chrono;

    std::string msg = "ISG rules. And some more                                                                        "
                      "data to have a bit                                                                              "
                      "longer message";

    Duration maxTimeout = 100_ms;

    // send till it breaks
    for (;;)
    {
        auto before = system_clock::now();
        auto result = this->client.timedSend(msg, maxTimeout);
        auto after = system_clock::now();
        if (result.has_error())
        {
            ASSERT_THAT(result.get_error(), Eq(IpcChannelError::TIMEOUT));
            // Do not exceed timeout
            auto timeDiff = units::Duration(after - before);
            EXPECT_GT(timeDiff, maxTimeout);

            break;
        }
    }
}
#endif

TYPED_TEST(IpcChannel_test, TimedReceiveWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "978c8c4e-9829-4467-b664-45457104d4a9");
    using namespace iox::units;
    using namespace std::chrono;

    std::string msg = "very useful text for tranmission";
    Duration timeout = 100_ms;

    ASSERT_FALSE(this->client.send(msg).has_error());

    auto received = this->server.timedReceive(timeout);
    ASSERT_FALSE(received.has_error());

    EXPECT_EQ(received.value(), msg);

    auto before = system_clock::now();
    received = this->server.timedReceive(timeout);
    auto after = system_clock::now();

    ASSERT_TRUE(received.has_error());
    ASSERT_THAT(received.get_error(), Eq(IpcChannelError::TIMEOUT));

    auto timeDiff = units::Duration(after - before);
    EXPECT_GT(timeDiff, timeout);
}
} // namespace
