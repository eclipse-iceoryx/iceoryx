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

#include "iceoryx_posh/internal/popo/building_blocks/event_variable_data.hpp"

namespace iox
{
namespace popo
{
/// only one event listener per event variable
class EventListener
{
  public:
    EventListener(EventVariableData& dataRef) noexcept;

    cxx::vector<uint64_t, MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET> wait() noexcept;

    void reset(const uint64_t index) noexcept;

  private:
    EventVariableData* m_pointerToEventVariableData{nullptr};
};
} // namespace popo
} // namespace iox

#endif
