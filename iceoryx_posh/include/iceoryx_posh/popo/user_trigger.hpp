// Copyright (c) 2020, 2021 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_POPO_USER_TRIGGER_HPP
#define IOX_POSH_POPO_USER_TRIGGER_HPP

#include "iceoryx_posh/popo/trigger.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"

#include <atomic>
#include <mutex>

namespace iox
{
namespace popo
{
/// @brief A trigger which can be used by the application developer directly.
///        If you would like to trigger a WaitSet through an event of your class
///        you should use the Trigger class.
class UserTrigger
{
  public:
    UserTrigger() noexcept;
    UserTrigger(const UserTrigger& rhs) = delete;
    UserTrigger(UserTrigger&& rhs) = delete;
    UserTrigger& operator=(const UserTrigger& rhs) = delete;
    UserTrigger& operator=(UserTrigger&& rhs) = delete;

    /// @brief If it is attached it will trigger otherwise it will do nothing
    void trigger() noexcept;

    /// @brief Checks if the UserTrigger was triggered
    /// @return true if the UserTrigger is trigger, otherwise false
    bool hasTriggered() const noexcept;

    /// @brief Resets the UserTrigger state to not triggered
    void resetTrigger() noexcept;

    friend class EventAttorney;

  private:
    /// @brief Only usable by the WaitSet, not for public use. Invalidates the internal triggerHandle.
    /// @param[in] uniqueTriggerId the id of the corresponding trigger
    void invalidateTrigger(const uint64_t uniqueTriggerId) noexcept;

    /// @brief Only usable by the WaitSet, not for public use. Returns method pointer to UserTrigger::hasTriggered
    WaitSetHasTriggeredCallback getHasTriggeredCallbackForEvent() const noexcept;

    /// @brief Only usable by the WaitSet, not for public use. Attaches the triggerHandle to the internal trigger.
    /// @param[in] triggerHandle rvalue reference to the triggerHandle. This class takes the ownership of that handle.
    void enableEvent(iox::popo::TriggerHandle&& triggerHandle) noexcept;

    /// @brief Only usable by the WaitSet, not for public use. Resets the internal triggerHandle
    void disableEvent() noexcept;

  private:
    TriggerHandle m_trigger;
    std::atomic_bool m_wasTriggered{false};
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_USER_TRIGGER_HPP
