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

#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/internal/popo/trigger.hpp"
#include "iceoryx_posh/popo/condition.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

namespace iox
{
namespace popo
{
WaitSet::WaitSet() noexcept
    : WaitSet(runtime::PoshRuntime::getInstance().getMiddlewareConditionVariable())
{
}

WaitSet::WaitSet(cxx::not_null<ConditionVariableData* const> condVarDataPtr) noexcept
    : m_conditionVariableDataPtr(condVarDataPtr)
    , m_conditionVariableWaiter(m_conditionVariableDataPtr)
{
}

WaitSet::~WaitSet() noexcept
{
    removeAllTrigger();
    /// @todo Notify RouDi that the condition variable data shall be destroyed
}

cxx::expected<Trigger, WaitSetError> WaitSet::acquireTrigger(Condition& condition,
                                                             const cxx::ConstMethodCallback<bool>& triggerCallback,
                                                             const cxx::MethodCallback<void>& invalidationCallback,
                                                             const uint64_t classId) noexcept
{
    if (!m_conditionVector.push_back(
            Trigger(&condition, triggerCallback, invalidationCallback, m_conditionVariableDataPtr, classId)))
    {
        return cxx::error<WaitSetError>(WaitSetError::CONDITION_VECTOR_OVERFLOW);
    }

    return iox::cxx::success<Trigger>(Trigger(m_conditionVector.back(), {this, &WaitSet::removeTrigger}));
}

void WaitSet::removeTrigger(Trigger& trigger) noexcept
{
    bool wasDeleted = false;
    for (auto& currentTrigger : m_conditionVector)
    {
        if (currentTrigger == trigger)
        {
            m_conditionVector.erase(&currentTrigger);
            wasDeleted = true;
            break;
        }
    }

    if (wasDeleted)
    {
        trigger.invalidate();
    }
}

void WaitSet::removeAllTrigger() noexcept
{
    for (auto& trigger : m_conditionVector)
    {
        trigger.invalidate();
    }

    m_conditionVector.clear();
}

typename WaitSet::ConditionVector WaitSet::timedWait(const units::Duration timeout) noexcept
{
    return waitAndReturnFulfilledConditions([this, timeout] { return !m_conditionVariableWaiter.timedWait(timeout); });
}

typename WaitSet::ConditionVector WaitSet::wait() noexcept
{
    return waitAndReturnFulfilledConditions([this] {
        m_conditionVariableWaiter.wait();
        return false;
    });
}

typename WaitSet::ConditionVector WaitSet::createVectorWithFullfilledConditions() noexcept
{
    ConditionVector conditions;
    for (auto& currentCondition : m_conditionVector)
    {
        if (currentCondition.hasTriggered())
        {
            // We do not need to verify if push_back was successful since
            // m_conditionVector and conditions are having the same type, a
            // vector with the same guaranteed capacity.
            // Therefore it is guaranteed that push_back works!
            conditions.push_back(currentCondition.m_condition);
        }
    }

    return conditions;
}

template <typename WaitFunction>
typename WaitSet::ConditionVector WaitSet::waitAndReturnFulfilledConditions(const WaitFunction& wait) noexcept
{
    WaitSet::ConditionVector conditions;

    if (m_conditionVariableWaiter.wasNotified())
    {
        /// Inbetween here and last wait someone could have set the trigger to true, hence reset it.
        m_conditionVariableWaiter.reset();
        conditions = createVectorWithFullfilledConditions();
    }

    // It is possible that after the reset call and before the createVectorWithFullfilledConditions call
    // another trigger came in. Then createVectorWithFullfilledConditions would have already handled it.
    // But this would lead to an empty conditions vector in the next run if no other trigger
    // came in.
    if (!conditions.empty())
    {
        return conditions;
    }

    return (wait()) ? conditions : createVectorWithFullfilledConditions();
}

} // namespace popo
} // namespace iox
