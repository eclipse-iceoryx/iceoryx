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

#include "iceoryx_posh/internal/popo/base_port.hpp"
#include "iceoryx_posh/popo/gateway_discovery.hpp"

#undef private
#undef protected

using namespace ::testing;
using ::testing::Return;

using CaproMessage = iox::capro::CaproMessage;
using BasePort = iox::popo::BasePort;
using InterfacePort = iox::popo::InterfacePort;


class InterfacePort_mock
{
  public:
    bool getCaProMessage(CaproMessage& f_message)
    {
        f_message.m_serviceDescription = iox::capro::ServiceDescription();
        f_message.m_type = iox::capro::CaproMessageType::ACK;
        return true;
    }
};


class GatewayDiscovery_test : public Test
{
  public:
    void SetUp(){};
    void TearDown(){};
};

TEST_F(GatewayDiscovery_test, GetCaproMessage)
{
    InterfacePort_mock interfacePortImplMock;
    iox::popo::GatewayDiscovery<InterfacePort_mock> GatewayDiscovery(interfacePortImplMock);
    CaproMessage msg;
    GatewayDiscovery.getCaproMessage(msg);
    EXPECT_EQ(iox::capro::CaproMessageType::ACK, msg.m_type);
    EXPECT_EQ(iox::capro::ServiceDescription(), msg.m_serviceDescription);
}
