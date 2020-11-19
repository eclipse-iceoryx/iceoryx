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

#include "iceoryx_posh/popo/guard_condition.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"

namespace iox
{
namespace popo
{
cxx::expected<WaitSetError> GuardCondition::attachToWaitset(WaitSet& waitset,
                                                            const uint64_t triggerId,
                                                            const Trigger::Callback<GuardCondition> callback) noexcept
{
    std::lock_guard<std::recursive_mutex> g(m_mutex);
    return waitset
        .acquireTrigger(this,
                        {this, &GuardCondition::hasTriggered},
                        {this, &GuardCondition::unsetConditionVariable},
                        triggerId,
                        callback)
        .and_then([this](Trigger& trigger) { m_trigger = std::move(trigger); });
}

void GuardCondition::detach() noexcept
{
    std::lock_guard<std::recursive_mutex> g(m_mutex);
    m_trigger.reset();
}

void GuardCondition::trigger() noexcept
{
    std::lock_guard<std::recursive_mutex> g(m_mutex);
    if (m_trigger)
    {
        m_wasTriggered.store(true, std::memory_order_relaxed);
        m_trigger.notify();
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

void GuardCondition::unsetConditionVariable() noexcept
{
    std::lock_guard<std::recursive_mutex> g(m_mutex);
    m_trigger.invalidate();
}

} // namespace popo
} // namespace iox
