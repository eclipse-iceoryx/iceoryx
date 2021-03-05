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
//
// SPDX-License-Identifier: Apache-2.0

#if !defined(_WIN32)
#include "iceoryx_utils/internal/posix_wrapper/message_queue.hpp"
#include "iceoryx_utils/internal/posix_wrapper/unix_domain_socket.hpp"

#include "test.hpp"

#include <chrono>

using namespace ::testing;
using namespace iox;
using namespace iox::posix;

#if defined(__APPLE__)
using IpcChannelTypes = Types<UnixDomainSocket>;
#else
using IpcChannelTypes = Types<MessageQueue, UnixDomainSocket>;
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
class IpcInterfaceCreator_test : public Test
{
  public:
    using IpcChannelType = T;
    
    void SetUp()
    {
        // auto serverResult = IpcChannelType::create(
        //     goodName, IpcChannelMode::BLOCKING, IpcChannelSide::SERVER, MaxMsgSize, MaxMsgNumber);
        // ASSERT_THAT(serverResult.has_error(), Eq(false));
        // server = std::move(serverResult.value());
        // internal::CaptureStderr();

        // auto clientResult = IpcChannelType::create(
        //     goodName, IpcChannelMode::BLOCKING, IpcChannelSide::CLIENT, MaxMsgSize, MaxMsgNumber);
        // ASSERT_THAT(clientResult.has_error(), Eq(false));
        // client = std::move(clientResult.value());
    }

    void TearDown()
    {
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }

    ~IpcInterfaceCreator_test()
    {
    }

    static const size_t MaxMsgSize;
    static constexpr uint64_t MaxMsgNumber = 10U;
    IpcChannelType server;
    IpcChannelType client;
};

template <typename T>
const size_t IpcInterfaceCreator_test<T>::MaxMsgSize = IpcChannelType::MAX_MESSAGE_SIZE;
template <typename T>
constexpr uint64_t IpcInterfaceCreator_test<T>::MaxMsgNumber;

/// we require TYPED_TEST since we support gtest 1.8 for our safety targets
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
TYPED_TEST_CASE(IpcInterfaceCreator_test, IpcChannelTypes);
#pragma GCC diagnostic pop

TYPED_TEST(IpcInterfaceCreator_test, CreateAgainLeadsToError)
{
    // auto serverResult = TestFixture::IpcChannelType::create(goodName,
    //                                                         IpcChannelMode::BLOCKING,
    //                                                         IpcChannelSide::SERVER,
    //                                                         TestFixture::MaxMsgSize + 1,
    //                                                         TestFixture::MaxMsgNumber);
    // EXPECT_TRUE(serverResult.has_error());
    // ASSERT_THAT(serverResult.get_error(), Eq(IpcChannelError::MAX_MESSAGE_SIZE_EXCEEDED));


    // auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
    // [&detectedError](const iox::Error error, const std::function<void()>, const iox::ErrorLevel errorLevel) {
    //     detectedError.emplace(error);
    //     EXPECT_THAT(errorLevel, Eq(iox::ErrorLevel::SEVERE));
    // });
}

#endif
