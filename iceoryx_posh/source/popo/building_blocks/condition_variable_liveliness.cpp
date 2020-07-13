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

#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_liveliness.hpp"

namespace iox
{
namespace popo
{
ConditionVariableLiveliness::ConditionVariableLiveliness(
    cxx::not_null<ConditionVariableData* const> condVarDataPtr) noexcept
    : m_condVarDataPtr(condVarDataPtr)
{
}

void ConditionVariableLiveliness::announce() noexcept
{
    getMembers()->m_referenceCounter.fetch_add(1u, std::memory_order_relaxed);
}

void ConditionVariableLiveliness::recall() noexcept
{
    getMembers()->m_referenceCounter.fetch_sub(1u, std::memory_order_relaxed);
}

uint64_t ConditionVariableLiveliness::getNumberOfUsers() const noexcept
{
    return getMembers()->m_referenceCounter.load(std::memory_order_relaxed);
}

const ConditionVariableData* ConditionVariableLiveliness::getMembers() const noexcept
{
    return m_condVarDataPtr;
}

ConditionVariableData* ConditionVariableLiveliness::getMembers() noexcept
{
    return m_condVarDataPtr;
}

} // namespace popo
} // namespace iox
