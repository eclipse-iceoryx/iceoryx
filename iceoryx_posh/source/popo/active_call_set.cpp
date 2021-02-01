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

void ActiveCallSet::addEvent(void* const origin, const uint64_t eventType, const Callback_t<void> callback) noexcept
{
    uint32_t index = 0U;
    if (!m_indexManager.pop(index))
    {
        return;
    }

    m_events[index].init(m_indexManager, index, origin, eventType, callback);
}

void ActiveCallSet::removeEvent(void* const origin, const uint64_t eventType) noexcept
{
    for (uint32_t index = 0U; index < MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET; ++index)
    {
        if (!m_events[index].isEqualTo(origin, eventType))
        {
            continue;
        }

        m_events[index].toBeDeleted();

        break;
    }
}

void ActiveCallSet::threadLoop() noexcept
{
    EventListener eventListener(*m_eventVariable);
    while (m_wasDtorCalled.load(std::memory_order_relaxed) == false)
    {
        auto activateNotificationIds = eventListener.wait();

        cxx::forEach(activateNotificationIds, [this](auto id) { m_events[id](); });
    }
}

////////////////
// Event_t
////////////////
void ActiveCallSet::Event_t::operator()() noexcept
{
    CallbackState expectedState = CallbackState::INACTIVE;

    if (m_callbackState.compare_exchange_strong(
            expectedState, CallbackState::ACTIVE, std::memory_order_acq_rel, std::memory_order_relaxed))
    {
        m_callback(m_origin);

        expectedState = CallbackState::ACTIVE;
        if (!m_callbackState.compare_exchange_strong(
                expectedState, CallbackState::INACTIVE, std::memory_order_relaxed, std::memory_order_relaxed))
        {
            if (expectedState == CallbackState::TO_BE_DELETED)
            {
                m_callbackState.exchange(CallbackState::EMPTY, std::memory_order_relaxed);
                reset();
            }
            else
            {
                // FATAL logic error
            }
        }
    }
    else
    {
        // FATAL logic error if expectedState == ACTIVE, TO_BE_DELETED
    }
}

void ActiveCallSet::Event_t::toBeDeleted() noexcept
{
    CallbackState expectedState = CallbackState::INACTIVE;
    CallbackState newState = CallbackState::EMPTY;

    while (!m_callbackState.compare_exchange_strong(
        expectedState, newState, std::memory_order_relaxed, std::memory_order_relaxed))
    {
        if (expectedState == CallbackState::EMPTY || expectedState == CallbackState::TO_BE_DELETED)
        {
            return;
        }
        else if (expectedState == CallbackState::ACTIVE)
        {
            newState = CallbackState::TO_BE_DELETED;
        }
        else if (expectedState == CallbackState::INACTIVE)
        {
            newState = CallbackState::EMPTY;
        }
    }

    if (newState == CallbackState::EMPTY)
    {
        reset();
    }
}

void ActiveCallSet::Event_t::reset() noexcept
{
    set(nullptr, 0U, nullptr);
    m_indexManager->push(m_index);
    m_indexManager = nullptr;
    m_index = 0U;
}

void ActiveCallSet::Event_t::init(concurrent::LoFFLi& indexManager,
                                  const uint32_t index,
                                  void* const origin,
                                  const uint64_t eventType,
                                  const Callback_t<void> callback) noexcept
{
    m_indexManager = &indexManager;
    m_index = index;
    set(origin, eventType, callback);
    m_callbackState.store(CallbackState::INACTIVE, std::memory_order_release);
}

bool ActiveCallSet::Event_t::isEqualTo(const void* const origin, const uint64_t eventType) const noexcept
{
    auto currentCallbackState = m_callbackState.load(std::memory_order_relaxed);
    if (currentCallbackState == CallbackState::TO_BE_DELETED || currentCallbackState == CallbackState::EMPTY)
    {
        return false;
    }

    uint64_t setCounter = 0U;
    do
    {
        setCounter = m_setCounter.load(std::memory_order_acquire);
        if (m_origin != origin || m_eventType != eventType)
        {
            return false;
        }
    } while (setCounter != m_setCounter.load(std::memory_order_acquire));

    return true;
}

void ActiveCallSet::Event_t::set(void* const origin, const uint64_t eventType, const Callback_t<void> callback) noexcept
{
    m_origin = origin;
    m_eventType = eventType;
    m_callback = callback;
    m_setCounter.fetch_add(1, std::memory_order_release);
}

} // namespace popo
} // namespace iox
