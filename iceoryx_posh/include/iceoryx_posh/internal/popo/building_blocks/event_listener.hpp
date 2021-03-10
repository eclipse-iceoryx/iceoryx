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


#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_EVENT_LISTENER_HPP
#define IOX_POSH_POPO_BUILDING_BLOCKS_EVENT_LISTENER_HPP

#include "iceoryx_posh/internal/popo/building_blocks/event_variable_data.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"

namespace iox
{
namespace popo
{
/// @brief An EventListener performs a blocking wait on a shared event variable.
///        When wait returns a list of all the EventNotifier id's which had
///        triggered the EventVariable is returned and the state is reset.
///
/// @attention Do not use multiple EventListener at the same time for the same EventVariable
class EventListener
{
  public:
    using NotificationVector_t =
        cxx::vector<cxx::BestFittingType_t<MAX_NUMBER_OF_EVENTS_PER_LISTENER>, MAX_NUMBER_OF_EVENTS_PER_LISTENER>;

    /// @brief creates new EventListener
    ///
    /// @param[in] dataRef reference to EventVariableData
    EventListener(EventVariableData& dataRef) noexcept;

    /// @brief returns vector of indices of active notifications; blocking if EventVariableData was
    /// not notified unless destroy() was called before. The indices of active notifications is
    /// never empty unless destroy() was called, then it's always empty.
    ///
    /// @return vector of active notifications
    NotificationVector_t wait() noexcept;

    /// @brief Used in classes to signal a thread which waits in wait() to return
    ///         and stop working. Destroy will send an empty notification to wait() and
    ///         after this call wait() turns into a non blocking call which always
    ///         returns an empty vector.
    void destroy() noexcept;

  private:
    void reset(const uint64_t index) noexcept;
    void resetSemaphore() noexcept;

    std::atomic_bool m_toBeDestroyed{false};
    EventVariableData* m_pointerToEventVariableData{nullptr};
};
} // namespace popo
} // namespace iox

#endif
