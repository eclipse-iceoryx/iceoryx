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

#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_EVENT_LISTENER_HPP
#define IOX_POSH_POPO_BUILDING_BLOCKS_EVENT_LISTENER_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/event_variable_data.hpp"
#include "iceoryx_utils/cxx/vector.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"

#include <cstdint>

namespace iox
{
namespace popo
{
class EventListener
{
  public:
    EventListener(EventVariableData& dataRef) noexcept
        : m_pointerToEventVariableData(&dataRef)
    {
    }

    cxx::vector<uint64_t, MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET> wait() noexcept
    {
        cxx::vector<uint64_t, MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET> activeNotifications;
        if (getMembers()->m_semaphore.wait().has_error())
        {
            errorHandler(Error::kPOPO__EVENT_VARIABLE_WAITER_SEMAPHORE_CORRUPTED_IN_WAIT, nullptr, ErrorLevel::FATAL);
        }
        else
        {
            // return vector of true entries in activeNotifications
            // mutex?
            for (uint64_t i = 0U; i < MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET; i++)
            {
                if (getMembers()->m_activeNotifications[i])
                {
                    activeNotifications.emplace_back(i);
                }
            }
        }
        return activeNotifications;
    }

    void reset(const uint64_t index) noexcept
    {
        m_pointerToEventVariableData->m_activeNotifications[index] = false;
    }

  private:
    const EventVariableData* getMembers() const noexcept
    {
        return m_pointerToEventVariableData;
    }

  private:
    EventVariableData* m_pointerToEventVariableData{nullptr};
};
} // namespace popo
} // namespace iox

#endif
