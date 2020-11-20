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

#include "iceoryx_posh/popo/condition.hpp"

#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"


namespace iox
{
namespace popo
{
Condition::~Condition() noexcept
{
    if (isConditionVariableAttached())
    {
        // In this particular case we can skip unsetConditionVariable since in the
        // dtor the object already degraded to a class which is only a condition
        // and has no more vtable pointer to the method unsetConditionVariable
        // which was once part of this object.
        // We can safely assume that the class which inherits from this class takes
        // care of its resources since it is the responsibility of that class and the
        // destructor was already called at this point.
        m_waitSet->removeCondition(*this);
    }
}

bool Condition::isConditionVariableAttached() const noexcept
{
    return m_waitSet != nullptr;
}

void Condition::attachConditionVariable(WaitSet* const waitSet,
                                        ConditionVariableData* const conditionVariableDataPtr) noexcept
{
    if (isConditionVariableAttached())
    {
        LogWarn()
            << "Attaching an already attached condition leads to a detach from the current WaitSet. Best practice "
               "is to detach Condition first before attaching it.";
        detachConditionVariable();
    }

    setConditionVariable(conditionVariableDataPtr);
    m_waitSet = waitSet;
}

void Condition::detachConditionVariable() noexcept
{
    if (!isConditionVariableAttached())
    {
        return;
    }

    unsetConditionVariable();
    m_waitSet->removeCondition(*this);
    m_waitSet = nullptr;
}

} // namespace popo
} // namespace iox
