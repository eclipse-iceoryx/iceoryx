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
#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_CONDITION_VARIABLE_SIGNALER_HPP
#define IOX_POSH_POPO_BUILDING_BLOCKS_CONDITION_VARIABLE_SIGNALER_HPP

#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"

namespace iox
{
namespace popo
{
/// @brief ConditionVariableSignaler can notifiy waiting threads and processes using a shared memory condition variable
class ConditionVariableSignaler
{
  public:
    explicit ConditionVariableSignaler(cxx::not_null<ConditionVariableData* const> condVarDataPtr) noexcept;
    virtual ~ConditionVariableSignaler() noexcept = default;
    ConditionVariableSignaler(const ConditionVariableSignaler& rhs) = delete;
    ConditionVariableSignaler(ConditionVariableSignaler&& rhs) noexcept = delete;
    ConditionVariableSignaler& operator=(const ConditionVariableSignaler& rhs) = delete;
    ConditionVariableSignaler& operator=(ConditionVariableSignaler&& rhs) noexcept = delete;

    /// @brief If threads are waiting on the condition variable, this call unblocks one of the waiting threads
    void notifyOne() noexcept;

  protected:
    const ConditionVariableData* getMembers() const noexcept;
    ConditionVariableData* getMembers() noexcept;

  private:
    ConditionVariableData* m_condVarDataPtr{nullptr};
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BUILDING_BLOCKS_CONDITION_VARIABLE_SIGNALER_HPP
