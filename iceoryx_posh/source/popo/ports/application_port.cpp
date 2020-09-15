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

#include "iceoryx_posh/internal/popo/ports/application_port.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"

namespace iox
{
namespace popo
{
ApplicationPort::ApplicationPort(ApplicationPortData* const applicationPortDataPtr) noexcept
    : BasePort(applicationPortDataPtr)
{
}

cxx::optional<capro::CaproMessage> ApplicationPort::tryGetCaProMessage() noexcept
{
   return getMembers()->m_caproMessageFiFo.pop();
}

void ApplicationPort::dispatchCaProMessage(const capro::CaproMessage& caProMessage) noexcept
{
    if (!getMembers()->m_caproMessageFiFo.push(caProMessage))
    {
        // information loss from application to RouDi daemon
        errorHandler(Error::kPOPO__APPLICATION_PORT_QUEUE_OVERFLOW, nullptr, ErrorLevel::SEVERE);
    }
}

const typename ApplicationPort::MemberType_t* ApplicationPort::getMembers() const noexcept
{
    return reinterpret_cast<const MemberType_t*>(BasePort::getMembers());
}

typename ApplicationPort::MemberType_t* ApplicationPort::getMembers() noexcept
{
    return reinterpret_cast<MemberType_t*>(BasePort::getMembers());
}

} // namespace popo
} // namespace iox
