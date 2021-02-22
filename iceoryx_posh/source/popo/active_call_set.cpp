// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/popo/active_call_set.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/event_listener.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/event_notifier.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"

namespace iox
{
namespace popo
{
ActiveCallSet::ActiveCallSet() noexcept
    : ActiveCallSet(runtime::PoshRuntime::getInstance().getMiddlewareEventVariable())
{
}

ActiveCallSet::ActiveCallSet(EventVariableData* eventVariable) noexcept
    : m_eventVariable(eventVariable)
    , m_eventListener(*eventVariable)
{
    m_thread = std::thread(&ActiveCallSet::threadLoop, this);
}

ActiveCallSet::~ActiveCallSet()
{
    m_wasDtorCalled.store(true, std::memory_order_relaxed);
    m_eventListener.destroy();

    m_thread.join();
    m_eventVariable->m_toBeDestroyed.store(true, std::memory_order_relaxed);
}

cxx::expected<uint32_t, ActiveCallSetError>
ActiveCallSet::addEvent(void* const origin,
                        const uint64_t eventType,
                        const uint64_t eventTypeHash,
                        CallbackRef_t<void> callback,
                        TranslationCallbackRef_t translationCallback,
                        const cxx::MethodCallback<void, uint64_t> invalidationCallback) noexcept
{
    std::lock_guard<std::mutex> lock(m_addEventMutex);

    for (uint32_t i = 0U; i < MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET; ++i)
    {
        if (m_events[i]->isEqualTo(origin, eventType, eventTypeHash))
        {
            return cxx::error<ActiveCallSetError>(ActiveCallSetError::EVENT_ALREADY_ATTACHED);
        }
    }

    uint32_t index = 0U;
    if (!m_indexManager.pop(index))
    {
        return cxx::error<ActiveCallSetError>(ActiveCallSetError::ACTIVE_CALL_SET_FULL);
    }

    m_events[index]->init(index, origin, eventType, eventTypeHash, callback, translationCallback, invalidationCallback);
    return cxx::success<uint32_t>(index);
}

void ActiveCallSet::removeEvent(void* const origin, const uint64_t eventType, const uint64_t eventTypeHash) noexcept
{
    for (uint32_t index = 0U; index < MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET; ++index)
    {
        if (m_events[index]->resetIfEqualTo(origin, eventType, eventTypeHash))
        {
            m_indexManager.push(index);
            break;
        }
    }
}

uint64_t ActiveCallSet::size() const noexcept
{
    return m_indexManager.indicesInUse();
}

void ActiveCallSet::threadLoop() noexcept
{
    while (m_wasDtorCalled.load(std::memory_order_relaxed) == false)
    {
        auto activateNotificationIds = m_eventListener.wait();

        cxx::forEach(activateNotificationIds, [this](auto id) { m_events[id]->executeCallback(); });
    }
}

void ActiveCallSet::removeTrigger(const uint64_t index) noexcept
{
    if (index >= MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET)
    {
        return;
    }

    if (m_events[index]->reset())
    {
        m_indexManager.push(static_cast<uint32_t>(index));
    }
}

////////////////
// Event_t
////////////////
ActiveCallSet::Event_t::~Event_t()
{
    reset();
}

void ActiveCallSet::Event_t::executeCallback() noexcept
{
    if (!isInitialized())
    {
        return;
    }

    m_translationCallback(m_origin, m_callback);
}

void ActiveCallSet::Event_t::init(const uint64_t eventId,
                                  void* const origin,
                                  const uint64_t eventType,
                                  const uint64_t eventTypeHash,
                                  CallbackRef_t<void> callback,
                                  TranslationCallbackRef_t translationCallback,
                                  const cxx::MethodCallback<void, uint64_t> invalidationCallback) noexcept
{
    m_eventId = eventId;
    m_origin = origin;
    m_eventType = eventType;
    m_eventTypeHash = eventTypeHash;
    m_callback = &callback;
    m_translationCallback = &translationCallback;
    m_invalidationCallback = invalidationCallback;
}

bool ActiveCallSet::Event_t::isEqualTo(const void* const origin,
                                       const uint64_t eventType,
                                       const uint64_t eventTypeHash) const noexcept
{
    return (m_origin == origin && m_eventType == eventType && m_eventTypeHash == eventTypeHash);
}

bool ActiveCallSet::Event_t::resetIfEqualTo(const void* const origin,
                                            const uint64_t eventType,
                                            const uint64_t eventTypeHash) noexcept
{
    return (isEqualTo(origin, eventType, eventTypeHash)) ? reset() : false;
}

bool ActiveCallSet::Event_t::reset() noexcept
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
        m_invalidationCallback = cxx::MethodCallback<void, uint64_t>();

        return true;
    }
    return false;
}

bool ActiveCallSet::Event_t::isInitialized() const noexcept
{
    return m_origin != nullptr && m_eventId != INVALID_ID && m_eventType != INVALID_ID && m_eventTypeHash != INVALID_ID
           && m_callback != nullptr && m_translationCallback != nullptr
           && m_invalidationCallback != cxx::MethodCallback<void, uint64_t>();
}

//////////////////
// IndexManager_t
//////////////////

ActiveCallSet::IndexManager_t::IndexManager_t() noexcept
{
    m_loffli.init(m_loffliStorage, MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);
}

bool ActiveCallSet::IndexManager_t::pop(uint32_t& value) noexcept
{
    if (m_loffli.pop(value))
    {
        ++m_indicesInUse;
        return true;
    }
    return false;
}

void ActiveCallSet::IndexManager_t::push(const uint32_t index) noexcept
{
    m_loffli.push(index);
    --m_indicesInUse;
}

uint64_t ActiveCallSet::IndexManager_t::indicesInUse() const noexcept
{
    return m_indicesInUse.load(std::memory_order_relaxed);
}


} // namespace popo
} // namespace iox
