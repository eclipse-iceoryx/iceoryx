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

#define private public
#define protected public


#include "iceoryx_posh/internal/runtime/message_queue_interface.hpp"
#include "iceoryx_posh/internal/runtime/message_queue_message.hpp"

#undef private
#undef protected

#include "mocks/mqueue_mock.hpp"
#include "mocks/time_mock.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"

using namespace ::testing;
using ::testing::Return;

using iox::runtime::MqBase;
using iox::runtime::MqInterfaceCreator;
using iox::runtime::MqInterfaceUser;
using iox::runtime::MqMessage;

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

const std::string ifName = "ifName";
constexpr long maxMessages = 10;
constexpr long messageSize = 512;

//////////////////////////////
// UnitTest Implementations
//////////////////////////////
template <typename T>
void CMqInterface_Receive(T& base)
{
    MqMessage result;

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
    MqMessage result;

    char msg1[] = "msg1,msg2,";
    char invalidMsg2[] = "msg1,msg2";   

    // clock_gettime fails, return false
    EXPECT_CALL(*time_MOCK::mock, clock_gettime(_, _)).WillOnce(Return(-1)).WillOnce(Return(0));
    EXPECT_THAT(base.timedReceive(1, result), Eq(false));
    EXPECT_CALL(*time_MOCK::mock, clock_gettime(_, _)).WillRepeatedly(Return(0));

    // mq_timedreceive failse, return false
    EXPECT_CALL(*mqueue_MOCK::mock, mq_timedreceive(_, _, _, _, _)).WillOnce(Return(-1));
    EXPECT_THAT(base.timedReceive(1, result), Eq(false));

    // valid message received, return true
    EXPECT_CALL(*mqueue_MOCK::mock, mq_timedreceive(_, _, _, _, _))
        .WillOnce(DoAll(SetArrayArgument<1>(std::begin(msg1), std::end(msg1)), Return(0)));
    EXPECT_THAT(base.timedReceive(1, result), Eq(true));
    EXPECT_THAT(result.getMessage(), Eq(msg1));

    // invalid message received, return false
    EXPECT_CALL(*mqueue_MOCK::mock, mq_timedreceive(_, _, _, _, _))
        .WillOnce(DoAll(SetArrayArgument<1>(std::begin(invalidMsg2), std::end(invalidMsg2)), Return(0)));
    EXPECT_THAT(base.timedReceive(1, result), Eq(false));
}

template <typename T>
void CMqInterface_Send(T& base)
{
    MqMessage validMsg, invalidMsg, overflowMsg;
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
    MqMessage validMsg, invalidMsg, overflowMsg;
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
void CMqInterface_IsInitialized(T& base[[gnu::unused]])
{
    // TODO: add correct mock settings with return
    ////    EXPECT_THAT(base.isInitialized(), Eq(true));
}

template <typename T>
void CMqInterface_RunAllMqBaseTests(T& base)
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

template <typename T>
void CMqInterface_CopyCTor()
{
    T* base = new T(ifName, maxMessages, messageSize);
    T destination(*base);

    CMqInterface_RunAllMqBaseTests(*base);
    delete base;

    CMqInterface_RunAllMqBaseTests(destination);
}

template <typename T>
void CMqInterface_MoveCTor()
{
    T* base = new T(ifName, maxMessages, messageSize);
    T destination(std::move(*base));
    delete base;

    CMqInterface_RunAllMqBaseTests(destination);
}

template <typename T>
void CMqInterface_CopyOperator()
{
    T* base = new T(ifName, maxMessages, messageSize);
    T destination("crap", maxMessages, messageSize);
    destination = *base;

    CMqInterface_RunAllMqBaseTests(*base);
    delete base;

    CMqInterface_RunAllMqBaseTests(destination);
}

template <typename T>
void CMqInterface_MoveOperator()
{
    T* base = new T(ifName, maxMessages, messageSize);
    T destination("crap", maxMessages, messageSize);
    destination = std::move(*base);
    delete base;

    CMqInterface_RunAllMqBaseTests(destination);
}

////////////////////////////////
// UnitTests: MqBase
////////////////////////////////

TEST_F(CMqInterface_test, MqBase_StringCTor)
{
    CMqInterface_StringCTor<MqBase>();
}

TEST_F(CMqInterface_test, MqBase_CopyCTor)
{
    CMqInterface_CopyCTor<MqBase>();
}

TEST_F(CMqInterface_test, MqBase_MoveCTor)
{
    CMqInterface_MoveCTor<MqBase>();
}

TEST_F(CMqInterface_test, MqBase_CopyOperator)
{
    CMqInterface_CopyOperator<MqBase>();
}

TEST_F(CMqInterface_test, MqBase_MoveOperator)
{
    CMqInterface_MoveOperator<MqBase>();
}

TEST_F(CMqInterface_test, MqBase_Receive)
{
    MqBase base(ifName, maxMessages, messageSize);
    CMqInterface_Receive<MqBase>(base);
}

TEST_F(CMqInterface_test, MqBase_TimedReceive)
{
    MqBase base(ifName, maxMessages, messageSize);
    CMqInterface_TimedReceive<MqBase>(base);
}

TEST_F(CMqInterface_test, MqBase_Send)
{
    MqBase base(ifName, maxMessages, messageSize);
    CMqInterface_Send<MqBase>(base);
}

TEST_F(CMqInterface_test, MqBase_TimedSend)
{
    MqBase base(ifName, maxMessages, messageSize);
    CMqInterface_TimedSend<MqBase>(base);
}

TEST_F(CMqInterface_test, MqBase_GetInterfaceName)
{
    MqBase base(ifName, maxMessages, messageSize);
    CMqInterface_GetInterfaceName<MqBase>(base);
}

TEST_F(CMqInterface_test, MqBase_IsInitialized)
{
    MqBase base(ifName, maxMessages, messageSize);
    CMqInterface_IsInitialized<MqBase>(base);
}

////////////////////////////////
// UnitTests: MqInterfaceUser
////////////////////////////////

TEST_F(CMqInterface_test, MqInterfaceUser_StringCTor)
{
    CMqInterface_StringCTor<MqInterfaceUser>();
}

TEST_F(CMqInterface_test, MqInterfaceUser_MoveCTor)
{
    CMqInterface_MoveCTor<MqInterfaceUser>();
}

TEST_F(CMqInterface_test, MqInterfaceUser_MoveOperator)
{
    CMqInterface_MoveOperator<MqInterfaceUser>();
}

TEST_F(CMqInterface_test, MqInterfaceUser_Receive)
{
    MqInterfaceUser base(ifName);
    CMqInterface_Receive<MqInterfaceUser>(base);
}

TEST_F(CMqInterface_test, MqInterfaceUser_TimedReceive)
{
    MqInterfaceUser base(ifName);
    CMqInterface_TimedReceive<MqInterfaceUser>(base);
}

TEST_F(CMqInterface_test, MqInterfaceUser_Send)
{
    MqInterfaceUser base(ifName);
    CMqInterface_Send<MqInterfaceUser>(base);
}

TEST_F(CMqInterface_test, MqInterfaceUser_TimedSend)
{
    MqInterfaceUser base(ifName);
    CMqInterface_TimedSend<MqInterfaceUser>(base);
}

TEST_F(CMqInterface_test, MqInterfaceUser_GetInterfaceName)
{
    MqInterfaceUser base(ifName);
    CMqInterface_GetInterfaceName<MqInterfaceUser>(base);
}

TEST_F(CMqInterface_test, MqInterfaceUser_IsInitialized)
{
    MqInterfaceUser base(ifName);
    CMqInterface_IsInitialized<MqInterfaceUser>(base);
}

////////////////////////////////
// UnitTests: MqInterfaceCreator
////////////////////////////////

TEST_F(CMqInterface_test, MqInterfaceCreator_StringCTor)
{
    CMqInterface_StringCTor<MqInterfaceCreator>();
}

TEST_F(CMqInterface_test, MqInterfaceCreator_MoveCTor)
{
    CMqInterface_MoveCTor<MqInterfaceCreator>();
}

TEST_F(CMqInterface_test, MqInterfaceCreator_MoveOperator)
{
    CMqInterface_MoveOperator<MqInterfaceCreator>();
}

TEST_F(CMqInterface_test, MqInterfaceCreator_Receive)
{
    MqInterfaceCreator base(ifName);
    CMqInterface_Receive<MqInterfaceCreator>(base);
}

TEST_F(CMqInterface_test, MqInterfaceCreator_TimedReceive)
{
    MqInterfaceCreator base(ifName);
    CMqInterface_TimedReceive<MqInterfaceCreator>(base);
}

TEST_F(CMqInterface_test, MqInterfaceCreator_Send)
{
    MqInterfaceCreator base(ifName);
    CMqInterface_Send<MqInterfaceCreator>(base);
}

TEST_F(CMqInterface_test, MqInterfaceCreator_TimedSend)
{
    MqInterfaceCreator base(ifName);
    CMqInterface_TimedSend<MqInterfaceCreator>(base);
}

TEST_F(CMqInterface_test, MqInterfaceCreator_GetInterfaceName)
{
    MqInterfaceCreator base(ifName);
    CMqInterface_GetInterfaceName<MqInterfaceCreator>(base);
}

TEST_F(CMqInterface_test, MqInterfaceCreator_IsInitialized)
{
    MqInterfaceCreator base(ifName);
    CMqInterface_IsInitialized<MqInterfaceCreator>(base);
}
