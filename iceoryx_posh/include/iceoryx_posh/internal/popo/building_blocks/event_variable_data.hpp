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

#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_EVENT_VARIABLE_DATA_HPP
#define IOX_POSH_POPO_BUILDING_BLOCKS_EVENT_VARIABLE_DATA_HPP

#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_utils/cxx/vector.hpp"

#include <atomic>

namespace iox
{
namespace popo
{
struct EventVariableData : public ConditionVariableData
{
    cxx::vector<std::atomic_bool, 100> m_activeNotifications;
};
} // namespace popo
} // namespace iox

#endif

