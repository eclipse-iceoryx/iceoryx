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
#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_CONDITION_VARIABLE_LIVELINESS_HPP
#define IOX_POSH_POPO_BUILDING_BLOCKS_CONDITION_VARIABLE_LIVELINESS_HPP

#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"

namespace iox
{
namespace popo
{
/// @brief Enables RouDi to keep track on how many users of a condition variable are there
class ConditionVariableLiveliness
{
  public:
    explicit ConditionVariableLiveliness(cxx::not_null<ConditionVariableData* const> condVarDataPtr) noexcept;
    virtual ~ConditionVariableLiveliness() noexcept = default;
    ConditionVariableLiveliness(const ConditionVariableLiveliness& rhs) noexcept = delete;
    ConditionVariableLiveliness(ConditionVariableLiveliness&& rhs) noexcept = default;
    ConditionVariableLiveliness& operator=(const ConditionVariableLiveliness& rhs) noexcept = delete;
    ConditionVariableLiveliness& operator=(ConditionVariableLiveliness&& rhs) noexcept = default;

    /// @brief Shall be called to announce the usage of the condition variable
    void announce() noexcept;
    /// @brief Shall be called to recall the usage of the condition variable
    void recall() noexcept;
    /// @brief How many users of the condition variable are still there?
    /// @return Number of users of the condition variable
    uint64_t getNumberOfUsers() const noexcept;

  protected:
    const ConditionVariableData* getMembers() const noexcept;
    ConditionVariableData* getMembers() noexcept;

  private:
    ConditionVariableData* m_condVarDataPtr{nullptr};
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BUILDING_BLOCKS_CONDITION_VARIABLE_LIVELINESS_HPP
