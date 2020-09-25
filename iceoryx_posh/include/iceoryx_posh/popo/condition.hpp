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
    Condition() noexcept = default;
    virtual ~Condition() noexcept;
    Condition(const Condition& rhs) noexcept;
    Condition(Condition&& rhs) = delete;
    Condition& operator=(const Condition& rhs) = delete;
    Condition& operator=(Condition&& rhs) = delete;

    /// @brief Was the condition fulfilled since last call?
    virtual bool hasTriggered() const noexcept = 0;
    /// @brief Called by a WaitSet before attaching a Condition to see whether it was already added
    bool isConditionVariableAttached() const noexcept;

  protected:
    friend class WaitSet;
    /// @brief User interface for specific attach of condition variable
    virtual bool setConditionVariable(ConditionVariableData* const conditionVariableDataPtr) noexcept = 0;
    /// @brief User interface for specific detach of condition variable
    virtual bool unsetConditionVariable() noexcept = 0;

    /// @brief Called by a WaitSet to announce the condition variable pointer that usually lives in shared memory
    bool attachConditionVariable(ConditionVariableData* const conditionVariableDataPtr) noexcept;
    /// @brief Called when removing the condition from a WaitSet
    bool detachConditionVariable() noexcept;

    std::atomic_bool m_conditionVariableAttached{false};
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_CONDITION_HPP
