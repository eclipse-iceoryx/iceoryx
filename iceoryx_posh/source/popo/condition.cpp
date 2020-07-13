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
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_liveliness.hpp"

namespace iox
{
namespace popo
{
Condition::~Condition() noexcept
{
    if (m_conditionVariableDataPtr)
    {
        ConditionVariableLiveliness condVarLiveliness{m_conditionVariableDataPtr};
        condVarLiveliness.recall();
    }
}

bool Condition::attachConditionVariableIntern(ConditionVariableData* const ConditionVariableDataPtr) noexcept
{
    // Save the pointer so we can notify the condition variable on destruction
    m_conditionVariableDataPtr = ConditionVariableDataPtr;
    // Call user implementation
    return attachConditionVariable(ConditionVariableDataPtr);
}

} // namespace popo
} // namespace iox
