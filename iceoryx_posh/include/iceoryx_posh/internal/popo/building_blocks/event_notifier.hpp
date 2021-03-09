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


#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_EVENT_NOTIFIER_HPP
#define IOX_POSH_POPO_BUILDING_BLOCKS_EVENT_NOTIFIER_HPP

#include "iceoryx_posh/internal/popo/building_blocks/event_variable_data.hpp"

namespace iox
{
namespace popo
{
/// @brief An EventNotifier notifies a corresponding EventListener via notify() which is
///        waiting on the same EventVariable.
class EventNotifier
{
  public:
    /// @brief creates new EventNotifier
    ///
    /// @param[in] dataRef reference to EventVariableData
    /// @param[in] index index which identifies EventNotifier uniquely. The user has to ensure the uniqueness and the
    /// index has to be in the range of [0, MAX_NUMBER_OF_EVENTS_PER_LISTENER)
    EventNotifier(EventVariableData& dataRef, const uint64_t index) noexcept;

    /// @brief wakes up the corresponding EventListener which is waiting in wait()
    void notify() noexcept;

  private:
    EventVariableData* m_pointerToEventVariableData{nullptr};
    uint64_t m_notificationIndex{0U};
};
} // namespace popo
} // namespace iox

#endif

