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
cxx::expected<WaitSetError> UserTrigger::attachTo(WaitSet& waitset,
                                                  const uint64_t triggerId,
                                                  const Trigger::Callback<UserTrigger> callback) noexcept
{
    m_trigger.reset();
    m_trigger = Trigger();

    return waitset
        .acquireTrigger(
            this, {this, &UserTrigger::hasTriggered}, {this, &UserTrigger::unsetTrigger}, triggerId, callback)
        .and_then([this](Trigger& trigger) { m_trigger = std::move(trigger); });
}

void UserTrigger::detach() noexcept
{
    unsetTrigger(m_trigger);
}

void UserTrigger::trigger() noexcept
{
    m_wasTriggered.store(true, std::memory_order_relaxed);
    if (m_trigger)
    {
        m_trigger.trigger();
    }
}

bool UserTrigger::hasTriggered() const noexcept
{
    return m_wasTriggered.load(std::memory_order_relaxed);
}

void UserTrigger::resetTrigger() noexcept
{
    m_wasTriggered.store(false, std::memory_order_relaxed);
}

void UserTrigger::unsetTrigger(const Trigger&) noexcept
{
    m_trigger.reset();
}

} // namespace popo
} // namespace iox
