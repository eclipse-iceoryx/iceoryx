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
#ifndef IOX_POSH_POPO_WAITSET_GUARD_CONDITION_HPP
#define IOX_POSH_POPO_WAITSET_GUARD_CONDITION_HPP

#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_signaler.hpp"
#include "iceoryx_posh/popo/condition.hpp"

#include <atomic>

namespace iox
{
namespace popo
{
/// @brief Allows the user to manually notify inside of one application
/// @note Contained in every WaitSet
class GuardCondition : public Condition
{
  public:
    explicit GuardCondition() noexcept = default;
    GuardCondition(const GuardCondition& rhs) = delete;
    GuardCondition(GuardCondition&& rhs) = delete;
    GuardCondition& operator=(const GuardCondition& rhs) = delete;
    GuardCondition& operator=(GuardCondition&& rhs) = delete;

    /// @brief Wakes up a waiting WaitSet
    void notify() noexcept;
    /// @brief Checks if trigger was set
    /// @return True if trigger is set, false if otherwise
    bool hasTriggered() const noexcept override;
    /// @brief Sets trigger to false
    void resetTrigger() noexcept;
    /// @return Always true on purpose
    bool isConditionVariableAttached() noexcept override;
    /// @return Always false on purpose
    bool attachConditionVariable(ConditionVariableData* ConditionVariableDataPtr) noexcept override;
    /// @return Always false on purpose
    bool detachConditionVariable() noexcept override;

  private:
    ConditionVariableData* m_conditionVariableDataPtr;
    std::atomic_bool m_conditionVariableAttached{false};
    std::atomic_bool m_wasTriggered{false};
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_WAITSET_CHUNK_RECEIVER_DATA_HPP
