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
template class WaitSet<WaitSetPolicy::DEFAULT>;
template class WaitSet<WaitSetPolicy::THREAD_SAFE>;

template <WaitSetPolicy Policy>
WaitSet<Policy>::WaitSet() noexcept
    : WaitSet(runtime::PoshRuntime::getInstance().getMiddlewareConditionVariable())
{
}

template <WaitSetPolicy Policy>
WaitSet<Policy>::WaitSet(cxx::not_null<ConditionVariableData* const> condVarDataPtr) noexcept
    : m_conditionVariableDataPtr(condVarDataPtr)
    , m_conditionVariableWaiter(m_conditionVariableDataPtr)
{
}

template <WaitSetPolicy Policy>
WaitSet<Policy>::~WaitSet() noexcept
{
    detachAllConditions();
    /// @todo Notify RouDi that the condition variable data shall be destroyed
}

template <WaitSetPolicy Policy>
cxx::expected<WaitSetError> WaitSet<Policy>::attachCondition(Condition& condition) noexcept
{
    std::lock_guard<WaitSetMutex> lock(m_mutex);
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

template <WaitSetPolicy Policy>
void WaitSet<Policy>::detachCondition(Condition& condition) noexcept
{
    std::lock_guard<WaitSetMutex> lock(m_mutex);
    if (!condition.isConditionVariableAttached())
    {
        return;
    }

    condition.detachConditionVariable();
}

template <WaitSetPolicy Policy>
void WaitSet<Policy>::remove(void* const entry) noexcept
{
    std::lock_guard<WaitSetMutex> lock(m_mutex);
    for (auto& currentCondition : m_conditionVector)
    {
        if (currentCondition == entry)
        {
            m_conditionVector.erase(&currentCondition);
            return;
        }
    }
}

template <WaitSetPolicy Policy>
void WaitSet<Policy>::detachAllConditions() noexcept
{
    std::lock_guard<WaitSetMutex> lock(m_mutex);
    for (auto& currentCondition : m_conditionVector)
    {
        currentCondition->detachConditionVariable();
    }
    m_conditionVector.clear();
}

template <WaitSetPolicy Policy>
typename WaitSet<Policy>::ConditionVector WaitSet<Policy>::timedWait(const units::Duration timeout) noexcept
{
    return waitAndReturnFulfilledConditions([this, timeout] { return !m_conditionVariableWaiter.timedWait(timeout); });
}

template <WaitSetPolicy Policy>
typename WaitSet<Policy>::ConditionVector WaitSet<Policy>::wait() noexcept
{
    return waitAndReturnFulfilledConditions([this] {
        m_conditionVariableWaiter.wait();
        return false;
    });
}

template <WaitSetPolicy Policy>
typename WaitSet<Policy>::ConditionVector WaitSet<Policy>::createVectorWithFullfilledConditions() noexcept
{
    std::lock_guard<WaitSetMutex> lock(m_mutex);
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

template <WaitSetPolicy Policy>
template <typename WaitFunction>
typename WaitSet<Policy>::ConditionVector
WaitSet<Policy>::waitAndReturnFulfilledConditions(const WaitFunction& wait) noexcept
{
    WaitSet<Policy>::ConditionVector conditions;

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

template <WaitSetPolicy Policy>
bool WaitSet<Policy>::isConditionAttached(const Condition& condition) noexcept
{
    std::lock_guard<WaitSetMutex> lock(m_mutex);
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
