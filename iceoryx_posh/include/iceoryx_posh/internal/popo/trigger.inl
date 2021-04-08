// Copyright (c) 2020, 2021 by Apex.AI Inc. All rights reserved.
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

#include <type_traits>
namespace iox
{
namespace popo
{
template <typename T>
inline Trigger::Trigger(T* const eventOrigin,
                        const cxx::ConstMethodCallback<bool>& hasTriggeredCallback,
                        const cxx::MethodCallback<void, uint64_t>& resetCallback,
                        const uint64_t eventId,
                        const Callback<T> callback,
                        const uint64_t uniqueId,
                        const TriggerType triggerType,
                        const uint64_t originTriggerType,
                        const uint64_t originTriggerTypeHash) noexcept
    : m_eventInfo(eventOrigin, eventId, callback)
    , m_hasTriggeredCallback(hasTriggeredCallback)
    , m_resetCallback(resetCallback)
    , m_uniqueId(uniqueId)
    , m_triggerType(triggerType)
    , m_originTriggerType(originTriggerType)
    , m_originTriggerTypeHash(originTriggerTypeHash)
{
    if (!resetCallback)
    {
        errorHandler(Error::kPOPO__TRIGGER_INVALID_RESET_CALLBACK, nullptr, ErrorLevel::FATAL);
        invalidate();
    }
}

template <typename T>
inline Trigger::Trigger(StateBasedTrigger_t,
                        T* const stateOrigin,
                        const cxx::ConstMethodCallback<bool>& hasTriggeredCallback,
                        const cxx::MethodCallback<void, uint64_t>& resetCallback,
                        const uint64_t eventId,
                        const Callback<T> callback,
                        const uint64_t uniqueId,
                        const uint64_t stateType,
                        const uint64_t stateTypeHash) noexcept
    : Trigger(stateOrigin,
              hasTriggeredCallback,
              resetCallback,
              eventId,
              callback,
              uniqueId,
              TriggerType::STATE_BASED,
              stateType,
              stateTypeHash)
{
    if (!hasTriggeredCallback)
    {
        errorHandler(Error::kPOPO__TRIGGER_INVALID_HAS_TRIGGERED_CALLBACK, nullptr, ErrorLevel::FATAL);
        invalidate();
    }
}

template <typename T>
inline Trigger::Trigger(EventBasedTrigger_t,
                        T* const eventOrigin,
                        const cxx::MethodCallback<void, uint64_t>& resetCallback,
                        const uint64_t eventId,
                        const Callback<T> callback,
                        const uint64_t uniqueId,
                        const uint64_t eventType,
                        const uint64_t eventTypeHash) noexcept
    : Trigger(eventOrigin,
              cxx::ConstMethodCallback<bool>(),
              resetCallback,
              eventId,
              callback,
              uniqueId,
              TriggerType::EVENT_BASED,
              eventType,
              eventTypeHash)
{
}

template <typename T>
inline void Trigger::updateOrigin(T& newOrigin) noexcept
{
    if (isValid() && &newOrigin != m_eventInfo.m_eventOrigin)
    {
        if (m_hasTriggeredCallback && m_hasTriggeredCallback.getObjectPointer<T>() == m_eventInfo.m_eventOrigin)
        {
            m_hasTriggeredCallback.setCallback(newOrigin, m_hasTriggeredCallback.getMethodPointer<T>());
        }

        if (m_resetCallback && m_resetCallback.getObjectPointer<T>() == m_eventInfo.m_eventOrigin)
        {
            m_resetCallback.setCallback(newOrigin, m_resetCallback.getMethodPointer<T>());
        }

        m_eventInfo.m_eventOrigin = &newOrigin;
    }
}


} // namespace popo
} // namespace iox

#endif
