// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "test.hpp"

#include "iceoryx_posh/gateway/gateway_discovery.hpp"
#include "iceoryx_posh/internal/popo/ports/base_port.hpp"

namespace
{
using namespace ::testing;

using CaproMessage = iox::capro::CaproMessage;
using BasePort = iox::popo::BasePort;
using InterfacePort = iox::popo::InterfacePort;

template <typename T>
class GatewayDiscoveryAccess : public iox::gw::GatewayDiscovery<T>
{
  public:
    GatewayDiscoveryAccess(T interfacePortImpl)
        : iox::gw::GatewayDiscovery<T>(interfacePortImpl)
    {
    }
};

class InterfacePort_mock
{
  public:
    bool getCaProMessage(CaproMessage& f_message)
    {
        f_message.m_serviceDescription = iox::capro::ServiceDescription("F", "o", "o");
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
    ::testing::Test::RecordProperty("TEST_ID", "ee901042-940c-4929-b7d0-acad0b752cb3");
    InterfacePort_mock interfacePortImplMock;
    iox::gw::GatewayDiscovery<InterfacePort_mock> GatewayDiscovery =
        GatewayDiscoveryAccess<InterfacePort_mock>(interfacePortImplMock);
    CaproMessage msg;
    GatewayDiscovery.getCaproMessage(msg);
    EXPECT_EQ(iox::capro::CaproMessageType::ACK, msg.m_type);
    EXPECT_EQ(iox::capro::ServiceDescription("F", "o", "o"), msg.m_serviceDescription);
}

} // namespace
