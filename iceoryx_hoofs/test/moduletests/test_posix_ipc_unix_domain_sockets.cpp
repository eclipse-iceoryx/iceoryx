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
#include "iceoryx_hoofs/testing/timing_test.hpp"
#include "iceoryx_platform/socket.hpp"
#include "iox/atomic.hpp"
#include "iox/posix_call.hpp"
#include "iox/string.hpp"
#include "iox/unix_domain_socket.hpp"

#include "test.hpp"

#include <chrono>
#include <thread>

namespace
{
using namespace ::testing;
using namespace ::testing::internal;
using namespace iox;
using namespace iox::units::duration_literals;
using namespace iox::units;

using sendCall_t = std::function<expected<void, PosixIpcChannelError>(const std::string&)>;
using receiveCall_t = std::function<expected<std::string, PosixIpcChannelError>()>;

using message_t = UnixDomainSocket::Message_t;
using message128_t = iox::string<128>;
using sendCallMsg_t = std::function<expected<void, PosixIpcChannelError>(const message_t&)>;
using receiveCallMsg_t = std::function<expected<void, PosixIpcChannelError>(message_t&)>;

template <typename Msg>
Msg memsetMessage(char value)
{
    Msg message;
    message.unsafe_raw_access([&](auto* str, const auto info) -> uint64_t {
        uint64_t size = info.total_size - 1;
        std::fill_n(str, size, value);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        str[size] = '\0';
        return size;
    });
    return message;
}

// NOLINTJUSTIFICATION used only for test purposes
// NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
constexpr char goodName[] = "channel_test";

/// @req
/// @brief This test suite verifies the functionality which is specific to the UnixDomainSocket class
/// @pre server and client object are allocated and move to the member object of the fixture
/// @note Most of the UnixDomainSocket functionality is tested in "IpcChannel_test"
class UnixDomainSocket_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
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
        auto nameSize = name.size();
        strncpy(&(sockAddr.sun_path[0]), name.c_str(), static_cast<size_t>(nameSize));

        IOX_POSIX_CALL(iox_socket)
        (AF_LOCAL, SOCK_DGRAM, 0)
            .failureReturnValue(ERROR_CODE)
            .evaluate()
            .and_then([&](auto& r) {
                IOX_POSIX_CALL(iox_bind)
                (r.value,
                 // NOLINTJUSTIFICATION enforced by POSIX API
                 // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                 reinterpret_cast<struct sockaddr*>(&sockAddr),
                 sizeof(sockAddr))
                    .failureReturnValue(ERROR_CODE)
                    .evaluate()
                    .or_else([&](auto&) {
                        IOX_LOG(ERROR, "unable to bind socket");
                        socketCreationSuccess = false;
                    });
            })
            .or_else([&](auto&) {
                IOX_LOG(ERROR, "unable to create socket");
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
    iox::concurrent::Atomic<bool> doWaitForThread{true};
    static constexpr uint64_t MaxMsgNumber = 10U;
    UnixDomainSocket server{UnixDomainSocketBuilder()
                                .name(goodName)
                                .channelSide(PosixIpcChannelSide::SERVER)
                                .maxMsgSize(UnixDomainSocket::MAX_MESSAGE_SIZE)
                                .maxMsgNumber(MaxMsgNumber)
                                .create()
                                .expect("Failed to create UnixDomainSocket")};
    UnixDomainSocket client{UnixDomainSocketBuilder()
                                .name(goodName)
                                .channelSide(PosixIpcChannelSide::CLIENT)
                                .maxMsgSize(UnixDomainSocket::MAX_MESSAGE_SIZE)
                                .maxMsgNumber(MaxMsgNumber)
                                .create()
                                .expect("Failed to create UnixDomainSocket")};
};

constexpr uint64_t UnixDomainSocket_test::MaxMsgNumber;

TEST_F(UnixDomainSocket_test, UnlinkEmptySocketNameLeadsToInvalidChannelNameError)
{
    ::testing::Test::RecordProperty("TEST_ID", "bdc1e253-2750-4b07-a528-83ca50246b29");
    auto ret = UnixDomainSocket::unlinkIfExists(UnixDomainSocket::NoPathPrefix, "");
    ASSERT_TRUE(ret.has_error());
    EXPECT_THAT(ret.error(), Eq(PosixIpcChannelError::INVALID_CHANNEL_NAME));
}

TEST_F(UnixDomainSocket_test, UnlinkEmptySocketNameWithPathPrefixLeadsToInvalidChannelNameError)
{
    ::testing::Test::RecordProperty("TEST_ID", "97793649-ac88-4e73-a0bc-602dca302746");
    auto ret = UnixDomainSocket::unlinkIfExists("");
    ASSERT_TRUE(ret.has_error());
    EXPECT_THAT(ret.error(), Eq(PosixIpcChannelError::INVALID_CHANNEL_NAME));
}

TEST_F(UnixDomainSocket_test, UnlinkTooLongSocketNameWithPathPrefixLeadsToInvalidChannelNameError)
{
    ::testing::Test::RecordProperty("TEST_ID", "2fae48fb-8247-4119-a0ec-c40dda87e0c7");
    UnixDomainSocket::UdsName_t longSocketName;
    for (uint64_t i = 0U;
         i < UnixDomainSocket::LONGEST_VALID_NAME - strlen(&platform::IOX_UDS_SOCKET_PATH_PREFIX[0]) + 1;
         ++i)
    {
        longSocketName.append(TruncateToCapacity, "o");
    }
    auto ret = UnixDomainSocket::unlinkIfExists(longSocketName);
    ASSERT_TRUE(ret.has_error());
    EXPECT_THAT(ret.error(), Eq(PosixIpcChannelError::INVALID_CHANNEL_NAME));
}

TEST_F(UnixDomainSocket_test, UnlinkExistingSocketIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "292879cd-89b5-4ebe-8459-f71d13a7befe");
    UnixDomainSocket::UdsName_t socketFileName = platform::IOX_UDS_SOCKET_PATH_PREFIX;
    socketFileName.append(TruncateToCapacity, "iceoryx-hoofs-moduletest.socket");
    ASSERT_TRUE(createTestSocket(socketFileName));
    auto ret = UnixDomainSocket::unlinkIfExists(UnixDomainSocket::NoPathPrefix, socketFileName);
    EXPECT_FALSE(ret.has_error());
}

TEST_F(UnixDomainSocket_test, UnlinkExistingSocketWithPathPrefixLeadsIsSuccessful)
{
    ::testing::Test::RecordProperty("TEST_ID", "33019857-7a2c-4aed-92b1-4218332a254c");
    UnixDomainSocket::UdsName_t socketFileName = "iceoryx-hoofs-moduletest.socket";
    UnixDomainSocket::UdsName_t socketFileNameWithPrefix = platform::IOX_UDS_SOCKET_PATH_PREFIX;
    socketFileNameWithPrefix.append(TruncateToCapacity, socketFileName);
    ASSERT_TRUE(createTestSocket(socketFileNameWithPrefix));
    auto ret = UnixDomainSocket::unlinkIfExists(socketFileName);
    EXPECT_FALSE(ret.has_error());
}

// the current contract of the unix domain socket is that a server can only receive
// and the client can only send
void sendOnServerLeadsToError(const sendCall_t& send)
{
    std::string message{"Foo"};
    auto result = send(message);
    EXPECT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(PosixIpcChannelError::INTERNAL_LOGIC_ERROR));
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

void sendOnServerLeadsToErrorMsg(const sendCallMsg_t& send)
{
    message_t message{"Foo"};
    auto result = send(message);
    EXPECT_TRUE(result.has_error());
    EXPECT_THAT(result.error(), Eq(PosixIpcChannelError::INTERNAL_LOGIC_ERROR));
}

TEST_F(UnixDomainSocket_test, TimedSendOnServerLeadsToErrorMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "b6850755-b8f9-4321-a548-4d0acbb3cbd0");
    sendOnServerLeadsToErrorMsg([&](auto& msg) { return server.timedSend(msg, 1_ms); });
}

TEST_F(UnixDomainSocket_test, SendOnServerLeadsToErrorMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "b4d82b43-8de4-486d-a270-21d60b2f5a61");
    sendOnServerLeadsToErrorMsg([&](auto& msg) { return server.send(msg); });
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

template <typename SendMsg, typename RecvMsg>
void successfulSendAndReceiveMsg(const std::vector<SendMsg>& messages,
                                 const std::function<expected<void, PosixIpcChannelError>(const SendMsg&)>& send,
                                 const std::function<expected<void, PosixIpcChannelError>(RecvMsg&)>& receive)
{
    for (const auto& m : messages)
    {
        ASSERT_FALSE(send(m).has_error());
    }

    for (const auto& sentMessage : messages)
    {
        RecvMsg msg;
        auto receivedMessage = receive(msg);
        ASSERT_FALSE(receivedMessage.has_error());
        EXPECT_EQ(msg, sentMessage);
    }
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfNonEmptyMessageWithSendAndReceiveMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "07fb2de2-151a-436a-8b17-bc940b0c197b");
    successfulSendAndReceiveMsg<message_t, message_t>(
        {"what's hypnotoads eye color?"},
        [&](auto& msg) { return client.send(msg); },
        [&](auto& msg) { return server.receive(msg); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfNonEmptyMessageWithTimedSendAndReceiveMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "6da4ca23-eb10-4afc-9732-42aef9f821bc");
    successfulSendAndReceiveMsg<message_t, message_t>(
        {"the earth is a disc on the back of elephants on the slimy back of hypnotoad - let's all hope that no "
         "elephant slips."},
        [&](auto& msg) { return client.timedSend(msg, 1_ms); },
        [&](auto& msg) { return server.receive(msg); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfNonEmptyMessageWithTimedSendAndTimedReceiveMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "6506b641-4a02-407f-b7de-fbbbbf09d622");
    successfulSendAndReceiveMsg<message_t, message_t>(
        {"it is not the sun that rises, it is hypnotoad who is opening its eyes"},
        [&](auto& msg) { return client.timedSend(msg, 1_ms); },
        [&](auto& msg) { return server.timedReceive(msg, 1_ms); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfNonEmptyMessageWithSendAndTimedReceiveMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "eb517664-f6ee-4504-9c27-686f1d70839e");
    successfulSendAndReceiveMsg<message_t, message_t>(
        {"what is the most beautiful color in the world? it's hypnotoad."},
        [&](auto& msg) { return client.send(msg); },
        [&](auto& msg) { return server.timedReceive(msg, 1_ms); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfNonEmptyMessageWithSend128AndReceiveMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "f43dd0ea-110b-40d6-8077-6e96c57f99ba");
    successfulSendAndReceiveMsg<message128_t, message_t>(
        {"what's hypnotoads eye color?"},
        [&](auto& msg) { return client.send(msg); },
        [&](auto& msg) { return server.receive(msg); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfNonEmptyMessageWithTimedSend128AndReceiveMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "94facde0-65e6-4aac-a1e7-d0415e5ecd7e");
    successfulSendAndReceiveMsg<message128_t, message_t>(
        {"the earth is a disc on the back of elephants on the slimy back of hypnotoad - let's all hope that no "
         "elephant slips."},
        [&](auto& msg) { return client.timedSend(msg, 1_ms); },
        [&](auto& msg) { return server.receive(msg); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfNonEmptyMessageWithTimedSend128AndTimedReceiveMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "f1349116-daed-44a1-9962-aa4bb22fe5a5");
    successfulSendAndReceiveMsg<message128_t, message_t>(
        {"it is not the sun that rises, it is hypnotoad who is opening its eyes"},
        [&](auto& msg) { return client.timedSend(msg, 1_ms); },
        [&](auto& msg) { return server.timedReceive(msg, 1_ms); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfNonEmptyMessageWithSend128AndTimedReceiveMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "7d3c2333-508e-4fcc-8057-87194123c0fb");
    successfulSendAndReceiveMsg<message128_t, message_t>(
        {"what is the most beautiful color in the world? it's hypnotoad."},
        [&](auto& msg) { return client.send(msg); },
        [&](auto& msg) { return server.timedReceive(msg, 1_ms); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfNonEmptyMessageWithSendAndReceive128Msg)
{
    ::testing::Test::RecordProperty("TEST_ID", "645ec34b-325f-459a-b87e-e9288add4392");
    successfulSendAndReceiveMsg<message_t, message128_t>(
        {"what's hypnotoads eye color?"},
        [&](auto& msg) { return client.send(msg); },
        [&](auto& msg) { return server.receive(msg); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfNonEmptyMessageWithTimedSendAndReceive128Msg)
{
    ::testing::Test::RecordProperty("TEST_ID", "b2daa776-fd29-4420-a653-2316e8321643");
    successfulSendAndReceiveMsg<message_t, message128_t>(
        {"the earth is a disc on the back of elephants on the slimy back of hypnotoad - let's all hope that no "
         "elephant slips."},
        [&](auto& msg) { return client.timedSend(msg, 1_ms); },
        [&](auto& msg) { return server.receive(msg); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfNonEmptyMessageWithTimedSendAndTimedReceive128Msg)
{
    ::testing::Test::RecordProperty("TEST_ID", "e48b9dbb-6f9c-4e78-a053-9129f6604a0b");
    successfulSendAndReceiveMsg<message_t, message128_t>(
        {"it is not the sun that rises, it is hypnotoad who is opening its eyes"},
        [&](auto& msg) { return client.timedSend(msg, 1_ms); },
        [&](auto& msg) { return server.timedReceive(msg, 1_ms); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfNonEmptyMessageWithSendAndTimedReceive128Msg)
{
    ::testing::Test::RecordProperty("TEST_ID", "dd1ce7b1-23fe-4547-b3ad-44531f5c9d5f");
    successfulSendAndReceiveMsg<message_t, message128_t>(
        {"what is the most beautiful color in the world? it's hypnotoad."},
        [&](auto& msg) { return client.send(msg); },
        [&](auto& msg) { return server.timedReceive(msg, 1_ms); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfEmptyMessageWithSendAndReceiveMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "b5c5f2bc-b319-4b75-86c2-c44ddd5f8d75");
    successfulSendAndReceiveMsg<message_t, message_t>(
        {""}, [&](auto& msg) { return client.send(msg); }, [&](auto& msg) { return server.receive(msg); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfEmptyMessageWithTimedSendAndReceiveMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "ccbbb0bb-f6b7-420b-9713-7642fd8f4766");
    successfulSendAndReceiveMsg<message_t, message_t>(
        {""}, [&](auto& msg) { return client.timedSend(msg, 1_ms); }, [&](auto& msg) { return server.receive(msg); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfEmptyMessageWithTimedSendAndTimedReceiveMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "5c076821-d02b-4ba8-9329-a8c19555229c");
    successfulSendAndReceiveMsg<message_t, message_t>(
        {""},
        [&](auto& msg) { return client.timedSend(msg, 1_ms); },
        [&](auto& msg) { return server.timedReceive(msg, 1_ms); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfEmptyMessageWithSendAndTimedReceiveMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "f68cfc06-07ae-4830-9f06-0127ecb7bcd8");
    successfulSendAndReceiveMsg<message_t, message_t>(
        {""}, [&](auto& msg) { return client.send(msg); }, [&](auto& msg) { return server.timedReceive(msg, 1_ms); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfFullLenghtMessageWithSendAndReceiveMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "dc059e53-3d31-4ad5-93be-bbf0a1c0425d");
    auto message = memsetMessage<message_t>('a');
    successfulSendAndReceiveMsg<message_t, message_t>(
        {message}, [&](auto& msg) { return client.send(msg); }, [&](auto& msg) { return server.receive(msg); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfFullLenghtMessageWithTimedSendAndReceiveMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "cb6f2575-2753-443f-804e-5e7d34ef6555");
    auto message = memsetMessage<message_t>('a');
    successfulSendAndReceiveMsg<message_t, message_t>(
        {message},
        [&](auto& msg) { return client.timedSend(msg, 1_ms); },
        [&](auto& msg) { return server.receive(msg); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfFullLenghtMessageWithTimedSendAndTimedReceiveMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "2014e782-c228-480b-a018-e7e9fe9f80d5");
    auto message = memsetMessage<message_t>('a');
    successfulSendAndReceiveMsg<message_t, message_t>(
        {message},
        [&](auto& msg) { return client.timedSend(msg, 1_ms); },
        [&](auto& msg) { return server.timedReceive(msg, 1_ms); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfFullLenghtMessageWithSendAndTimedReceiveMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "dd06c632-2bc7-4ff2-96fc-21ab9aa1c711");
    auto message = memsetMessage<message_t>('a');
    successfulSendAndReceiveMsg<message_t, message_t>(
        {message},
        [&](auto& msg) { return client.send(msg); },
        [&](auto& msg) { return server.timedReceive(msg, 1_ms); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfMessageWithSendAndReceiveMsgPrefilled)
{
    ::testing::Test::RecordProperty("TEST_ID", "437b2b55-95e5-4d99-9e23-003eb68dce5d");
    successfulSendAndReceiveMsg<message_t, message_t>(
        {"All glory to the hypnotoad"},
        [&](auto& msg) { return client.send(msg); },
        [&](auto& msg) {
            msg = memsetMessage<message_t>('a');
            return server.receive(msg);
        });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfMultipleMessagesWithSendAndReceiveMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "42263a83-f588-44af-b6ff-d3cdbd01af40");
    successfulSendAndReceiveMsg<message_t, message_t>(
        {"Famous hypnotoad alike creators from around the world:",
         "Zoich, proposed mascot for the winter olympics 2014",
         "Ed Bighead",
         "Jason Funderburker"},
        [&](auto& msg) { return client.send(msg); },
        [&](auto& msg) { return server.receive(msg); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfMultipleMessagesWithTimedSendAndReceiveMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "ed817677-b0b1-4327-a995-ab70a6589e3b");
    successfulSendAndReceiveMsg<message_t, message_t>(
        {"Facts about hypnotoad",
         "according to 'The Thief of Baghead' hypnotoad is divorced and has children",
         "hypnotoad is shown in the open sequence in Simpsons - Treehouse of Horror XXIV",
         "hypnotoad has its own tv show called: everyone loves hypnotoad",
         "his homeworld is maybe Kif Krokers homeworld",
         "he knows the answer to the ultimate question of life, the universe, and everything - just look deep into ",
         "his eyes"},
        [&](auto& msg) { return client.timedSend(msg, 1_ms); },
        [&](auto& msg) { return server.receive(msg); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfMultipleMessagesWithTimedSendAndTimedReceiveMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "4fced677-42fe-49d4-8770-ca787ba35d44");
    successfulSendAndReceiveMsg<message_t, message_t>(
        {"hypnotoad was part of the german pop band Modern Talking and produced songs like",
         "you're my, heart you're my seal",
         "cheri cheri hypnotoad",
         "brother hypno hypno toad",
         "you are not alone hypnotoad is there for you"},
        [&](auto& msg) { return client.timedSend(msg, 1_ms); },
        [&](auto& msg) { return server.timedReceive(msg, 1_ms); });
}

TEST_F(UnixDomainSocket_test, SuccessfulCommunicationOfMultipleMessagesWithSendAndTimedReceiveMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "882553db-05a1-4b68-9e9d-a5510ac78364");
    successfulSendAndReceiveMsg<message_t, message_t>(
        {"most famous actors and politicians claim that the licked hypnotoad which was later the key to their "
         "success",
         "homer simpson licked hypnotoad before he was famous (Missionary Impossible)",
         "but remember, always ask the toad before licking otherwise it is just rude",
         "if the toad answers you the licking question, please consult David Hasselhof first or some other random "
         "person"},
        [&](auto& msg) { return client.send(msg); },
        [&](auto& msg) { return server.timedReceive(msg, 1_ms); });
}

void unableToSendTooLongMessage(const sendCall_t& send)
{
    std::string message(UnixDomainSocket::MAX_MESSAGE_SIZE + 1, 'x');
    auto result = send(message);
    ASSERT_TRUE(result.has_error());
    EXPECT_EQ(result.error(), PosixIpcChannelError::MESSAGE_TOO_LONG);
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
    ASSERT_THAT(result.error(), Eq(PosixIpcChannelError::INTERNAL_LOGIC_ERROR));
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

// the current contract of the unix domain socket is that a server can only receive
// and the client can only send
void receivingOnClientLeadsToErrorMsg(const receiveCallMsg_t& receive)
{
    message_t msg;
    auto result = receive(msg);
    EXPECT_TRUE(result.has_error());
    ASSERT_THAT(result.error(), Eq(PosixIpcChannelError::INTERNAL_LOGIC_ERROR));
}

TEST_F(UnixDomainSocket_test, ReceivingOnClientLeadsToErrorWithReceiveMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "880fc304-2da6-464d-a331-5976e94b60f3");
    receivingOnClientLeadsToErrorMsg([&](auto& msg) { return client.receive(msg); });
}

TEST_F(UnixDomainSocket_test, ReceivingOnClientLeadsToErrorWithTimedReceiveMsg)
{
    ::testing::Test::RecordProperty("TEST_ID", "3c7f88f4-0033-42d4-a0cf-88714f91c14c");
    receivingOnClientLeadsToErrorMsg([&](auto& msg) { return client.timedReceive(msg, 1_ms); });
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
    TIMING_TEST_EXPECT_TRUE(msg.error() == PosixIpcChannelError::TIMEOUT);
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

TIMING_TEST_F(UnixDomainSocket_test, TimedReceiveBlocksMsg, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "13890933-f269-4331-a3e2-358f03b8b200");
    auto start = std::chrono::steady_clock::now();
    message_t msg;
    auto result = server.timedReceive(msg, units::Duration::fromMilliseconds(WAIT_IN_MS.count()));
    auto end = std::chrono::steady_clock::now();
    TIMING_TEST_EXPECT_TRUE(end - start >= WAIT_IN_MS);

    TIMING_TEST_ASSERT_TRUE(result.has_error());
    TIMING_TEST_EXPECT_TRUE(result.error() == PosixIpcChannelError::TIMEOUT);
})

TIMING_TEST_F(UnixDomainSocket_test, TimedReceiveBlocksUntilMessageIsReceivedMsg, Repeat(5), [&] {
    ::testing::Test::RecordProperty("TEST_ID", "fd3e7e5f-b83a-4d25-9463-8174b718ebb1");
    message_t message = "asdasda";
    std::thread waitThread([&] {
        this->signalThreadReady();
        auto start = std::chrono::steady_clock::now();
        message_t msg;
        auto result = server.timedReceive(msg, units::Duration::fromMilliseconds(WAIT_IN_MS.count() * 2));
        auto end = std::chrono::steady_clock::now();
        TIMING_TEST_EXPECT_TRUE(end - start >= WAIT_IN_MS);

        TIMING_TEST_ASSERT_FALSE(result.has_error());
        TIMING_TEST_EXPECT_TRUE(msg == message);
    });

    this->waitForThread();
    std::this_thread::sleep_for(WAIT_IN_MS);
    TIMING_TEST_ASSERT_FALSE(client.send(message).has_error());
    waitThread.join();
})
#endif
} // namespace
#endif
