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

#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"

namespace iox
{
namespace popo
{
void UserTrigger::disableEvent() noexcept
{
    m_trigger.reset();
}

void UserTrigger::trigger() noexcept
{
    m_wasTriggered.store(true, std::memory_order_relaxed);
    m_trigger.trigger();
}

bool UserTrigger::hasTriggered() const noexcept
{
    return m_wasTriggered.load(std::memory_order_relaxed);
}

void UserTrigger::resetTrigger() noexcept
{
    m_wasTriggered.store(false, std::memory_order_relaxed);
}

void UserTrigger::invalidateTrigger(const uint64_t uniqueTriggerId) noexcept
{
    if (uniqueTriggerId == m_trigger.getUniqueId())
    {
        m_trigger.invalidate();
    }
}

} // namespace popo
} // namespace iox
