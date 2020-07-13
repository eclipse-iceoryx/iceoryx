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
#ifndef IOX_POSH_POPO_CONDITION_HPP
#define IOX_POSH_POPO_CONDITION_HPP

#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"

namespace iox
{
namespace popo
{
class WaitSet;
/// @brief Base class representing a generic condition that can be stored in a WaitSet
class Condition
{
  public:
    virtual ~Condition() noexcept;

    /// @brief Was the condition fulfilled since last call?
    virtual bool hasTriggered() const noexcept = 0;
    /// @brief Called by a WaitSet before attaching a Condition to see whether it was already added
    virtual bool isConditionVariableAttached() const noexcept = 0;
    /// @brief Called by a WaitSet to announce the condition variable pointer that usually lives in shared memory
    virtual bool attachConditionVariable(ConditionVariableData* const ConditionVariableDataPtr) noexcept = 0;
    /// @brief Called when removing the condition from a WaitSet
    virtual bool detachConditionVariable() noexcept = 0;

  private:
    friend class WaitSet;
    bool attachConditionVariableIntern(ConditionVariableData* const ConditionVariableDataPtr) noexcept;
    ConditionVariableData* m_conditionVariableDataPtr{nullptr};
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_CONDITION_HPP
