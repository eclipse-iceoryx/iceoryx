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

#include "iceoryx_posh/internal/popo/building_blocks/event_notifier.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"

namespace iox
{
namespace popo
{
EventNotifier::EventNotifier(EventVariableData& dataRef, const uint64_t index) noexcept
    : m_pointerToEventVariableData(&dataRef)
    , m_notificationIndex(index)
{
    if (index >= MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET)
    {
        LogWarn() << "The provided index " << index << " is too large.";
    }
}

void EventNotifier::notify() noexcept
{
    if (m_notificationIndex < MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET)
    {
        m_pointerToEventVariableData->m_activeNotifications[m_notificationIndex].store(true, std::memory_order_release);
    }
    m_pointerToEventVariableData->m_semaphore.post();
}
} // namespace popo
} // namespace iox

