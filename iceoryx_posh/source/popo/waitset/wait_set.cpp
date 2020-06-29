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

#include "iceoryx_posh/internal/popo/waitset/wait_set.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

namespace iox
{
namespace popo
{
WaitSet::WaitSet() noexcept
    : m_conditionVariableDataPtr(runtime::PoshRuntime::getInstance().getMiddlewareConditionVariable())
    , m_conditionVariableWaiter(m_conditionVariableDataPtr)
{
    /// @todo Add GuardCondition to m_conditionVector, it's the default condition
}

bool WaitSet::attachCondition(Condition& condition) noexcept
{
    if (!condition.isConditionVariableAttached())
    {
        if (condition.attachConditionVariable(m_conditionVariableDataPtr))
        {
            return m_conditionVector.push_back(&condition);
        }
    }

    return false;
}

bool WaitSet::detachCondition(Condition& condition) noexcept
{
    for (auto currentCondition : m_conditionVector)
    {
        if (currentCondition == &condition)
        {
            m_conditionVector.erase(&currentCondition);
            return true;
        }
    }
    return false;
}

void WaitSet::timedWait(units::Duration timeout) noexcept
{
    m_conditionVariableWaiter.timedWait(timeout);
}

cxx::vector<Condition, MAX_NUMBER_OF_CONDITIONS> WaitSet::wait() noexcept
{
    cxx::vector<Condition, MAX_NUMBER_OF_CONDITIONS> conditionsWithFulfilledPredicate;

    /// @note Inbetween here and last wait someone could have set the trigger to true, hence reset it
    m_conditionVariableWaiter.reset();

    // Is one of the conditons true?
    for (auto currentCondition : m_conditionVector)
    {
        if (currentCondition->hasTrigger())
        {
            conditionsWithFulfilledPredicate.push_back(*currentCondition);
        }
    }

    if (conditionsWithFulfilledPredicate.empty())
    {
        m_conditionVariableWaiter.wait();

        // Check again if one of the conditions is true after we received the signal
        for (auto currentCondition : m_conditionVector)
        {
            if (currentCondition->hasTrigger())
            {
                conditionsWithFulfilledPredicate.push_back(*currentCondition);
            }
        }
    }
    // Return of a copy of all conditions that were fulfilled
    return conditionsWithFulfilledPredicate;
}
} // namespace popo
} // namespace iox
