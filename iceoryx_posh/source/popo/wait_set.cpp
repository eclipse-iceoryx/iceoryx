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

#include "iceoryx_posh/popo/wait_set.hpp"

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
    // Notify all conditions in the vector that the condition variable data will be destroyed
    detachAllConditions();
    /// @todo Notify RouDi that the condition variable data shall be destroyed
}

cxx::expected<WaitSetError> WaitSet::attachCondition(Condition& condition) noexcept
{
    if (!condition.isConditionVariableAttached())
    {
        if (!m_conditionVector.push_back(&condition))
        {
            return cxx::error<WaitSetError>(WaitSetError::CONDITION_VECTOR_OVERFLOW);
        }
        if (!condition.attachConditionVariable(m_conditionVariableDataPtr))
        {
            return cxx::error<WaitSetError>(WaitSetError::CONDITION_VARIABLE_ATTACH_FAILED);
        }
        return iox::cxx::success<>();
    }
    return cxx::error<WaitSetError>(WaitSetError::CONDITION_VARIABLE_ALREADY_SET);
}

bool WaitSet::detachCondition(Condition& condition) noexcept
{
    if (condition.isConditionVariableAttached())
    {
        if (!condition.detachConditionVariable())
        {
            errorHandler(Error::kPOPO__WAITSET_COULD_NOT_DETACH_CONDITION, nullptr, ErrorLevel::FATAL);
            return false;
        }

        for (auto& currentCondition : m_conditionVector)
        {
            if (currentCondition == &condition)
            {
                m_conditionVector.erase(&currentCondition);
                return true;
            }
        }
    }
    return false;
}

void WaitSet::detachAllConditions() noexcept
{
    for (auto& currentCondition : m_conditionVector)
    {
        if (!currentCondition->detachConditionVariable())
        {
            errorHandler(Error::kPOPO__WAITSET_COULD_NOT_DETACH_CONDITION, nullptr, ErrorLevel::FATAL);
        }
    }
    m_conditionVector.clear();
}

WaitSet::ConditionVector WaitSet::timedWait(const units::Duration timeout) noexcept
{
    return waitAndReturnFulfilledConditions<WaitPolicy::TIMED_WAIT>(cxx::make_optional<units::Duration>(timeout));
}

WaitSet::ConditionVector WaitSet::wait() noexcept
{
    return waitAndReturnFulfilledConditions<WaitPolicy::BLOCKING_WAIT>(cxx::nullopt);
}

} // namespace popo
} // namespace iox
