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
#include "mocks/mqueue_mock.hpp"
#include "mocks/time_mock.hpp"

#include "test.hpp"

#define private public
#define protected public

#include "iceoryx_posh/internal/runtime/ipc_interface_creator.hpp"
#include "iceoryx_posh/internal/runtime/ipc_interface_user.hpp"
#include "iceoryx_posh/internal/runtime/ipc_message.hpp"
#include "iceoryx_utils/internal/posix_wrapper/ipc_channel.hpp"

#undef private
#undef protected

#include "iceoryx_utils/internal/units/duration.hpp"

using namespace ::testing;
using ::testing::Return;

using iox::runtime::IpcInterfaceBase;
using iox::runtime::IpcInterfaceCreator;
using iox::runtime::IpcInterfaceUser;
using iox::runtime::IpcMessage;
using namespace iox::units::duration_literals;

class CMqInterface_test : public Test
{
  public:
    CMqInterface_test()
    {
        mqueue_MOCK::doUseMock = true;
        mqueue_MOCK::mock.reset(new NiceMock<mqueue_MOCK>());
        time_MOCK::doUseMock = true;
        time_MOCK::mock.reset(new NiceMock<time_MOCK>());
    }
    ~CMqInterface_test()
    {
        time_MOCK::mock.reset();
        time_MOCK::doUseMock = false;
        mqueue_MOCK::mock.reset();
        mqueue_MOCK::doUseMock = false;
    }
    virtual void SetUp()
    {
        internal::CaptureStderr();
    }
    virtual void TearDown()
    {
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }
};

const iox::ProcessName_t ifName = "ifName";
constexpr long maxMessages = 10;
constexpr long messageSize = 512;

//////////////////////////////
// UnitTest Implementations
//////////////////////////////
template <typename T>
void CMqInterface_Open(T& base)
{
    /// @todo for whatever reason the mock is not called, check why the dlopen trick doesn't work here
    // EXPECT_CALL(*mqueue_MOCK::mock, mq_open(_, _, _, _)).WillOnce(Return(0));
    EXPECT_THAT(base.openMessageQueue(iox::posix::IpcChannelSide::SERVER), Eq(true));
}

template <typename T>
void CMqInterface_Receive(T& base)
{
    IpcMessage result;

    char msg1[] = "msg1,msg2,";
    char invalidMsg2[] = "msg1,msg2";

    // valid message received, return true
    EXPECT_CALL(*mqueue_MOCK::mock, mq_receive(_, _, _, _))
        .WillOnce(DoAll(SetArrayArgument<1>(std::begin(msg1), std::end(msg1)), Return(0)));
    EXPECT_THAT(base.receive(result), Eq(true));
    EXPECT_THAT(result.getMessage(), "msg1,msg2,");

    // mq_receive fails, return false
    EXPECT_CALL(*mqueue_MOCK::mock, mq_receive(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_THAT(base.receive(result), Eq(false));

    // invalid message received, return false
    EXPECT_CALL(*mqueue_MOCK::mock, mq_receive(_, _, _, _))
        .WillOnce(DoAll(SetArrayArgument<1>(std::begin(invalidMsg2), std::end(invalidMsg2)), Return(0)));
    EXPECT_THAT(base.receive(result), Eq(false));
}

template <typename T>
void CMqInterface_TimedReceive(T& base)
{
    IpcMessage result;

    char msg1[] = "msg1,msg2,";
    char invalidMsg2[] = "msg1,msg2";

    // clock_gettime fails, return false
    EXPECT_CALL(*time_MOCK::mock, clock_gettime(_, _)).WillOnce(Return(-1)).WillOnce(Return(0));
    EXPECT_THAT(base.timedReceive(1_ms, result), Eq(false));
    EXPECT_CALL(*time_MOCK::mock, clock_gettime(_, _)).WillRepeatedly(Return(0));

    // mq_timedreceive failse, return false
    EXPECT_CALL(*mqueue_MOCK::mock, mq_timedreceive(_, _, _, _, _)).WillOnce(Return(-1));
    EXPECT_THAT(base.timedReceive(1_ms, result), Eq(false));

    // valid message received, return true
    EXPECT_CALL(*mqueue_MOCK::mock, mq_timedreceive(_, _, _, _, _))
        .WillOnce(DoAll(SetArrayArgument<1>(std::begin(msg1), std::end(msg1)), Return(0)));
    EXPECT_THAT(base.timedReceive(1_ms, result), Eq(true));
    EXPECT_THAT(result.getMessage(), Eq(msg1));

    // invalid message received, return false
    EXPECT_CALL(*mqueue_MOCK::mock, mq_timedreceive(_, _, _, _, _))
        .WillOnce(DoAll(SetArrayArgument<1>(std::begin(invalidMsg2), std::end(invalidMsg2)), Return(0)));
    EXPECT_THAT(base.timedReceive(1_ms, result), Eq(false));
}

template <typename T>
void CMqInterface_Send(T& base)
{
    IpcMessage validMsg, invalidMsg, overflowMsg;
    validMsg.setMessage("msg1,msg2,msg3,");
    invalidMsg.setMessage("msg1,msg2,msg3");

    // create a string which with MSG_SIZE and filled with "@" chars
    std::string msgoverflow(messageSize, '@');

    // append separator, this should lead to an overflow
    msgoverflow.append(",");
    overflowMsg.setMessage(msgoverflow);

    // send invalid message, return false
    EXPECT_THAT(base.send(invalidMsg), Eq(false));

    // send overflow message, return false
    EXPECT_THAT(base.send(overflowMsg), Eq(false));

    // mq_send fails, return false
    EXPECT_CALL(*mqueue_MOCK::mock, mq_send(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_THAT(base.send(validMsg), Eq(false));

    // send valid message, return true
    EXPECT_CALL(*mqueue_MOCK::mock, mq_send(_, _, _, _)).WillOnce(Return(0));
    EXPECT_THAT(base.send(validMsg), Eq(true));
}

template <typename T>
void CMqInterface_TimedSend(T& base)
{
    IpcMessage validMsg, invalidMsg, overflowMsg;
    validMsg.setMessage("msg1,msg2,msg3,");
    invalidMsg.setMessage("msg1,msg2,msg3");

    // create a string which with MSG_SIZE and filled with "@" chars (ASCII Code 64)
    std::string msgoverflow(messageSize, '@');

    // append separator, this should lead to an overflow
    msgoverflow.append(",");
    overflowMsg.setMessage(msgoverflow);

    using namespace iox::units;
    // mq_timedsend failse, return false
    EXPECT_CALL(*mqueue_MOCK::mock, mq_timedsend(_, _, _, _, _)).WillOnce(Return(-1)).WillRepeatedly(Return(0));
    EXPECT_THAT(base.timedSend(validMsg, 1_ms), Eq(false));

    // mq_timedsend valid message, return true
    EXPECT_THAT(base.timedSend(validMsg, 1_ms), Eq(true));

    // mq_timedsend invalid message, return false
    EXPECT_THAT(base.timedSend(invalidMsg, 1_ms), Eq(false));

    // mq_timedsend overflow message, return false
    EXPECT_THAT(base.timedSend(overflowMsg, 1_ms), Eq(false));
}

template <typename T>
void CMqInterface_GetInterfaceName(T& base)
{
    EXPECT_THAT(base.getInterfaceName(), Eq(ifName));
}

template <typename T>
void CMqInterface_IsInitialized(T& base [[gnu::unused]])
{
    // TODO: add correct mock settings with return
    EXPECT_THAT(base.isInitialized(), Eq(true));
}

template <typename T>
void CMqInterface_RunAllIpcInterfaceBaseTests(T& base)
{
    CMqInterface_IsInitialized(base);
    CMqInterface_Receive(base);
    CMqInterface_TimedReceive(base);
    CMqInterface_Send(base);
    CMqInterface_TimedReceive(base);
    CMqInterface_GetInterfaceName(base);
}

template <typename T>
void CMqInterface_StringCTor()
{
    T base(ifName, maxMessages, messageSize);
    EXPECT_THAT(base.getInterfaceName(), Eq(ifName));
}


////////////////////////////////
// UnitTests: IpcInterfaceBase
////////////////////////////////

TEST_F(CMqInterface_test, IpcInterfaceBase_StringCTor)
{
    CMqInterface_StringCTor<IpcInterfaceBase>();
}

TEST_F(CMqInterface_test, DISABLED_IpcInterfaceBase_Receive)
{
    IpcInterfaceBase base(ifName, maxMessages, messageSize);
    CMqInterface_Open(base);
    CMqInterface_Receive<IpcInterfaceBase>(base);
}

TEST_F(CMqInterface_test, DISABLED_IpcInterfaceBase_TimedReceive)
{
    IpcInterfaceBase base(ifName, maxMessages, messageSize);
    CMqInterface_Open(base);
    CMqInterface_TimedReceive<IpcInterfaceBase>(base);
}

TEST_F(CMqInterface_test, DISABLED_IpcInterfaceBase_Send)
{
    IpcInterfaceBase base(ifName, maxMessages, messageSize);
    CMqInterface_Open(base);
    CMqInterface_Send<IpcInterfaceBase>(base);
}

TEST_F(CMqInterface_test, DISABLED_IpcInterfaceBase_TimedSend)
{
    IpcInterfaceBase base(ifName, maxMessages, messageSize);
    CMqInterface_Open(base);
    CMqInterface_TimedSend<IpcInterfaceBase>(base);
}

TEST_F(CMqInterface_test, IpcInterfaceBase_GetInterfaceName)
{
    IpcInterfaceBase base(ifName, maxMessages, messageSize);
    CMqInterface_Open(base);
    CMqInterface_GetInterfaceName<IpcInterfaceBase>(base);
}

TEST_F(CMqInterface_test, IpcInterfaceBase_IsInitialized)
{
    IpcInterfaceBase base(ifName, maxMessages, messageSize);
    CMqInterface_Open(base);
    CMqInterface_IsInitialized<IpcInterfaceBase>(base);
}

////////////////////////////////
// UnitTests: IpcInterfaceUser
////////////////////////////////

TEST_F(CMqInterface_test, IpcInterfaceUser_StringCTor)
{
    CMqInterface_StringCTor<IpcInterfaceUser>();
}

TEST_F(CMqInterface_test, DISABLED_IpcInterfaceUser_Receive)
{
    IpcInterfaceUser base(ifName);
    CMqInterface_Open(base);
    CMqInterface_Receive<IpcInterfaceUser>(base);
}

TEST_F(CMqInterface_test, DISABLED_IpcInterfaceUser_TimedReceive)
{
    IpcInterfaceUser base(ifName);
    CMqInterface_Open(base);
    CMqInterface_TimedReceive<IpcInterfaceUser>(base);
}

TEST_F(CMqInterface_test, DISABLED_IpcInterfaceUser_Send)
{
    IpcInterfaceUser base(ifName);
    CMqInterface_Open(base);
    CMqInterface_Send<IpcInterfaceUser>(base);
}

TEST_F(CMqInterface_test, DISABLED_IpcInterfaceUser_TimedSend)
{
    IpcInterfaceUser base(ifName);
    CMqInterface_Open(base);
    CMqInterface_TimedSend<IpcInterfaceUser>(base);
}

TEST_F(CMqInterface_test, IpcInterfaceUser_GetInterfaceName)
{
    IpcInterfaceUser base(ifName);
    CMqInterface_Open(base);
    CMqInterface_GetInterfaceName<IpcInterfaceUser>(base);
}

TEST_F(CMqInterface_test, IpcInterfaceUser_IsInitialized)
{
    IpcInterfaceUser base(ifName);
    CMqInterface_Open(base);
    CMqInterface_IsInitialized<IpcInterfaceUser>(base);
}

////////////////////////////////
// UnitTests: IpcInterfaceCreator
////////////////////////////////

TEST_F(CMqInterface_test, IpcInterfaceCreator_StringCTor)
{
    CMqInterface_StringCTor<IpcInterfaceCreator>();
}

TEST_F(CMqInterface_test, DISABLED_IpcInterfaceCreator_Receive)
{
    IpcInterfaceCreator base(ifName);
    CMqInterface_Open(base);
    CMqInterface_Receive<IpcInterfaceCreator>(base);
}

TEST_F(CMqInterface_test, DISABLED_IpcInterfaceCreator_TimedReceive)
{
    IpcInterfaceCreator base(ifName);
    CMqInterface_Open(base);
    CMqInterface_TimedReceive<IpcInterfaceCreator>(base);
}

TEST_F(CMqInterface_test, DISABLED_IpcInterfaceCreator_Send)
{
    IpcInterfaceCreator base(ifName);
    CMqInterface_Open(base);
    CMqInterface_Send<IpcInterfaceCreator>(base);
}

TEST_F(CMqInterface_test, DISABLED_IpcInterfaceCreator_TimedSend)
{
    IpcInterfaceCreator base(ifName);
    CMqInterface_Open(base);
    CMqInterface_TimedSend<IpcInterfaceCreator>(base);
}

TEST_F(CMqInterface_test, IpcInterfaceCreator_GetInterfaceName)
{
    IpcInterfaceCreator base(ifName);
    CMqInterface_Open(base);
    CMqInterface_GetInterfaceName<IpcInterfaceCreator>(base);
}

TEST_F(CMqInterface_test, IpcInterfaceCreator_IsInitialized)
{
    IpcInterfaceCreator base(ifName);
    CMqInterface_Open(base);
    CMqInterface_IsInitialized<IpcInterfaceCreator>(base);
}
#endif
