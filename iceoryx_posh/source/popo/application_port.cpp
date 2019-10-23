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

#include "iceoryx_posh/internal/popo/application_port.hpp"

namespace iox
{
namespace popo
{
ApplicationPort::ApplicationPort(ApplicationPortData* const f_memberPtr)
    : BasePort(f_memberPtr)
{
}

bool ApplicationPort::dispatchCaProMessage(const capro::CaproMessage& f_message)
{
    return getMembers()->m_caproMessageFiFo.push(f_message);
}

bool ApplicationPort::getCaProMessage(capro::CaproMessage& f_message)
{
    auto msg = getMembers()->m_caproMessageFiFo.pop();
    if (msg.has_value())
    {
        f_message = msg.value();
        return true;
    }
    return false;
}

const typename ApplicationPort::MemberType_t* ApplicationPort::getMembers() const
{
    return reinterpret_cast<const MemberType_t*>(BasePort::getMembers());
}

typename ApplicationPort::MemberType_t* ApplicationPort::getMembers()
{
    return reinterpret_cast<MemberType_t*>(BasePort::getMembers());
}

} // namespace popo
} // namespace iox
