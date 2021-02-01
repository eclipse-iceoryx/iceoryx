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
    : m_eventVariable(runtime::PoshRuntime::getInstance().getMiddlewareEventVariable())
{
    m_indexManager.init(m_loffliStorage, MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET);
}

ActiveCallSet::~ActiveCallSet()
{
    m_wasDtorCalled.store(true, std::memory_order_relaxed);

    // notify eventVariable without origin to signal that we are in the dtor
    m_eventVariable->m_semaphore.post();

    m_eventVariable->m_toBeDestroyed.store(true, std::memory_order_relaxed);
}

void ActiveCallSet::addEvent(void* const origin,
                             const uint64_t eventType,
                             const Callback_t<void> callback,
                             const TranslationCallback_t translationCallback) noexcept
{
    uint32_t index = 0U;
    if (!m_indexManager.pop(index))
    {
        return;
    }

    m_events[index]->init(origin, eventType, callback, translationCallback);
}

void ActiveCallSet::removeEvent(void* const origin, const uint64_t eventType) noexcept
{
    for (uint32_t index = 0U; index < MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET; ++index)
    {
        if (m_events[index]->resetIfEqualTo(origin, eventType))
        {
            m_indexManager.push(index);
            break;
        }
    }
}

void ActiveCallSet::threadLoop() noexcept
{
    EventListener eventListener(*m_eventVariable);
    while (m_wasDtorCalled.load(std::memory_order_relaxed) == false)
    {
        auto activateNotificationIds = eventListener.wait();

        cxx::forEach(activateNotificationIds, [this](auto id) { m_events[id]->executeCallback(); });
    }
}

////////////////
// Event_t
////////////////
void ActiveCallSet::Event_t::executeCallback() noexcept
{
    if (!isInitialized())
    {
        return;
    }

    m_translationCallback(m_origin, m_callback);
}

void ActiveCallSet::Event_t::init(void* const origin,
                                  const uint64_t eventType,
                                  const Callback_t<void> callback,
                                  const TranslationCallback_t translationCallback) noexcept
{
    m_origin = origin;
    m_eventType = eventType;
    m_callback = callback;
    m_translationCallback = translationCallback;
}

bool ActiveCallSet::Event_t::resetIfEqualTo(const void* const origin, const uint64_t eventType) noexcept
{
    if (m_origin == origin && m_eventType == eventType)
    {
        m_origin = nullptr;
        m_eventType = 0U;
        m_callback = nullptr;
        m_translationCallback = nullptr;
        return true;
    }
    return false;
}

bool ActiveCallSet::Event_t::isInitialized() const noexcept
{
    return m_origin != nullptr;
}

} // namespace popo
} // namespace iox
