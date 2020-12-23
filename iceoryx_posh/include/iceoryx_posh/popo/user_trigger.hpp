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
    UserTrigger() noexcept = default;
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

    template <uint64_t>
    friend class WaitSet;

  private:
    void invalidateTrigger(const uint64_t uniqueTriggerId) noexcept;

    /// @brief enables the trigger event
    /// @param[in] waitset reference to the waitset to which the UserTrigger should be attached
    /// @param[in] eventId optional parameter, the id of the corresponding event
    /// @param[in] callback optional parameter, the callback of the event
    /// @return if the event could not be attached to the given waitset the expected contains the error, otherwise
    /// the expected signals success
    template <uint64_t WaitSetCapacity>
    cxx::expected<WaitSetError> enableEvent(WaitSet<WaitSetCapacity>& waitset,
                                            const uint64_t eventId = EventInfo::INVALID_ID,
                                            const EventInfo::Callback<UserTrigger> callback = nullptr) noexcept;

    /// @brief enables the trigger event
    /// @param[in] waitset reference to the waitset to which the UserTrigger should be attached
    /// @param[in] callback optional parameter, the callback of the event
    /// @return if the event could not be attached to the given waitset the expected contains the error, otherwise
    /// the expected signals success
    template <uint64_t WaitSetCapacity>
    cxx::expected<WaitSetError> enableEvent(WaitSet<WaitSetCapacity>& waitset,
                                            const EventInfo::Callback<UserTrigger> callback) noexcept;

    /// @brief disables the trigger event. If it was not enabled nothing happens
    /// happens.
    void disableEvent() noexcept;

  private:
    TriggerHandle m_trigger;
    std::atomic_bool m_wasTriggered{false};
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/user_trigger.inl"

#endif // IOX_POSH_POPO_USER_TRIGGER_HPP
