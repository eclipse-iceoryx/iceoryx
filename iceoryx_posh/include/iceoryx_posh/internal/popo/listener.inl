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

#ifndef IOX_POSH_POPO_LISTENER_INL
#define IOX_POSH_POPO_LISTENER_INL
#include "iceoryx_posh/popo/listener.hpp"

namespace iox
{
namespace popo
{
template <typename T, typename ContextDataType>
inline expected<void, ListenerError>
Listener::attachEvent(T& eventOrigin, const NotificationCallback<T, ContextDataType>& eventCallback) noexcept
{
    if (eventCallback.m_callback == nullptr)
    {
        return err(ListenerError::EMPTY_EVENT_CALLBACK);
    }

    return addEvent(&eventOrigin,
                    eventCallback.m_contextData,
                    static_cast<uint64_t>(NoEnumUsed::PLACEHOLDER),
                    typeid(NoEnumUsed).hash_code(),
                    reinterpret_cast<internal::GenericCallbackRef_t>(*eventCallback.m_callback),
                    internal::TranslateAndCallTypelessCallback<T, ContextDataType>::call,
                    NotificationAttorney::getInvalidateTriggerMethod(eventOrigin))
        .and_then([&](auto& eventId) {
            NotificationAttorney::enableEvent(
                eventOrigin, TriggerHandle(*m_conditionVariableData, {*this, &Listener::removeTrigger}, eventId));
        });
}

template <typename T, typename EventType, typename ContextDataType, typename>
inline expected<void, ListenerError> Listener::attachEvent(
    T& eventOrigin, const EventType eventType, const NotificationCallback<T, ContextDataType>& eventCallback) noexcept
{
    if (eventCallback.m_callback == nullptr)
    {
        return err(ListenerError::EMPTY_EVENT_CALLBACK);
    }

    return addEvent(&eventOrigin,
                    eventCallback.m_contextData,
                    static_cast<uint64_t>(eventType),
                    typeid(EventType).hash_code(),
                    reinterpret_cast<internal::GenericCallbackRef_t>(*eventCallback.m_callback),
                    internal::TranslateAndCallTypelessCallback<T, ContextDataType>::call,
                    NotificationAttorney::getInvalidateTriggerMethod(eventOrigin))
        .and_then([&](auto& eventId) {
            NotificationAttorney::enableEvent(
                eventOrigin,
                TriggerHandle(*m_conditionVariableData, {*this, &Listener::removeTrigger}, eventId),
                eventType);
        });
}

inline expected<uint32_t, ListenerError>
Listener::addEvent(void* const origin,
                   void* const userType,
                   const uint64_t eventType,
                   const uint64_t eventTypeHash,
                   internal::GenericCallbackRef_t callback,
                   internal::TranslationCallbackRef_t translationCallback,
                   const function<void(uint64_t)> invalidationCallback) noexcept
{
    std::lock_guard<std::mutex> lock(m_addEventMutex);

    for (uint32_t i = 0U; i < MAX_NUMBER_OF_EVENTS; ++i)
    {
        if (m_events[i]->isEqualTo(origin, eventType, eventTypeHash))
        {
            return err(ListenerError::EVENT_ALREADY_ATTACHED);
        }
    }

    uint32_t index = 0U;
    if (!m_indexManager.pop(index))
    {
        return err(ListenerError::LISTENER_FULL);
    }

    m_events[index]->init(
        index, origin, userType, eventType, eventTypeHash, callback, translationCallback, invalidationCallback);
    return ok(index);
}

template <typename T, typename EventType, typename>
inline void Listener::detachEvent(T& eventOrigin, const EventType eventType) noexcept
{
    static_assert(IS_EVENT_ENUM<EventType>,
                  "Only enums with an underlying EventEnumIdentifier can be attached/detached to the Listener");
    NotificationAttorney::disableEvent(eventOrigin, eventType);
}

template <typename T>
inline void Listener::detachEvent(T& eventOrigin) noexcept
{
    NotificationAttorney::disableEvent(eventOrigin);
}

inline constexpr uint64_t Listener::capacity() noexcept
{
    return MAX_NUMBER_OF_EVENTS;
}

} // namespace popo
} // namespace iox
#endif
