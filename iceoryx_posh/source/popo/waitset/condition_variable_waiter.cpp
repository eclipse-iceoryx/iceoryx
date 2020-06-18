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

#include "iceoryx_posh/internal/popo/waitset/condition_variable_waiter.hpp"

namespace iox
{
namespace popo
{
ConditionVariableWaiter::ConditionVariableWaiter(cxx::not_null<ConditionVariableData* const> condVarDataPtr) noexcept
    : m_condVarDataPtr(condVarDataPtr)
{
}

void ConditionVariableWaiter::reset() noexcept
{
    // Call c'tor again
    getMembers()->m_semaphore.create(0u);
}

void ConditionVariableWaiter::wait() noexcept
{
    getMembers()->m_semaphore.wait();
}

bool ConditionVariableWaiter::timedWait(units::Duration timeToWait) noexcept
{
    auto timeout = timeToWait.timespec();
    auto continueOnInterrupt{false};
    return getMembers()->m_semaphore.timedWait(&timeout, continueOnInterrupt);
}

const ConditionVariableData* ConditionVariableWaiter::getMembers() const noexcept
{
    return m_condVarDataPtr;
}

ConditionVariableData* ConditionVariableWaiter::getMembers() noexcept
{
    return m_condVarDataPtr;
}

} // namespace popo
} // namespace iox
