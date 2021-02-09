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

#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_EVENT_NOTIFIER_HPP
#define IOX_POSH_POPO_BUILDING_BLOCKS_EVENT_NOTIFIER_HPP

#include "iceoryx_posh/internal/popo/building_blocks/event_variable_data.hpp"

namespace iox
{
namespace popo
{
class EventNotifier
{
  public:
    /// @brief creates new EventNotifier
    ///
    /// @param dataRef reference to EventVariableData
    /// @param index index corresponding to trigger id
    EventNotifier(EventVariableData& dataRef, const uint64_t index) noexcept;

    /// @brief sets corresponding entry in EventVariableData array to true which means that the respective signaller has
    /// triggered the event variable
    void notify() noexcept;

  private:
    EventVariableData* m_pointerToEventVariableData{nullptr};
    uint64_t m_notificationIndex{0U};
};
} // namespace popo
} // namespace iox

#endif

