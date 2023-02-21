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

namespace iox
{
namespace popo
{
Listener::Listener() noexcept
{
}

Listener::Listener(ConditionVariableData& conditionVariableData) noexcept
    : Parent(conditionVariableData)
{
}

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
