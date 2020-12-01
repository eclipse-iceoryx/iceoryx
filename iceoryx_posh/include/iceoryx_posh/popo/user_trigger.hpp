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

    /// @brief attaches the UserTrigger to a WaitSet
    /// @param[in] waitset reference to the waitset to which the UserTrigger should be attached
    /// @param[in] triggerId optional parameter, the id of the trigger
    /// @param[in] callback optional parameter, the callback of the trigger
    /// @param[in] if the trigger could not be attached to the given waitset the expected contains the error, otherwise
    /// the expected signals success
    cxx::expected<WaitSetError> attachTo(WaitSet& waitset,
                                         const uint64_t triggerId = Trigger::INVALID_TRIGGER_ID,
                                         const Trigger::Callback<UserTrigger> callback = nullptr) noexcept;

    /// @brief detaches the UserTrigger from the waitset. If it was not attached to a waitset nothing happens.
    void detach() noexcept;

    /// @brief If it is attached it will trigger otherwise it will do nothing
    void trigger() noexcept;

    /// @brief Checks if the UserTrigger was triggered
    /// @return true if the UserTrigger is trigger, otherwise false
    bool hasTriggered() const noexcept;

    /// @brief Resets the UserTrigger state to not triggered
    void resetTrigger() noexcept;

  private:
    void unsetTrigger(const Trigger&) noexcept;

  private:
    Trigger m_trigger;
    std::atomic_bool m_wasTriggered{false};
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_USER_TRIGGER_HPP
