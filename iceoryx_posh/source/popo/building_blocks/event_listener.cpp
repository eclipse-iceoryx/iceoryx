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


#include "iceoryx_posh/internal/popo/building_blocks/event_listener.hpp"

namespace iox
{
namespace popo
{
EventListener::EventListener(EventVariableData& dataRef) noexcept
    : m_pointerToEventVariableData(&dataRef)
{
}

void EventListener::destroy() noexcept
{
    m_toBeDestroyed.store(true, std::memory_order_relaxed);
    if (m_pointerToEventVariableData->m_semaphore.post().has_error())
    {
        errorHandler(Error::kPOPO__EVENT_VARIABLE_WAITER_SEMAPHORE_CORRUPTED_IN_DESTROY, nullptr, ErrorLevel::FATAL);
    }
}

EventListener::NotificationVector_t EventListener::wait() noexcept
{
    using Type_t = iox::cxx::BestFittingType_t<iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER>;
    NotificationVector_t activeNotifications;

    resetSemaphore();
    while (!m_toBeDestroyed.load(std::memory_order_relaxed))
    {
        for (Type_t i = 0U; i < MAX_NUMBER_OF_EVENTS_PER_LISTENER; i++)
        {
            if (m_pointerToEventVariableData->m_activeNotifications[i].load(std::memory_order_relaxed))
            {
                reset(i);
                activeNotifications.emplace_back(i);
            }
        }
        if (!activeNotifications.empty())
        {
            return activeNotifications;
        }

        if (m_pointerToEventVariableData->m_semaphore.wait().has_error())
        {
            errorHandler(Error::kPOPO__EVENT_VARIABLE_WAITER_SEMAPHORE_CORRUPTED_IN_WAIT, nullptr, ErrorLevel::FATAL);
            break;
        }
    }

    return activeNotifications;
}

void EventListener::reset(const uint64_t index) noexcept
{
    if (index < MAX_NUMBER_OF_EVENTS_PER_LISTENER)
    {
        m_pointerToEventVariableData->m_activeNotifications[index].store(false, std::memory_order_relaxed);
    }
}

void EventListener::resetSemaphore() noexcept
{
    // Count the semaphore down to zero
    bool hasFatalError = false;
    while (!hasFatalError
           && m_pointerToEventVariableData->m_semaphore.tryWait()
                  .or_else([&](posix::SemaphoreError) {
                      errorHandler(
                          Error::kPOPO__EVENT_VARIABLE_WAITER_SEMAPHORE_CORRUPTED_IN_RESET, nullptr, ErrorLevel::FATAL);
                      hasFatalError = true;
                  })
                  .value())
    {
    }
}
} // namespace popo
} // namespace iox

