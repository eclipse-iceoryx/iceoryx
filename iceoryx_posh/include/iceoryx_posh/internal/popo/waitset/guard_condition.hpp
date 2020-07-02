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

#include "iceoryx_posh/internal/popo/waitset/condition.hpp"
#include "iceoryx_posh/internal/popo/waitset/condition_variable_signaler.hpp"

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
    GuardCondition(cxx::not_null<ConditionVariableData* const> condVarDataPtr) noexcept;

    /// @brief Wakes up a waiting WaitSet
    void notify() noexcept;
    bool hasTrigger() noexcept override;
    void resetTrigger() noexcept override;
    /// @return Always true on purpose
    bool isConditionVariableAttached() noexcept override;
    /// @return Always false on purpose
    bool attachConditionVariable(ConditionVariableData* ConditionVariableDataPtr) noexcept override;
    /// @return Always false on purpose
    bool detachConditionVariable() noexcept override;

  private:
    ConditionVariableSignaler m_conditionVariableSignaler;
    std::atomic_bool m_wasTriggered{false};
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_WAITSET_CHUNK_RECEIVER_DATA_HPP
