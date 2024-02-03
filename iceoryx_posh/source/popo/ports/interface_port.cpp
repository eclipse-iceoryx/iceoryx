// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
#include "iceoryx_posh/internal/posh_error_reporting.hpp"

namespace iox
{
namespace popo
{
InterfacePort::InterfacePort(InterfacePortData* const interfacePortDataPtr) noexcept
    : BasePort(interfacePortDataPtr)
{
}

optional<capro::CaproMessage> InterfacePort::tryGetCaProMessage() noexcept
{
    return getMembers()->m_caproMessageFiFo.pop();
}

void InterfacePort::dispatchCaProMessage(const capro::CaproMessage& caProMessage) noexcept
{
    auto messageInterface = caProMessage.m_serviceDescription.getSourceInterface();
    auto myInterface = getMembers()->m_serviceDescription.getSourceInterface();

    // Do only forward messages for internal ports or if the ports interface is different
    // than the messageInterface otherwise it is possible that a gateway subscribes to its
    // own services. This would lead to running messages in cycles.
    if (myInterface != iox::capro::Interfaces::INTERNAL && myInterface == messageInterface)
    {
        return;
    }

    if (!getMembers()->m_caproMessageFiFo.push(caProMessage))
    {
        // information loss for this interface port
        IOX_REPORT(PoshError::POSH__INTERFACEPORT_CAPRO_MESSAGE_DISMISSED, iox::er::RUNTIME_ERROR);
    }
}

const InterfacePortData* InterfacePort::getMembers() const noexcept
{
    return reinterpret_cast<const InterfacePortData*>(BasePort::getMembers());
}

InterfacePortData* InterfacePort::getMembers() noexcept
{
    return reinterpret_cast<InterfacePortData*>(BasePort::getMembers());
}


} // namespace popo
} // namespace iox
