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

#include "iceoryx_posh/internal/popo/ports/interface_port.hpp"

#include "test.hpp"

namespace
{
using namespace iox;
using namespace iox::popo;
using namespace ::testing;
using ::testing::_;

class InterfacePort_test : public Test
{
  public:
    void SetUp()
    {
    }

    void TearDown()
    {
    }

    capro::CaproMessage generateMessage(const capro::Interfaces interface) noexcept
    {
        return {capro::CaproMessageType::ACK, {"Cheri", "Cheri", "Hypnotoad", {0U, 0U, 0U, 0U}, interface}};
    }
};


TEST_F(InterfacePort_test, EveryMessageCanBeDispatchedWhenInterfacePortIsInternal)
{
    InterfacePortData interfacePortData("", capro::Interfaces::INTERNAL);

    for (uint16_t interface = 0; interface < static_cast<uint16_t>(capro::Interfaces::INTERFACE_END); ++interface)
    {
        auto message = generateMessage(static_cast<capro::Interfaces>(interface));
        InterfacePort(&interfacePortData).dispatchCaProMessage(message);

        auto maybeMessage = InterfacePort(&interfacePortData).tryGetCaProMessage();
        ASSERT_TRUE(maybeMessage.has_value());
        EXPECT_THAT(message.m_serviceDescription, Eq(maybeMessage->m_serviceDescription));
    }
}

TEST_F(InterfacePort_test, MessageDispatchedIfInterfacesDifferWhenInterfacePortIsNotInternal)
{
    for (uint16_t myInterface = 0; myInterface < static_cast<uint16_t>(capro::Interfaces::INTERFACE_END); ++myInterface)
    {
        if (static_cast<capro::Interfaces>(myInterface) == capro::Interfaces::INTERNAL)
        {
            continue;
        }

        InterfacePortData interfacePortData("", static_cast<capro::Interfaces>(myInterface));

        for (uint16_t interface = 0; interface < static_cast<uint16_t>(capro::Interfaces::INTERFACE_END); ++interface)
        {
            if (interface != myInterface)
            {
                auto message = generateMessage(static_cast<capro::Interfaces>(interface));
                InterfacePort(&interfacePortData).dispatchCaProMessage(message);

                auto maybeMessage = InterfacePort(&interfacePortData).tryGetCaProMessage();
                ASSERT_TRUE(maybeMessage.has_value());
                EXPECT_THAT(message.m_serviceDescription, Eq(maybeMessage->m_serviceDescription));
            }
        }
    }
}

TEST_F(InterfacePort_test, MessageDiscaredIfInterfacesAreEqualWhenInterfacePortIsNotInternal)
{
    for (uint16_t myInterface = 0; myInterface < static_cast<uint16_t>(capro::Interfaces::INTERFACE_END); ++myInterface)
    {
        if (static_cast<capro::Interfaces>(myInterface) == capro::Interfaces::INTERNAL)
        {
            continue;
        }

        InterfacePortData interfacePortData("", static_cast<capro::Interfaces>(myInterface));

        auto message = generateMessage(static_cast<capro::Interfaces>(myInterface));
        InterfacePort(&interfacePortData).dispatchCaProMessage(message);

        auto maybeMessage = InterfacePort(&interfacePortData).tryGetCaProMessage();
        ASSERT_FALSE(maybeMessage.has_value());
    }
}
} // namespace
