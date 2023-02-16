// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/internal/popo/building_blocks/condition_listener.hpp"
#include "iceoryx_posh/error_handling/error_handling.hpp"

namespace iox
{
namespace popo
{
ConditionListener::ConditionListener(ConditionVariableData& condVarData) noexcept
    : m_condVarDataPtr(&condVarData)
{
}

void ConditionListener::resetSemaphore() noexcept
{
    // Count the semaphore down to zero
    bool hasFatalError = false;
    while (!hasFatalError
           && getMembers()
                  ->m_semaphore->tryWait()
                  .or_else([&](posix::SemaphoreError) {
                      errorHandler(PoshError::POPO__CONDITION_LISTENER_SEMAPHORE_CORRUPTED_IN_RESET, ErrorLevel::FATAL);
                      hasFatalError = true;
                  })
                  .value())
    {
    }
}

void ConditionListener::destroy() noexcept
{
    m_toBeDestroyed.store(true, std::memory_order_relaxed);
    getMembers()->m_semaphore->post().or_else([](auto) {
        errorHandler(PoshError::POPO__CONDITION_LISTENER_SEMAPHORE_CORRUPTED_IN_DESTROY, ErrorLevel::FATAL);
    });
}

bool ConditionListener::wasNotified() const noexcept
{
    return getMembers()->m_wasNotified.load(std::memory_order_relaxed);
}

ConditionListener::NotificationVector_t ConditionListener::wait() noexcept
{
    return waitImpl([this]() -> bool {
        if (this->getMembers()->m_semaphore->wait().has_error())
        {
            errorHandler(PoshError::POPO__CONDITION_LISTENER_SEMAPHORE_CORRUPTED_IN_WAIT, ErrorLevel::FATAL);
            return false;
        }
        return true;
    });
}

ConditionListener::NotificationVector_t ConditionListener::timedWait(const units::Duration& timeToWait) noexcept
{
    return waitImpl([this, timeToWait]() -> bool {
        if (this->getMembers()->m_semaphore->timedWait(timeToWait).has_error())
        {
            errorHandler(PoshError::POPO__CONDITION_LISTENER_SEMAPHORE_CORRUPTED_IN_TIMED_WAIT, ErrorLevel::FATAL);
        }
        return false;
    });
}

ConditionListener::NotificationVector_t ConditionListener::waitImpl(const function_ref<bool()>& waitCall) noexcept
{
    using Type_t = iox::BestFittingType_t<iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER>;
    NotificationVector_t activeNotifications;

    resetSemaphore();
    bool doReturnAfterNotificationCollection = false;
    while (!m_toBeDestroyed.load(std::memory_order_relaxed))
    {
        for (Type_t i = 0U; i < MAX_NUMBER_OF_NOTIFIERS; i++)
        {
            if (getMembers()->m_activeNotifications[i].load(std::memory_order_relaxed))
            {
                resetUnchecked(i);
                activeNotifications.emplace_back(i);
            }
        }
        if (!activeNotifications.empty() || doReturnAfterNotificationCollection)
        {
            return activeNotifications;
        }

        doReturnAfterNotificationCollection = !waitCall();
    }

    return activeNotifications;
}

void ConditionListener::resetUnchecked(const uint64_t index) noexcept
{
    getMembers()->m_activeNotifications[index].store(false, std::memory_order_relaxed);
    getMembers()->m_wasNotified.store(false, std::memory_order_relaxed);
}

const ConditionVariableData* ConditionListener::getMembers() const noexcept
{
    return m_condVarDataPtr;
}

ConditionVariableData* ConditionListener::getMembers() noexcept
{
    return m_condVarDataPtr;
}

} // namespace popo
} // namespace iox
