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
#ifndef IOX_POSH_POPO_WAITSET_WAIT_SET_HPP
#define IOX_POSH_POPO_WAITSET_WAIT_SET_HPP

#include "condition_variable_waiter.hpp"
#include "guard_condition.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/waitset/condition.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/cxx/vector.hpp"

namespace iox
{
namespace popo
{
/// @brief Logical disjunction of a certain number of conditions
class WaitSet
{
  public:
    using ConditionVector = cxx::vector<Condition*, MAX_NUMBER_OF_CONDITIONS>;

    explicit WaitSet(cxx::not_null<ConditionVariableData* const> =
                         runtime::PoshRuntime::getInstance().getMiddlewareConditionVariable()) noexcept;

    /// @brief Adds a condition to the internal vector
    /// @return True if successful, false if unsuccessful
    bool attachCondition(Condition& condition) noexcept;

    /// @brief Removes a condition from the internal vector
    /// @return True if successful, false if unsuccessful
    bool detachCondition(const Condition& condition) noexcept;

    /// @brief Clears all conditions from the waitset
    void clear() noexcept;

    /// @brief Blocking wait with time limit till one or more of the condition become true
    /// @param[in] timeout How long shall be waited for a signalling condition
    /// @param[out] ConditionVector vector of condition pointers that have become
    /// fulfilled
    ConditionVector timedWait(units::Duration timeout) noexcept;

    /// @brief Blocking wait till one or more of the condition become true
    /// @param[out] ConditionVector vector of condition pointers that have become
    /// fulfilled
    ConditionVector wait() noexcept;

    /// @brief Returns a reference to the GuardCondition
    GuardCondition& getGuardCondition() noexcept;

  private:
    ConditionVector waitAndReturnFulfilledConditions(cxx::optional<units::Duration> timeout = cxx::nullopt) noexcept;
    ConditionVector m_conditionVector;
    ConditionVariableData* m_conditionVariableDataPtr;
    ConditionVariableWaiter m_conditionVariableWaiter;
    GuardCondition m_guardCondition;
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_WAITSET_WAIT_SET_HPP
