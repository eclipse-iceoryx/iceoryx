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

#include "iceoryx_posh/internal/popo/interface_port.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"

namespace iox
{
namespace popo
{
InterfacePort::InterfacePort(InterfacePortData* const f_member)
    : BasePort(f_member)
{
}

bool InterfacePort::dispatchCaProMessage(const capro::CaproMessage& f_message)
{
    bool returner = getMembers()->m_caproMessageFiFo.push(f_message);
    if (!returner)
    {
        errorHandler(Error::kPOSH__INTERFACEPORT_CAPRO_MESSAGE_DISMISSED, nullptr, iox::ErrorLevel::SEVERE);
    }
    return returner;
}

bool InterfacePort::getCaProMessage(capro::CaproMessage& f_message)
{
    auto msg = getMembers()->m_caproMessageFiFo.pop();
    if (msg.has_value())
    {
        f_message = msg.value();
        return true;
    }
    return false;
}

const InterfacePortData* InterfacePort::getMembers() const
{
    return reinterpret_cast<const InterfacePortData*>(BasePort::getMembers());
}

InterfacePortData* InterfacePort::getMembers()
{
    return reinterpret_cast<InterfacePortData*>(BasePort::getMembers());
}


} // namespace popo
} // namespace iox
