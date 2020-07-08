// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/internal/roudi/introspection/port_introspection.hpp"
#include "test.hpp"

using namespace ::testing;
using namespace iox::capro;

class CaproMessage_test : public Test
{
  public:
    void SetUp(){};
    void TearDown(){};
};

TEST_F(CaproMessage_test, CtorNoParams)
{
    CaproMessage testObj = CaproMessage();
    EXPECT_EQ(nullptr, testObj.m_requestPort);
    EXPECT_EQ(CaproMessageType::NOTYPE, testObj.m_type);
    EXPECT_EQ(CaproMessageSubType::NOSUBTYPE, testObj.m_subType);
    EXPECT_EQ(nullptr, testObj.m_chunkQueueData);
    EXPECT_EQ(0u, testObj.m_historyCapacity);
}


TEST_F(CaproMessage_test, CtorParams)
{
    uint16_t testServiceID = 1;
    uint16_t testEventID = 2;
    uint16_t testInstanceID = 3;
    ServiceDescription sd = ServiceDescription(testServiceID, testEventID, testInstanceID);
    iox::popo::ReceiverPortData recData;

    CaproMessage testObj = CaproMessage(CaproMessageType::OFFER, sd, CaproMessageSubType::SERVICE, &recData);
    EXPECT_EQ(&recData, testObj.m_requestPort);
    EXPECT_EQ(CaproMessageType::OFFER, testObj.m_type);
    EXPECT_EQ(CaproMessageSubType::SERVICE, testObj.m_subType);
    EXPECT_EQ(nullptr, testObj.m_chunkQueueData);
    EXPECT_EQ(0u, testObj.m_historyCapacity);
    EXPECT_EQ(sd, testObj.m_serviceDescription);
}


TEST_F(CaproMessage_test, CtorDefaultArgs)
{
    uint16_t testServiceID = 1;
    uint16_t testEventID = 2;
    uint16_t testInstanceID = 3;
    ServiceDescription sd = ServiceDescription(testServiceID, testEventID, testInstanceID);
    iox::popo::ReceiverPortData recData;

    CaproMessage testObj = CaproMessage(CaproMessageType::OFFER, sd);
    EXPECT_EQ(nullptr, testObj.m_requestPort);
    EXPECT_EQ(CaproMessageSubType::NOSUBTYPE, testObj.m_subType);
}