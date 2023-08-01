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
template <uint64_t Capacity>
template <typename T, typename ContextDataType>
inline expected<void, ListenerError>
ListenerImpl<Capacity>::attachEvent(T& eventOrigin,
                                    const NotificationCallback<T, ContextDataType>& eventCallback) noexcept
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
                eventOrigin,
                TriggerHandle(*m_conditionVariableData, {*this, &ListenerImpl<Capacity>::removeTrigger}, eventId));
        });
}

template <uint64_t Capacity>
template <typename T, typename EventType, typename ContextDataType, typename>
inline expected<void, ListenerError> ListenerImpl<Capacity>::attachEvent(
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
                TriggerHandle(*m_conditionVariableData, {*this, &ListenerImpl<Capacity>::removeTrigger}, eventId),
                eventType);
        });
}

template <uint64_t Capacity>
template <typename T, typename EventType, typename>
inline void ListenerImpl<Capacity>::detachEvent(T& eventOrigin, const EventType eventType) noexcept
{
    static_assert(IS_EVENT_ENUM<EventType>,
                  "Only enums with an underlying EventEnumIdentifier can be attached/detached to the Listener");
    NotificationAttorney::disableEvent(eventOrigin, eventType);
}

template <uint64_t Capacity>
template <typename T>
inline void ListenerImpl<Capacity>::detachEvent(T& eventOrigin) noexcept
{
    NotificationAttorney::disableEvent(eventOrigin);
}

template <uint64_t Capacity>
inline constexpr uint64_t ListenerImpl<Capacity>::capacity() noexcept
{
    return Capacity;
}

template <uint64_t Capacity>
inline ListenerImpl<Capacity>::ListenerImpl() noexcept
    : ListenerImpl(*runtime::PoshRuntime::getInstance().getMiddlewareConditionVariable())
{
}

template <uint64_t Capacity>
inline ListenerImpl<Capacity>::ListenerImpl(ConditionVariableData& conditionVariable) noexcept
    : m_conditionVariableData(&conditionVariable)
    , m_conditionListener(conditionVariable)
{
    m_thread = std::thread(&ListenerImpl<Capacity>::threadLoop, this);
}

template <uint64_t Capacity>
inline ListenerImpl<Capacity>::~ListenerImpl() noexcept
{
    m_wasDtorCalled.store(true, std::memory_order_relaxed);
    m_conditionListener.destroy();

    m_thread.join();
    m_conditionVariableData->m_toBeDestroyed.store(true, std::memory_order_relaxed);
}

template <uint64_t Capacity>
inline expected<uint32_t, ListenerError>
ListenerImpl<Capacity>::addEvent(void* const origin,
                                 void* const userType,
                                 const uint64_t eventType,
                                 const uint64_t eventTypeHash,
                                 internal::GenericCallbackRef_t callback,
                                 internal::TranslationCallbackRef_t translationCallback,
                                 const function<void(uint64_t)> invalidationCallback) noexcept
{
    std::lock_guard<std::mutex> lock(m_addEventMutex);

    for (uint32_t i = 0U; i < MAX_NUMBER_OF_EVENTS_PER_LISTENER; ++i)
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

template <uint64_t Capacity>
inline uint64_t ListenerImpl<Capacity>::size() const noexcept
{
    return m_indexManager.indicesInUse();
}

template <uint64_t Capacity>
inline void ListenerImpl<Capacity>::threadLoop() noexcept
{
    while (m_wasDtorCalled.load(std::memory_order_relaxed) == false)
    {
        auto activateNotificationIds = m_conditionListener.wait();

        for (auto& id : activateNotificationIds)
        {
            m_events[id]->executeCallback();
        }
    }
}

template <uint64_t Capacity>
inline void ListenerImpl<Capacity>::removeTrigger(const uint64_t index) noexcept
{
    if (index >= MAX_NUMBER_OF_EVENTS_PER_LISTENER)
    {
        return;
    }

    if (m_events[index]->reset())
    {
        m_indexManager.push(static_cast<uint32_t>(index));
    }
}

///////////////////////
// BEGIN IndexManager_t
///////////////////////
template <uint64_t Capacity>
inline ListenerImpl<Capacity>::IndexManager_t::IndexManager_t() noexcept
{
    m_loffli.init(m_loffliStorage, MAX_NUMBER_OF_EVENTS_PER_LISTENER);
}

template <uint64_t Capacity>
inline bool ListenerImpl<Capacity>::IndexManager_t::pop(uint32_t& value) noexcept
{
    if (m_loffli.pop(value))
    {
        ++m_indicesInUse;
        return true;
    }
    return false;
}

template <uint64_t Capacity>
inline void ListenerImpl<Capacity>::IndexManager_t::push(const uint32_t index) noexcept
{
    cxx::Expects(m_loffli.push(index));
    --m_indicesInUse;
}

template <uint64_t Capacity>
uint64_t ListenerImpl<Capacity>::IndexManager_t::indicesInUse() const noexcept
{
    return m_indicesInUse.load(std::memory_order_relaxed);
}
/////////////////////
// END IndexManager_t
/////////////////////
} // namespace popo
} // namespace iox
#endif
