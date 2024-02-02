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

#include "iceoryx_posh/popo/listener.hpp"
#include "iox/assertions.hpp"

namespace iox
{
namespace popo
{
Listener::Listener() noexcept
    : Listener(*runtime::PoshRuntime::getInstance().getMiddlewareConditionVariable())
{
}

Listener::Listener(ConditionVariableData& conditionVariable) noexcept
    : m_conditionVariableData(&conditionVariable)
    , m_conditionListener(conditionVariable)
{
    m_thread = std::thread(&Listener::threadLoop, this);
}

Listener::~Listener() noexcept
{
    m_wasDtorCalled.store(true, std::memory_order_relaxed);
    m_conditionListener.destroy();

    m_thread.join();
    m_conditionVariableData->m_toBeDestroyed.store(true, std::memory_order_relaxed);
}

uint64_t Listener::size() const noexcept
{
    return m_indexManager.indicesInUse();
}

void Listener::threadLoop() noexcept
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

void Listener::removeTrigger(const uint64_t index) noexcept
{
    if (index >= MAX_NUMBER_OF_EVENTS)
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
Listener::IndexManager_t::IndexManager_t() noexcept
{
    m_loffli.init(m_loffliStorage, MAX_NUMBER_OF_EVENTS);
}

bool Listener::IndexManager_t::pop(uint32_t& value) noexcept
{
    if (m_loffli.pop(value))
    {
        ++m_indicesInUse;
        return true;
    }
    return false;
}

void Listener::IndexManager_t::push(const uint32_t index) noexcept
{
    IOX_ENFORCE(m_loffli.push(index), "Releasing used index back to free list");
    --m_indicesInUse;
}

uint64_t Listener::IndexManager_t::indicesInUse() const noexcept
{
    return m_indicesInUse.load(std::memory_order_relaxed);
}
/////////////////////
// END IndexManager_t
/////////////////////

namespace internal
{
Event_t::~Event_t() noexcept
{
    reset();
}

void Event_t::executeCallback() noexcept
{
    if (!isInitialized())
    {
        return;
    }

    m_translationCallback(m_origin, m_userType, m_callback);
}

void Event_t::init(const uint64_t eventId,
                   void* const origin,
                   void* const userType,
                   const uint64_t eventType,
                   const uint64_t eventTypeHash,
                   internal::GenericCallbackRef_t callback,
                   internal::TranslationCallbackRef_t translationCallback,
                   const function<void(uint64_t)> invalidationCallback) noexcept
{
    m_eventId = eventId;
    m_origin = origin;
    m_userType = userType;
    m_eventType = eventType;
    m_eventTypeHash = eventTypeHash;
    m_callback = &callback;
    m_translationCallback = &translationCallback;
    m_invalidationCallback = invalidationCallback;
}

bool Event_t::isEqualTo(const void* const origin, const uint64_t eventType, const uint64_t eventTypeHash) const noexcept
{
    return (m_origin == origin && m_eventType == eventType && m_eventTypeHash == eventTypeHash);
}

bool Event_t::reset() noexcept
{
    if (isInitialized())
    {
        m_invalidationCallback(m_eventId);

        m_eventId = INVALID_ID;
        m_origin = nullptr;
        m_eventType = INVALID_ID;
        m_eventTypeHash = INVALID_ID;
        m_callback = nullptr;
        m_translationCallback = nullptr;
        m_invalidationCallback = [](auto) {};

        return true;
    }
    return false;
}

bool Event_t::isInitialized() const noexcept
{
    return m_origin != nullptr && m_eventId != INVALID_ID && m_eventType != INVALID_ID && m_eventTypeHash != INVALID_ID
           && m_callback != nullptr && m_translationCallback != nullptr;
}
} // namespace internal

} // namespace popo
} // namespace iox
