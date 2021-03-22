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


#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_EVENT_VARIABLE_DATA_HPP
#define IOX_POSH_POPO_BUILDING_BLOCKS_EVENT_VARIABLE_DATA_HPP

#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"

namespace iox
{
namespace popo
{
/// @brief Shared member variable structure used by EventListener and EventNotifier
struct EventVariableData : public ConditionVariableData
{
    /// @brief sets all entries of notification array to false
    EventVariableData() noexcept;

    /// @brief sets all entries of notification array to false and sets process name
    ///
    /// @param[in] process name of process
    EventVariableData(const ProcessName_t& process) noexcept;

    std::atomic_bool m_activeNotifications[MAX_NUMBER_OF_EVENTS_PER_LISTENER];
};
} // namespace popo
} // namespace iox

#endif

