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

#include "iceoryx_posh/internal/popo/waitset/guard_condition.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

namespace iox
{
namespace popo
{
GuardCondition::GuardCondition(cxx::not_null<ConditionVariableData* const> condVarDataPtr) noexcept
    : m_conditionVariableSignaler(condVarDataPtr)
{
}

void GuardCondition::notify() noexcept
{
    m_wasTriggered = true;
    m_conditionVariableSignaler.notifyOne();
}

bool GuardCondition::hasTrigger() noexcept
{
    return m_wasTriggered.load(std::memory_order_relaxed);
}

} // namespace popo
} // namespace iox
