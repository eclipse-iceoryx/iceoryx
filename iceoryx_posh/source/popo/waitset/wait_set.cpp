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
    : m_conditionVariableWaiter(runtime::PoshRuntime::getInstance().getMiddlewareConditionVariable())
{
    /// @todo Add GuardCondition to m_conditionVector, it's the default condition
}

bool WaitSet::attachCondition(Condition& condition) noexcept
{
    return m_conditionVector.push_back(condition);
}

bool WaitSet::detachCondition(Condition& condition) noexcept
{
    for (auto& currentCondition : m_conditionVector)
    {
        if (currentCondition == condition)
        {
            /// @todo
            //m_conditionVector.erase(currentCondition);
        }
    }
}

void WaitSet::timedWait(units::Duration timeout) noexcept
{
    m_conditionVariableWaiter.timedWait(timeout);
}

void WaitSet::wait() noexcept
{
    m_conditionVariableWaiter.wait();
}

} // namespace popo
} // namespace iox
