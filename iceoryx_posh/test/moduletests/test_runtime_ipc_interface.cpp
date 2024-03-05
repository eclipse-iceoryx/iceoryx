// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_platform/platform_settings.hpp"
#include "iceoryx_posh/internal/runtime/ipc_interface_base.hpp"
#include "iox/message_queue.hpp"
#include "iox/named_pipe.hpp"
#include "iox/std_chrono_support.hpp"
#include "iox/std_string_support.hpp"
#include "iox/unix_domain_socket.hpp"

#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "test.hpp"

#include <chrono>

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::testing;
using namespace iox::units::duration_literals;

#if defined(__APPLE__)
using IpcChannelTypes = Types<runtime::IpcInterface<UnixDomainSocket>>;
#elif defined(_WIN32)
using IpcChannelTypes = Types<runtime::IpcInterface<NamedPipe>>;
#elif defined(__linux) || defined(__gnu_linux__) || defined(linux) || defined(__linux__)
using IpcChannelTypes = Types<runtime::IpcInterface<UnixDomainSocket>,
                              runtime::IpcInterface<NamedPipe>,
                              runtime::IpcInterface<MessageQueue>>;
#elif defined(unix) || defined(__unix) || defined(__unix__)
using IpcChannelTypes = Types<runtime::IpcInterface<UnixDomainSocket>, runtime::IpcInterface<NamedPipe>>;
#else
using IpcChannelTypes = Types<runtime::IpcInterface<UnixDomainSocket>,
                              runtime::IpcInterface<NamedPipe>,
                              runtime::IpcInterface<MessageQueue>>;
#endif

constexpr char goodName[] = "channel_test";
constexpr char anotherGoodName[] = "horst";
constexpr char slashName[] = "/miau";

/// @brief This test suite verifies that the abstract interface IpcChannelType is fulfilled by the UnixDomainSocket,
/// NamedPipe and the MessageQueue
template <typename T>
class IpcInterface_test : public Test
{
  public:
    using IpcChannelType = T;

    class SutType : public IpcChannelType
    {
      public:
        SutType(const RuntimeName_t& runtimeName,
                const uint64_t maxMessages = MaxMsgNumber,
                const uint64_t messageSize = MaxMsgSize) noexcept
            : IpcChannelType(runtimeName, DEFAULT_DOMAIN_ID, ResourceType::USER_DEFINED, maxMessages, messageSize)
        {
        }
        using IpcChannelType::ipcChannelMapsToFile;
        using IpcChannelType::openIpcChannel;
    };


    void SetUp()
    {
        server.emplace(goodName);
        ASSERT_TRUE(server->openIpcChannel(PosixIpcChannelSide::SERVER));

        client.emplace(goodName);
        ASSERT_TRUE(client->openIpcChannel(PosixIpcChannelSide::CLIENT));
    }

    void TearDown()
    {
    }

    ~IpcInterface_test()
    {
    }

    optional<SutType> server;
    optional<SutType> client;
    static const size_t MaxMsgSize;
    static constexpr uint64_t MaxMsgNumber = 10U;
};

template <typename T>
const size_t IpcInterface_test<T>::MaxMsgSize = IpcChannelType::MAX_MESSAGE_SIZE;
template <typename T>
constexpr uint64_t IpcInterface_test<T>::MaxMsgNumber;

TYPED_TEST_SUITE(IpcInterface_test, IpcChannelTypes, );


TYPED_TEST(IpcInterface_test, CreateWithTooLargeMessageSizeWillBeClampedToMaxMessageSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "c9e63950-e7f1-4efc-9d55-a5c445ee7dee");
    typename TestFixture::SutType sut(goodName, TestFixture::MaxMsgNumber, TestFixture::MaxMsgSize + 1);
    EXPECT_TRUE(sut.openIpcChannel(PosixIpcChannelSide::SERVER));
    EXPECT_TRUE(sut.isInitialized());
}

TYPED_TEST(IpcInterface_test, CreateWithNoNameFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "3ffe2cf2-26f4-4b93-8baf-d997dc71e610");

    IOX_EXPECT_FATAL_FAILURE([] { typename TestFixture::SutType sut(""); }, iox::er::FATAL);
}

TYPED_TEST(IpcInterface_test, CreateWithLeadingSlashFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "89340ebd-f80d-480b-833f-da37dff06cef");

    IOX_EXPECT_FATAL_FAILURE([] { typename TestFixture::SutType sut(slashName); }, iox::er::FATAL);
}

TYPED_TEST(IpcInterface_test, CreateAgainWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "74f7e785-36fc-418c-9807-3dc95ae8aa91");
    // if there is a leftover from a crashed channel, we can create a new one. This is simulated by creating twice
    typename TestFixture::SutType first(anotherGoodName);
    EXPECT_TRUE(first.openIpcChannel(PosixIpcChannelSide::SERVER));
    EXPECT_TRUE(first.isInitialized());

    typename TestFixture::SutType second(anotherGoodName);
    EXPECT_TRUE(second.openIpcChannel(PosixIpcChannelSide::SERVER));
    EXPECT_TRUE(second.isInitialized());
}

TYPED_TEST(IpcInterface_test, CreateAgainAndEmptyWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "c6bf30dc-661d-47f9-8365-2cda5ca067f9");
    if (std::is_same<typename TestFixture::IpcChannelType, runtime::IpcInterface<NamedPipe>>::value)
    {
        // A NamedPipe server creates and destroys a pipe only when it was created
        // by itself. It is a normal use case that multiple instances can send
        // or receive concurrently via the same named pipe therefore the ctor of
        // the named pipe does not purge the underlying data.
        return;
    }

    using namespace iox::units;
    using namespace std::chrono;

    optional<typename TestFixture::SutType> server;
    server.emplace(anotherGoodName);
    ASSERT_TRUE(server->openIpcChannel(PosixIpcChannelSide::SERVER));

    typename TestFixture::SutType client(anotherGoodName);
    ASSERT_TRUE(client.openIpcChannel(PosixIpcChannelSide::CLIENT));

    // send and receive as usual
    runtime::IpcMessage message;
    message << "Hello " << 5 << true;
    ASSERT_TRUE(client.send(message));

    runtime::IpcMessage receivedMessage;
    ASSERT_TRUE(server->receive(receivedMessage));
    EXPECT_THAT(message, Eq(receivedMessage));

    // send a message, create the server again and there is no message
    runtime::IpcMessage newMessage;
    newMessage << "I'm still talking to you! " << 12.01f << "blubb";
    ASSERT_TRUE(client.send(newMessage));

    server.emplace(anotherGoodName);
    ASSERT_TRUE(server->openIpcChannel(PosixIpcChannelSide::SERVER));

    Duration timeout = 100_ms;
    ASSERT_FALSE(server->timedReceive(timeout, receivedMessage));
}

TYPED_TEST(IpcInterface_test, ClientWithoutServerCannotOpenIpcChannel)
{
    ::testing::Test::RecordProperty("TEST_ID", "6a0f75fc-cb6a-4593-a815-ea587c8e9d9e");
    if (std::is_same<typename TestFixture::IpcChannelType, runtime::IpcInterface<MessageQueue>>::value)
    {
        // A message queue can always open a message queue since it is persistent!
        return;
    }

    typename TestFixture::SutType client(anotherGoodName);
    ASSERT_FALSE(client.openIpcChannel(PosixIpcChannelSide::CLIENT));
}

TYPED_TEST(IpcInterface_test, NotDestroyingServerLeadsToAChannelMappedToFile)
{
    ::testing::Test::RecordProperty("TEST_ID", "b3eef376-ae04-425b-aa5e-6b0ec4360253");

    typename TestFixture::SutType server(anotherGoodName);
    EXPECT_TRUE(server.openIpcChannel(PosixIpcChannelSide::SERVER));

    typename TestFixture::SutType client(anotherGoodName);
    EXPECT_TRUE(client.openIpcChannel(PosixIpcChannelSide::CLIENT));

    EXPECT_TRUE(client.ipcChannelMapsToFile());
}

TYPED_TEST(IpcInterface_test, SendAndReceiveWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "76a3df9f-736e-4803-af11-912927c352d4");

    runtime::IpcMessage message;
    message << "Hey"
            << "I'm"
            << "talking"
            << "to"
            << "you";
    ASSERT_TRUE(this->client->send(message));

    runtime::IpcMessage anotherMessage;
    anotherMessage << "This"
                   << "is"
                   << "a"
                   << "message";
    ASSERT_TRUE(this->client->send(anotherMessage));

    runtime::IpcMessage receivedMessage;
    ASSERT_TRUE(this->server->receive(receivedMessage));
    EXPECT_EQ(message, receivedMessage);

    ASSERT_TRUE(this->server->receive(receivedMessage));
    EXPECT_EQ(anotherMessage, receivedMessage);
}

TYPED_TEST(IpcInterface_test, SendAfterServerDestroyLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "95919ff0-ffe2-47e1-8a1d-0cb1e1df02df");
    if (std::is_same<typename TestFixture::IpcChannelType, runtime::IpcInterface<MessageQueue>>::value
        || std::is_same<typename TestFixture::IpcChannelType, runtime::IpcInterface<NamedPipe>>::value)
    {
        // NamedPipes are valid as long as there is at least one instance with the handle to it
        //   opened. The underlying shared memory is removed only when the last instance has called
        //   close(namedPipeHandle) [persistent].
        //
        // MessageQueues are persistent as well. When we destroy a server and another instance has
        // still opened the message queue it is still valid and data can be written into it. To make this
        // detectable isOutdated was introduced for the message queue and is tested here as well in another test.
        return;
    }
    this->server.reset();

    runtime::IpcMessage message;
    message << "Try"
            << "to"
            << "send"
            << "me";
    EXPECT_FALSE(this->client->send(message));
}

TYPED_TEST(IpcInterface_test, SendMoreThanAllowedLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "4e49c2fe-7278-4bc5-b91f-41777287e452");
    runtime::IpcMessage shortMessage;
    shortMessage << "Iceoryx rules.";
    ASSERT_TRUE(this->client->send(shortMessage));

    runtime::IpcMessage longMessage;
    longMessage << std::string(this->server->MAX_MESSAGE_SIZE + 8, 'x');
    ASSERT_FALSE(this->client->send(longMessage));

    runtime::IpcMessage receivedMessage;
    ASSERT_TRUE(this->server->receive(receivedMessage));
    EXPECT_EQ(shortMessage, receivedMessage);
}

/// @todo iox-#1623 Check if blocking send is supported on all platforms
#if !(defined(__APPLE__) || defined(unix) || defined(__unix) || defined(__unix__))
TYPED_TEST(IpcInterface_test, TimedSendWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "12fe0ee5-37f8-4c34-ba44-ed50872a5fd9");
    using namespace iox::units;
    using namespace std::chrono;

    runtime::IpcMessage msg;
    msg << "ISG rules. And some more                                                                        "
           "data to have a bit longer message                                                               ";

    Duration maxTimeout = 100_ms;

    // send till it breaks
    for (;;)
    {
        auto before = system_clock::now();
        auto result = this->client->timedSend(msg, maxTimeout);
        auto after = system_clock::now();
        if (!result)
        {
            // Do not exceed timeout
            auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(after - before);
            EXPECT_GE(into<units::Duration>(timeDiff), maxTimeout);

            break;
        }
    }
}
#endif

TYPED_TEST(IpcInterface_test, TimedReceiveWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "978c8c4e-9829-4467-b664-45457104d4a9");
    using namespace iox::units;
    using namespace std::chrono;

    runtime::IpcMessage msg;
    msg << "very useful text for tranmission";
    Duration timeout = 100_ms;

    ASSERT_TRUE(this->client->send(msg));

    runtime::IpcMessage receivedMessage;
    ASSERT_TRUE(this->server->timedReceive(timeout, receivedMessage));

    EXPECT_EQ(receivedMessage, msg);

    auto before = system_clock::now();
    ASSERT_FALSE(this->server->timedReceive(timeout, receivedMessage));
    auto after = system_clock::now();

    auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(after - before);
    EXPECT_GE(into<units::Duration>(timeDiff), timeout);
}
} // namespace
