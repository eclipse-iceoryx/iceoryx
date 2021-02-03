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

#include "iceoryx_posh/internal/popo/building_blocks/event_listener.hpp"

namespace iox
{
namespace popo
{
EventListener::EventListener(EventVariableData& dataRef) noexcept
    : m_pointerToEventVariableData(&dataRef)
{
}

cxx::vector<uint64_t, MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET> EventListener::wait() noexcept
{
    cxx::vector<uint64_t, MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET> activeNotifications;
    if (m_pointerToEventVariableData->m_semaphore.wait().has_error())
    {
        errorHandler(Error::kPOPO__EVENT_VARIABLE_WAITER_SEMAPHORE_CORRUPTED_IN_WAIT, nullptr, ErrorLevel::FATAL);
    }
    else
    {
        for (uint64_t i = 0U; i < MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET; i++)
        {
            if (m_pointerToEventVariableData->m_activeNotifications[i].load(std::memory_order_relaxed))
            {
                activeNotifications.emplace_back(i);
            }
        }
    }
    return activeNotifications;
}

void EventListener::reset(const uint64_t index) noexcept
{
    if (index < MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET)
    {
        m_pointerToEventVariableData->m_activeNotifications[index].store(false, std::memory_order_relaxed);
    }
}
} // namespace popo
} // namespace iox
