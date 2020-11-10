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
// limitations under the License.

#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_waiter.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"

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
    // Count the semaphore down to zero
    while (getMembers()
               ->m_semaphore.tryWait()
               .or_else([](posix::SemaphoreError) {
                   errorHandler(Error::kPOPO__CONDITION_VARIABLE_WAITER_SEMAPHORE_CORRUPTED_IN_RESET,
                                nullptr,
                                ErrorLevel::FATAL);
               })
               .get_value())
    {
    }
}

void ConditionVariableWaiter::wait() noexcept
{
    if (getMembers()->m_semaphore.wait().has_error())
    {
        errorHandler(Error::kPOPO__CONDITION_VARIABLE_WAITER_SEMAPHORE_CORRUPTED_IN_WAIT, nullptr, ErrorLevel::FATAL);
    }
}

bool ConditionVariableWaiter::timedWait(units::Duration timeToWait) noexcept
{
    auto timeout = timeToWait.timespec(units::TimeSpecReference::Epoch);
    auto continueOnInterrupt{false};
    auto result = getMembers()->m_semaphore.timedWait(&timeout, continueOnInterrupt);

    if (result.has_error())
    {
        errorHandler(
            Error::kPOPO__CONDITION_VARIABLE_WAITER_SEMAPHORE_CORRUPTED_IN_TIMED_WAIT, nullptr, ErrorLevel::FATAL);
        return false;
    }

    return *result != posix::SemaphoreWaitState::TIMEOUT;
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
