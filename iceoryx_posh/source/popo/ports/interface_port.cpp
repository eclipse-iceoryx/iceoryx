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

#include "iceoryx_posh/internal/popo/ports/interface_port.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"

namespace iox
{
namespace popo
{
InterfacePort::InterfacePort(InterfacePortData* const interfacePortDataPtr) noexcept
    : BasePort(interfacePortDataPtr)
{
}

cxx::optional<capro::CaproMessage> InterfacePort::getCaProMessage() noexcept
{
    return getMembers()->m_caproMessageFiFo.pop();
}

void InterfacePort::dispatchCaProMessage(const capro::CaproMessage& caProMessage) noexcept
{
    if (!getMembers()->m_caproMessageFiFo.push(caProMessage))
    {
        // information loss for this interface port
        errorHandler(Error::kPOSH__INTERFACEPORT_CAPRO_MESSAGE_DISMISSED, nullptr, ErrorLevel::SEVERE);
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
