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
#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_CONDITION_VARIABLE_WAITER_HPP
#define IOX_POSH_POPO_BUILDING_BLOCKS_CONDITION_VARIABLE_WAITER_HPP

#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/mepoo/memory_info.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"

namespace iox
{
namespace popo
{
/// @brief ConditionVariableWaiter allows one to wait using a shared memory condition variable
class ConditionVariableWaiter
{
  public:
    explicit ConditionVariableWaiter(cxx::not_null<ConditionVariableData* const> condVarDataPtr) noexcept;
    virtual ~ConditionVariableWaiter() noexcept = default;
    ConditionVariableWaiter(const ConditionVariableWaiter& rhs) = delete;
    ConditionVariableWaiter(ConditionVariableWaiter&& rhs) noexcept = delete;
    ConditionVariableWaiter& operator=(const ConditionVariableWaiter& rhs) = delete;
    ConditionVariableWaiter& operator=(ConditionVariableWaiter&& rhs) noexcept = delete;

    /// @brief Reinitialises the condition variable
    void reset() noexcept;
    /// @brief Waits until notify is called on the ConditionVariableSignaler or time has run out
    /// @param[in] timeToWait, time to wait until the function returns
    /// @return False if timeout occured, true if no timeout occured
    bool timedWait(const units::Duration timeToWait) noexcept;
    /// @brief Waits until notify is called on the ConditionVariableSignaler
    void wait() noexcept;

    /// @brief Was the ConditionVariableWaiter notified by a ConditionVariableSignaler?
    /// @return true if it was notified otherwise false
    bool wasNotified() const noexcept;

  protected:
    const ConditionVariableData* getMembers() const noexcept;
    ConditionVariableData* getMembers() noexcept;

  private:
    ConditionVariableData* m_condVarDataPtr{nullptr};
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BUILDING_BLOCKS_CONDITION_VARIABLE_WAITER_HPP
