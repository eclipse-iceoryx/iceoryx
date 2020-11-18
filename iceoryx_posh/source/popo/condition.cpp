// Copyright (c) 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/popo/condition.hpp"

#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"


namespace iox
{
namespace popo
{
Condition::Condition(const ConditionType type) noexcept
    : m_type{type}
{
}

Condition::~Condition() noexcept
{
}

void Condition::attach(Trigger&& trigger) noexcept
{
    m_trigger = std::move(trigger);
    setConditionVariable(m_trigger.getConditionVariableData());
}

void Condition::detach() noexcept
{
    if (m_trigger)
    {
        unsetConditionVariable();
        m_trigger.reset();
    }
}

ConditionType Condition::getType() const noexcept
{
    return m_type;
}

} // namespace popo
} // namespace iox
