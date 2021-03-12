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
//
// SPDX-License-Identifier: Apache-2.0
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
    using NotificationVector_t = cxx::vector<cxx::BestFittingType_t<MAX_NUMBER_OF_NOTIFIERS_PER_CONDITION_VARIABLE>,
                                             MAX_NUMBER_OF_NOTIFIERS_PER_CONDITION_VARIABLE>;

    explicit ConditionVariableWaiter(ConditionVariableData& condVarData) noexcept;
    ~ConditionVariableWaiter() noexcept = default;
    ConditionVariableWaiter(const ConditionVariableWaiter& rhs) = delete;
    ConditionVariableWaiter(ConditionVariableWaiter&& rhs) noexcept = delete;
    ConditionVariableWaiter& operator=(const ConditionVariableWaiter& rhs) = delete;
    ConditionVariableWaiter& operator=(ConditionVariableWaiter&& rhs) noexcept = delete;


    void resetSemaphore() noexcept;

    /// @brief Waits until notify is called on the ConditionVariableSignaler or time has run out
    /// @param[in] timeToWait, time to wait until the function returns
    /// @return False if timeout occured, true if no timeout occured
    bool timedWait(const units::Duration timeToWait) noexcept;

    /// @brief Waits until notify is called on the ConditionVariableSignaler
    void wait() noexcept;

    /// @brief Was the ConditionVariableWaiter notified by a ConditionVariableSignaler?
    /// @return true if it was notified otherwise false
    bool wasNotified() const noexcept;

    /// @brief Used in classes to signal a thread which waits in wait() to return
    ///         and stop working. Destroy will send an empty notification to wait() and
    ///         after this call wait() turns into a non blocking call which always
    ///         returns an empty vector.
    void destroy() noexcept;

    /// @brief returns vector of indices of active notifications; blocking if EventVariableData was
    /// not notified unless destroy() was called before. The indices of active notifications is
    /// never empty unless destroy() was called, then it's always empty.
    ///
    /// @return vector of active notifications
    NotificationVector_t waitForNotifications() noexcept;


  protected:
    const ConditionVariableData* getMembers() const noexcept;
    ConditionVariableData* getMembers() noexcept;

  private:
    void reset(const uint64_t index) noexcept;

  private:
    ConditionVariableData* m_condVarDataPtr{nullptr};
    std::atomic_bool m_toBeDestroyed{false};
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BUILDING_BLOCKS_CONDITION_VARIABLE_WAITER_HPP
