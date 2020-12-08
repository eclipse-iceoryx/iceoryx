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
    detachAllConditions();
    /// @todo Notify RouDi that the condition variable data shall be destroyed
}

cxx::expected<WaitSetError> WaitSet::attachCondition(Condition& condition) noexcept
{
    if (!isConditionAttached(condition))
    {
        if (!m_conditionVector.push_back(&condition))
        {
            return cxx::error<WaitSetError>(WaitSetError::CONDITION_VECTOR_OVERFLOW);
        }

        condition.attachConditionVariable(this, m_conditionVariableDataPtr);
    }

    return iox::cxx::success<>();
}

void WaitSet::detachCondition(Condition& condition) noexcept
{
    if (!condition.isConditionVariableAttached())
    {
        return;
    }

    condition.detachConditionVariable();
}

void WaitSet::removeCondition(const Condition& condition) noexcept
{
    for (auto& currentCondition : m_conditionVector)
    {
        if (currentCondition == &condition)
        {
            m_conditionVector.erase(&currentCondition);
            return;
        }
    }
}

void WaitSet::detachAllConditions() noexcept
{
    for (auto& currentCondition : m_conditionVector)
    {
        currentCondition->detachConditionVariable();
    }
    m_conditionVector.clear();
}

WaitSet::ConditionVector WaitSet::timedWait(const units::Duration timeout) noexcept
{
    return waitAndReturnFulfilledConditions([this, timeout] { return !m_conditionVariableWaiter.timedWait(timeout); });
}

WaitSet::ConditionVector WaitSet::wait() noexcept
{
    return waitAndReturnFulfilledConditions([this] {
        m_conditionVariableWaiter.wait();
        return false;
    });
}

WaitSet::ConditionVector WaitSet::createVectorWithFullfilledConditions() noexcept
{
    ConditionVector conditions;
    for (auto& currentCondition : m_conditionVector)
    {
        if (currentCondition->hasTriggered())
        {
            // We do not need to verify if push_back was successful since
            // m_conditionVector and conditions are having the same type, a
            // vector with the same guaranteed capacity.
            // Therefore it is guaranteed that push_back works!
            conditions.push_back(currentCondition);
        }
    }

    return conditions;
}

template <typename WaitFunction>
WaitSet::ConditionVector WaitSet::waitAndReturnFulfilledConditions(const WaitFunction& wait) noexcept
{
    WaitSet::ConditionVector conditions;

    /// Inbetween here and last wait someone could have set the trigger to true, hence reset it.
    m_conditionVariableWaiter.reset();
    conditions = createVectorWithFullfilledConditions();

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

bool WaitSet::isConditionAttached(const Condition& condition) noexcept
{
    for (auto& currentCondition : m_conditionVector)
    {
        if (currentCondition == &condition)
        {
            return true;
        }
    }
    return false;
}


} // namespace popo
} // namespace iox
