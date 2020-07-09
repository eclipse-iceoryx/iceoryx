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

namespace iox
{
namespace popo
{
WaitSet::WaitSet(cxx::not_null<ConditionVariableData* const> condVarDataPtr) noexcept
    : m_conditionVariableDataPtr(condVarDataPtr)
    , m_conditionVariableWaiter(m_conditionVariableDataPtr)
{
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

bool WaitSet::detachCondition(const Condition& condition) noexcept
{
    for (auto& currentCondition : m_conditionVector)
    {
        if (currentCondition == &condition)
        {
            m_conditionVector.erase(&currentCondition);
            return true;
        }
    }
    return false;
}

void WaitSet::clear() noexcept
{
    m_conditionVector.clear();
}

WaitSet::ConditionVector WaitSet::waitAndReturnFulfilledConditions(cxx::optional<units::Duration> timeout) noexcept
{
    ConditionVector conditionsWithFulfilledPredicate;

    auto checkIfOneOfConditionsIsFulfilled = [&]() {
        for (auto& currentCondition : m_conditionVector)
        {
            if (currentCondition->hasTrigger())
            {
                if (!conditionsWithFulfilledPredicate.push_back(currentCondition))
                {
                    errorHandler(Error::kPOPO__WAITSET_CONDITION_VECTOR_OVERFLOW, nullptr, ErrorLevel::FATAL);
                }
            }
        }
    };

    /// @note Inbetween here and last wait someone could have set the trigger to true, hence reset it
    m_conditionVariableWaiter.reset();

    // Is one of the conditons true?
    checkIfOneOfConditionsIsFulfilled();

    if (conditionsWithFulfilledPredicate.empty())
    {
        if (timeout.has_value())
        {
            auto hasTimeOut = !m_conditionVariableWaiter.timedWait(timeout.value());

            if (hasTimeOut == true)
            {
                // Return empty list
                return conditionsWithFulfilledPredicate;
            }
        }
        else
        {
            m_conditionVariableWaiter.wait();
        }

        // Check again if one of the conditions is true after we received the signal
        checkIfOneOfConditionsIsFulfilled();
    }
    // Return of a copy of all conditions that were fulfilled
    return conditionsWithFulfilledPredicate;
}

WaitSet::ConditionVector WaitSet::timedWait(units::Duration timeout) noexcept
{
    return waitAndReturnFulfilledConditions(cxx::make_optional<units::Duration>(timeout));
}

WaitSet::ConditionVector WaitSet::wait() noexcept
{
    return waitAndReturnFulfilledConditions(cxx::nullopt);
}

} // namespace popo
} // namespace iox
