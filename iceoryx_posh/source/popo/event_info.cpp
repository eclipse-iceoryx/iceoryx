// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/popo/event_info.hpp"

namespace iox
{
namespace popo
{
constexpr uint64_t EventInfo::INVALID_ID;

uint64_t EventInfo::getEventId() const noexcept
{
    return m_eventId;
}

bool EventInfo::operator()() const noexcept
{
    if (m_eventOrigin != nullptr && m_callbackPtr != nullptr)
    {
        m_callback(m_eventOrigin, m_callbackPtr);
        return true;
    }
    return false;
}

} // namespace popo
} // namespace iox
