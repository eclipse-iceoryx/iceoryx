// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
// limitations under the License

#include "iceoryx_posh/popo/guard_condition.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

namespace iox
{
namespace popo
{
void GuardCondition::trigger() noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);
    if (isConditionVariableAttached() && m_conditionVariableDataPtr)
    {
        m_wasTriggered.store(true, std::memory_order_relaxed);
        ConditionVariableSignaler condVarSignaler{m_conditionVariableDataPtr};
        condVarSignaler.notifyOne();
    }
}

bool GuardCondition::hasTriggered() const noexcept
{
    return m_wasTriggered.load(std::memory_order_relaxed);
}

void GuardCondition::resetTrigger() noexcept
{
    m_wasTriggered.store(false, std::memory_order_relaxed);
}

bool GuardCondition::setConditionVariable(ConditionVariableData* conditionVariableDataPtr) noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);
    m_conditionVariableDataPtr = conditionVariableDataPtr;
    return true;
}

bool GuardCondition::unsetConditionVariable() noexcept
{
    std::lock_guard<std::mutex> g(m_mutex);
    m_conditionVariableDataPtr = nullptr;
    return true;
}

} // namespace popo
} // namespace iox
