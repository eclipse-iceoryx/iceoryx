// Copyright (c) 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_POPO_GUARD_CONDITION_HPP
#define IOX_POSH_POPO_GUARD_CONDITION_HPP

#include "iceoryx_posh/internal/popo/trigger.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"

#include <atomic>
#include <mutex>

namespace iox
{
namespace popo
{
class WaitSet;
/// @brief Allows the user to manually notify inside of one application
/// @note Contained in every WaitSet
class GuardCondition
{
  public:
    GuardCondition() noexcept = default;
    GuardCondition(const GuardCondition& rhs) = delete;
    GuardCondition(GuardCondition&& rhs) = delete;
    GuardCondition& operator=(const GuardCondition& rhs) = delete;
    GuardCondition& operator=(GuardCondition&& rhs) = delete;

    cxx::expected<WaitSetError> attachToWaitset(WaitSet& waitset,
                                                const uint64_t triggerId = Trigger::INVALID_TRIGGER_ID,
                                                const Trigger::Callback<GuardCondition> callback = nullptr) noexcept;

    void detach() noexcept;

    /// @brief Wakes up a waiting WaitSet
    void trigger() noexcept;

    /// @brief Checks if trigger was set
    /// @return True if trigger is set, false if otherwise
    bool hasTriggered() const noexcept;

    /// @brief Sets trigger to false
    void resetTrigger() noexcept;

  private:
    /// @brief Deletes the condition variable data pointer
    void unsetConditionVariable() noexcept;

  private:
    Trigger m_trigger;
    std::atomic_bool m_wasTriggered{false};
    std::recursive_mutex m_mutex;
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_GUARD_CONDITION_HPP
