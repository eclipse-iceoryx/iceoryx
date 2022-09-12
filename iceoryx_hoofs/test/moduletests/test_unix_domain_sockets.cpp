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

#if !defined(_WIN32)
#include "iceoryx_hoofs/internal/posix_wrapper/unix_domain_socket.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"
#include "iceoryx_hoofs/testing/timing_test.hpp"
#include "iceoryx_platform/socket.hpp"

#include "test.hpp"

#include <atomic>
#include <chrono>
#include <thread>

namespace
{
using namespace ::testing;
using namespace ::testing::internal;
using namespace iox;
using namespace iox::posix;
using namespace iox::units::duration_literals;
using namespace iox::units;

using sendCall_t = std::function<cxx::expected<IpcChannelError>(const std::string&)>;
using receiveCall_t = std::function<cxx::expected<std::string, IpcChannelError>()>;

// NOLINTJUSTIFICATION used only for test purposes
// NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
constexpr char goodName[] = "channel_test";

/// @req
/// @brief This test suite verifies the functionality which is specific to the UnixDomainSocket class
/// @pre server and client object are allocated and move to the member object of the fixture
/// @post StdErr is capture and outputed to StdCout
/// @note Most of the UnixDomainSocket functionality is tested in "IpcChannel_test"
class UnixDomainSocket_test : public Test
{
  public:
    void SetUp() override
    {
        auto serverResult = UnixDomainSocket::create(
            goodName, IpcChannelSide::SERVER, UnixDomainSocket::MAX_MESSAGE_SIZE, MaxMsgNumber);
        ASSERT_THAT(serverResult.has_error(), Eq(false));
        server = std::move(serverResult.value());
        CaptureStderr();

        auto clientResult = UnixDomainSocket::create(
            goodName, IpcChannelSide::CLIENT, UnixDomainSocket::MAX_MESSAGE_SIZE, MaxMsgNumber);
        ASSERT_THAT(clientResult.has_error(), Eq(false));
        client = std::move(clientResult.value());
    }

    void TearDown() override
    {
        std::string output = GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }

    static bool createTestSocket(const UnixDomainSocket::UdsName_t& name)
    {
        bool socketCreationSuccess = true;
        static constexpr int32_t ERROR_CODE = -1;
        // NOLINTJUSTIFICATION initialized in next line with memset
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
        struct sockaddr_un sockAddr;

        memset(&sockAddr, 0, sizeof(sockAddr));
        sockAddr.sun_family = AF_LOCAL;
        strncpy(&(sockAddr.sun_path[0]), name.c_str(), name.size());

        iox::posix::posixCall(iox_socket)(AF_LOCAL, SOCK_DGRAM, 0)
            .failureReturnValue(ERROR_CODE)
            .evaluate()
            .and_then([&](auto& r) {
                iox::posix::posixCall(iox_bind)(r.value,
                                                // NOLINTJUSTIFICATION enforced by POSIX API
                                                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                                                reinterpret_cast<struct sockaddr*>(&sockAddr),
                                                sizeof(sockAddr))
                    .failureReturnValue(ERROR_CODE)
                    .evaluate()
                    .or_else([&](auto&) {
                        std::cerr << "unable to bind socket\n";
                        socketCreationSuccess = false;
                    });
            })
            .or_else([&](auto&) {
                std::cerr << "unable to create socket\n";
                socketCreationSuccess = false;
            });
        return socketCreationSuccess;
    }

    void signalThreadReady()
    {
        doWaitForThread.store(false, std::memory_order_relaxed);
    }

    void waitForThread() const
    {
        while (doWaitForThread.load(std::memory_order_relaxed))
        {
            std::this_thread::yield();
        }
    }

    const std::chrono::milliseconds WAIT_IN_MS{10};
    std::atomic_bool doWaitForThread{true};
    static constexpr uint64_t MaxMsgNumber = 10U;
    UnixDomainSocket server;
    UnixDomainSocket client;
};

constexpr uint64_t UnixDomainSocket_test::MaxMsgNumber;

TEST_F(UnixDomainSocket_test, UnlinkEmptySocketNameLeadsToInvalidChannelNameError)
{
    ::testing::Test::RecordProperty("TEST_ID", "bdc1e253-2750-4b07-a528-83ca50246b29");
    auto ret = UnixDomainSocket::unlinkIfExists(UnixDomainSocket::NoPathPrefix, "");
    ASSERT_TRUE(ret.has_error());
    EXPECT_THAT(ret.get_error(), Eq(IpcChannelError::INVALID_CHANNEL_NAME));
}

TEST_F(UnixDomainSocket_test, UnlinkEmptySocketNameWithPathPrefixLeadsToInvalidChannelNameError)
{
    ::testing::Test::RecordProperty("TEST_ID", "97793649-ac88-4e73-a0bc-602dca302746");
    auto ret = UnixDomainSocket::unlinkIfExists("");
    ASSERT_TRUE(ret.has_error());
    EXPECT_THAT(ret.get_error(), Eq(IpcChannelError::INVALID_CHANNEL_NAME));
}

TEST_F(UnixDomainSocket_test, UnlinkTooLongSocketNameWithPathPrefixLeadsToInvalidChannelNameError)
{
    ::testing::Test::RecordProperty("TEST_ID", "2fae48fb-8247-4119-a0ec-c40dda87e0c7");
    UnixDomainSocket::UdsName_t longSocketName;
    for (uint64_t i = 0U;
         i < UnixDomainSocket::LONGEST_VALID_NAME - strlen(&platform::IOX_UDS_SOCKET_PATH_PREFIX[0]) + 1;
         ++i)
    {
        longSocketName.append(cxx::TruncateToCapacity, "o");
    }
    auto ret = UnixDomainSocket::unlinkIfExists(longSocketName);
    ASSERT_TRUE(ret.has_error());
    EXPECT_THAT(ret.get_error(), Eq(IpcChannelError::INVALID_CHANNEL_NAME));
}

TEST_F(UnixDomainSocket_test, UnlinkExistingSocketIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "292879cd-89b5-4ebe-8459-f71d13a7befe");
    UnixDomainSocket::UdsName_t socketFileName = platform::IOX_UDS_SOCKET_PATH_PREFIX;
    socketFileName.append(cxx::TruncateToCapacity, "iceoryx-hoofs-moduletest.socket");
    ASSERT_TRUE(createTestSocket(socketFileName));
    auto ret = UnixDomainSocket::unlinkIfExists(UnixDomainSocket::NoPathPrefix, socketFileName);
    EXPECT_FALSE(ret.has_error());
}

TEST_F(UnixDomainSocket_test, UnlinkExistingSocketWithPathPrefixLeadsIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "33019857-7a2c-4aed-92b1-4218332a254c");
    UnixDomainSocket::UdsName_t socketFileName = "iceoryx-hoofs-moduletest.socket";
    UnixDomainSocket::UdsName_t socketFileNameWithPrefix = platform::IOX_UDS_SOCKET_PATH_PREFIX;
    socketFileNameWithPrefix.append(cxx::TruncateToCapacity, socketFileName);
    ASSERT_TRUE(createTestSocket(socketFileNameWithPrefix));
    auto ret = UnixDomainSocket::unlinkIfExists(socketFileName);
    EXPECT_FALSE(ret.has_error());
}

// the current contract of the unix domain socket is that a server can only receive
// and the client can only send
void sendOnServerLeadsToError(const sendCall_t& send)
{
    cxx::string<10> message{"Foo"};
    auto result = send(message);
    EXPECT_TRUE(result.has_error());
    EXPECT_THAT(result.get_error(), Eq(IpcChannelError::INTERNAL_LOGIC_ERROR));
}

TEST_F(UnixDomainSocket_test, TimedSendOnServerLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "d2a4986a-afe7-49bc-b870-d1baf069aad2");
    sendOnServerLeadsToError([&](auto& msg) { return server.timedSend(msg, 1_ms); });
}

TEST_F(UnixDomainSocket_test, SendOnServerLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "82721639-8514-410f-b761-54c9f519a6e4");
    sendOnServerLeadsToError([&](auto& msg) { return server.send(msg); });
}

void successfulSendAndReceive(const std::vector<std::string>& messages,
                              const sendCall_t& send,
                              const receiveCall_t& receive)
{
    for (const auto& m : messages)
    {
        ASSERT_FALSE(send(m).has_error());
    }

    for (const auto& sentMessage : messages)
    {
        auto receivedMessage = receive();
        ASSERT_FALSE(receivedMessage.has_error());
        EXPECT_EQ(*receivedMessage, sentMessage);
    }
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfNonEmptyMessageWithSendAndReceive)
{
    ::testing::Test::RecordProperty("TEST_ID", "69a2f9f4-2a4a-48e2-aa50-72b00e657f1d");
    successfulSendAndReceive(
        {"what's hypnotoads eye color?"},
        [&](auto& msg) { return client.send(msg); },
        [&]() { return server.receive(); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfNonEmptyMessageWithTimedSendAndReceive)
{
    ::testing::Test::RecordProperty("TEST_ID", "b5b2b116-04df-4ec8-ba2c-71ca2ff98b3a");
    successfulSendAndReceive(
        {"the earth is a disc on the back of elephants on the slimy back of hypnotoad - let's all hope that no "
         "elephant slips."},
        [&](auto& msg) { return client.timedSend(msg, 1_ms); },
        [&]() { return server.receive(); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfNonEmptyMessageWithTimedSendAndTimedReceive)
{
    ::testing::Test::RecordProperty("TEST_ID", "7b5f4b19-4721-42e4-899f-9b61d5f2e467");
    successfulSendAndReceive(
        {"it is not the sun that rises, it is hypnotoad who is opening its eyes"},
        [&](auto& msg) { return client.timedSend(msg, 1_ms); },
        [&]() { return server.timedReceive(1_ms); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfNonEmptyMessageWithSendAndTimedReceive)
{
    ::testing::Test::RecordProperty("TEST_ID", "48dfea98-9b8f-4bc5-ba6b-b29229238c1c");
    successfulSendAndReceive(
        {"what is the most beautiful color in the world? it's hypnotoad."},
        [&](auto& msg) { return client.send(msg); },
        [&]() { return server.timedReceive(1_ms); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfEmptyMessageWithSendAndReceive)
{
    ::testing::Test::RecordProperty("TEST_ID", "1cbb2b57-5bde-4d36-b11d-879f55a313c0");
    successfulSendAndReceive(
        {""}, [&](auto& msg) { return client.send(msg); }, [&]() { return server.receive(); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfEmptyMessageWithTimedSendAndReceive)
{
    ::testing::Test::RecordProperty("TEST_ID", "1fecbbc7-762c-4dcd-b7c2-c195d29d4023");
    successfulSendAndReceive(
        {""}, [&](auto& msg) { return client.timedSend(msg, 1_ms); }, [&]() { return server.receive(); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfEmptyMessageWithTimedSendAndTimedReceive)
{
    ::testing::Test::RecordProperty("TEST_ID", "22d0ed9c-6ab1-4239-909e-41dccc0f9510");
    successfulSendAndReceive(
        {""}, [&](auto& msg) { return client.timedSend(msg, 1_ms); }, [&]() { return server.timedReceive(1_ms); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfEmptyMessageWithSendAndTimedReceive)
{
    ::testing::Test::RecordProperty("TEST_ID", "16ee1bee-67a0-4d2f-8f13-5fe6ca67f3b8");
    successfulSendAndReceive(
        {""}, [&](auto& msg) { return client.send(msg); }, [&]() { return server.timedReceive(1_ms); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfMaxLengthMessageWithSendAndReceive)
{
    ::testing::Test::RecordProperty("TEST_ID", "51fb179e-7256-47e8-8af9-6f14493ef253");
    successfulSendAndReceive(
        {std::string(UnixDomainSocket::MAX_MESSAGE_SIZE, 'x')},
        [&](auto& msg) { return client.send(msg); },
        [&]() { return server.receive(); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfMaxLengthMessageWithTimedSendAndReceive)
{
    ::testing::Test::RecordProperty("TEST_ID", "c5e9dbea-c514-4335-a151-bd38a806f048");
    successfulSendAndReceive(
        {std::string(UnixDomainSocket::MAX_MESSAGE_SIZE, 'x')},
        [&](auto& msg) { return client.timedSend(msg, 1_ms); },
        [&]() { return server.receive(); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfMaxLengthMessageWithTimedSendAndTimedReceive)
{
    ::testing::Test::RecordProperty("TEST_ID", "6359e2bc-46ea-4cfa-9c51-bb3e5ad36834");
    successfulSendAndReceive(
        {std::string(UnixDomainSocket::MAX_MESSAGE_SIZE, 'x')},
        [&](auto& msg) { return client.timedSend(msg, 1_ms); },
        [&]() { return server.timedReceive(1_ms); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfMaxLengthMessageWithSendAndTimedReceive)
{
    ::testing::Test::RecordProperty("TEST_ID", "ec6b3ae4-5a87-499c-b41a-c759ee5a14f5");
    successfulSendAndReceive(
        {std::string(UnixDomainSocket::MAX_MESSAGE_SIZE, 'x')},
        [&](auto& msg) { return client.send(msg); },
        [&]() { return server.timedReceive(1_ms); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfMultipleMessagesWithSendAndReceive)
{
    ::testing::Test::RecordProperty("TEST_ID", "d0dd293f-8dc5-493b-99bc-34859eaa7ca6");
    successfulSendAndReceive(
        {"Famous hypnotoad alike creators from around the world:",
         "Zoich, proposed mascot for the winter olympics 2014",
         "Ed Bighead",
         "Jason Funderburker"},
        [&](auto& msg) { return client.send(msg); },
        [&]() { return server.receive(); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfMultipleMessagesWithTimedSendAndReceive)
{
    ::testing::Test::RecordProperty("TEST_ID", "92cb2d91-2fa8-4600-bb42-042cfe97de01");
    successfulSendAndReceive(
        {"Facts about hypnotoad",
         "according to 'The Thief of Baghead' hypnotoad is divorced and has children",
         "hypnotoad is shown in the open sequence in Simpsons - Treehouse of Horror XXIV",
         "hypnotoad has its own tv show called: everyone loves hypnotoad",
         "his homeworld is maybe Kif Krokers homeworld",
         "he knows the answer to the ultimate question of life, the universe, and everything - just look deep into ",
         "his eyes"},
        [&](auto& msg) { return client.timedSend(msg, 1_ms); },
        [&]() { return server.receive(); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfMultipleMessagesWithTimedSendAndTimedReceive)
{
    ::testing::Test::RecordProperty("TEST_ID", "31daf91d-1b98-400e-a29b-e43643962dcc");
    successfulSendAndReceive(
        {"hypnotoad was part of the german pop band Modern Talking and produced songs like",
         "you're my, heart you're my seal",
         "cheri cheri hypnotoad",
         "brother hypno hypno toad",
         "you are not alone hypnotoad is there for you"},
        [&](auto& msg) { return client.timedSend(msg, 1_ms); },
        [&]() { return server.timedReceive(1_ms); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfMultipleMessagesWithSendAndTimedReceive)
{
    ::testing::Test::RecordProperty("TEST_ID", "eb25f813-ab2d-40e4-a363-5e025a2d53c8");
    successfulSendAndReceive(
        {"most famous actors and politicians claim that the licked hypnotoad which was later the key to their "
         "success",
         "homer simpson licked hypnotoad before he was famous (Missionary Impossible)",
         "but remember, always ask the toad before licking otherwise it is just rude",
         "if the toad answers you the licking question, please consult David Hasselhof first or some other random "
         "person"},
        [&](auto& msg) { return client.send(msg); },
        [&]() { return server.timedReceive(1_ms); });
}

void unableToSendTooLongMessage(const sendCall_t& send)
{
    std::string message(UnixDomainSocket::MAX_MESSAGE_SIZE + 1, 'x');
    auto result = send(message);
    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(result.get_error(), IpcChannelError::MESSAGE_TOO_LONG);
}

TEST_F(UnixDomainSocket_test, UnableToSendTooLongMessageWithSend)
{
    ::testing::Test::RecordProperty("TEST_ID", "1af52c13-bc61-4d01-889b-4df7773edb44");
    unableToSendTooLongMessage([&](auto& msg) { return client.send(msg); });
}

TEST_F(UnixDomainSocket_test, UnableToSendTooLongMessageWithTimedSend)
{
    ::testing::Test::RecordProperty("TEST_ID", "712f1bfe-4ca8-4337-83cd-4483afaeeab5");
    unableToSendTooLongMessage([&](auto& msg) { return client.timedSend(msg, 1_ms); });
}

// the current contract of the unix domain socket is that a server can only receive
// and the client can only send
void receivingOnClientLeadsToError(const receiveCall_t& receive)
{
    auto result = receive();
    EXPECT_TRUE(result.has_error());
    ASSERT_THAT(result.get_error(), Eq(IpcChannelError::INTERNAL_LOGIC_ERROR));
}

TEST_F(UnixDomainSocket_test, ReceivingOnClientLeadsToErrorWithReceive)
{
    ::testing::Test::RecordProperty("TEST_ID", "055b3e28-e958-43e7-ad9b-81a9702009cd");
    receivingOnClientLeadsToError([&] { return client.receive(); });
}

TEST_F(UnixDomainSocket_test, ReceivingOnClientLeadsToErrorWithTimedReceive)
{
    ::testing::Test::RecordProperty("TEST_ID", "f46991ff-29f5-4cf7-9d6d-d1d0b4da97dc");
    receivingOnClientLeadsToError([&] { return client.timedReceive(1_ms); });
}

// is not supported on mac os and behaves there like receive
#if !defined(__APPLE__)
TIMING_TEST_F(UnixDomainSocket_test, TimedReceiveBlocks, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "5c43ae51-35ca-4e3e-b5bc-4261c80b7a4d");
    auto start = std::chrono::steady_clock::now();
    auto msg = server.timedReceive(units::Duration::fromMilliseconds(WAIT_IN_MS.count()));
    auto end = std::chrono::steady_clock::now();
    TIMING_TEST_EXPECT_TRUE(end - start >= WAIT_IN_MS);

    TIMING_TEST_ASSERT_TRUE(msg.has_error());
    TIMING_TEST_EXPECT_TRUE(msg.get_error() == IpcChannelError::TIMEOUT);
})

TIMING_TEST_F(UnixDomainSocket_test, TimedReceiveBlocksUntilMessageIsReceived, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "76df3d40-d420-4c5f-b82a-3bf8b684a21b");
    std::string message = "asdasda";
    std::thread waitThread([&] {
        this->signalThreadReady();
        auto start = std::chrono::steady_clock::now();
        auto msg = server.timedReceive(units::Duration::fromMilliseconds(WAIT_IN_MS.count() * 2));
        auto end = std::chrono::steady_clock::now();
        TIMING_TEST_EXPECT_TRUE(end - start >= WAIT_IN_MS);

        TIMING_TEST_ASSERT_FALSE(msg.has_error());
        TIMING_TEST_EXPECT_TRUE(*msg == message);
    });

    this->waitForThread();
    std::this_thread::sleep_for(WAIT_IN_MS);
    TIMING_TEST_ASSERT_FALSE(client.send(message).has_error());
    waitThread.join();
})
#endif
} // namespace
#endif
