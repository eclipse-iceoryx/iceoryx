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

#include "iceoryx_posh/popo/condition.hpp"

namespace iox
{
namespace popo
{
Condition::~Condition() noexcept
{
    // WaitSet is still alive as it uses RAII, hence our contract with the user isn't fulfilled anymore (no dangling
    // condition allowed)
    if (isConditionVariableAttached())
    {
        errorHandler(Error::kPOPO__WAITSET_CONDITION_LIFETIME_ISSUE, nullptr, ErrorLevel::FATAL);
    }
}

Condition::Condition(const Condition& rhs) noexcept
{
    m_conditionVariableAttached.store(rhs.m_conditionVariableAttached, std::memory_order_relaxed);
}

bool Condition::isConditionVariableAttached() const noexcept
{
    return m_conditionVariableAttached.load(std::memory_order_relaxed);
}

bool Condition::attachConditionVariable(ConditionVariableData* const conditionVariableDataPtr) noexcept
{
    if (isConditionVariableAttached())
    {
        return false;
    }

    // Call user implementation
    if (setConditionVariable(conditionVariableDataPtr))
    {
        // Save the info so we can notify the user on illegal destruction
        m_conditionVariableAttached.store(true, std::memory_order_relaxed);
        return true;
    }
    return false;
}

bool Condition::detachConditionVariable() noexcept
{
    if (!isConditionVariableAttached())
    {
        return false;
    }

    // Call user implementation
    if (unsetConditionVariable())
    {
        m_conditionVariableAttached.store(false, std::memory_order_relaxed);
        return true;
    }
    return false;
}

} // namespace popo
} // namespace iox
