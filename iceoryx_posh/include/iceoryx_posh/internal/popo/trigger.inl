// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_TRIGGER_INL
#define IOX_POSH_POPO_TRIGGER_INL

#include "iceoryx_posh/popo/trigger.hpp"

namespace iox
{
namespace popo
{
template <typename T, typename ContextDataType>
inline Trigger::Trigger(T* const notificationOrigin,
                        const function<bool()>& hasTriggeredCallback,
                        const function<void(uint64_t)>& resetCallback,
                        const uint64_t notificationId,
                        const NotificationCallback<T, ContextDataType>& callback,
                        const uint64_t uniqueId,
                        const TriggerType triggerType,
                        const uint64_t originTriggerType,
                        const uint64_t originTriggerTypeHash) noexcept
    : m_notificationInfo(notificationOrigin, notificationId, callback)
    , m_hasTriggeredCallback(hasTriggeredCallback)
    , m_resetCallback(resetCallback)
    , m_uniqueId(uniqueId)
    , m_triggerType(triggerType)
    , m_originTriggerType(originTriggerType)
    , m_originTriggerTypeHash(originTriggerTypeHash)
{
}

template <typename T, typename ContextDataType>
inline Trigger::Trigger(StateBasedTrigger_t,
                        T* const stateOrigin,
                        const function<bool()>& hasTriggeredCallback,
                        const function<void(uint64_t)>& resetCallback,
                        const uint64_t notificationId,
                        const NotificationCallback<T, ContextDataType>& callback,
                        const uint64_t uniqueId,
                        const uint64_t stateType,
                        const uint64_t stateTypeHash) noexcept
    : Trigger(stateOrigin,
              hasTriggeredCallback,
              resetCallback,
              notificationId,
              callback,
              uniqueId,
              TriggerType::STATE_BASED,
              stateType,
              stateTypeHash)
{
}

template <typename T, typename ContextDataType>
inline Trigger::Trigger(EventBasedTrigger_t,
                        T* const notificationOrigin,
                        const function<void(uint64_t)>& resetCallback,
                        const uint64_t notificationId,
                        const NotificationCallback<T, ContextDataType>& callback,
                        const uint64_t uniqueId,
                        const uint64_t notificationType,
                        const uint64_t notificationTypeHash) noexcept
    : Trigger(
        notificationOrigin,
        []() { return false; },
        resetCallback,
        notificationId,
        callback,
        uniqueId,
        TriggerType::EVENT_BASED,
        notificationType,
        notificationTypeHash)
{
}
} // namespace popo
} // namespace iox

#endif
