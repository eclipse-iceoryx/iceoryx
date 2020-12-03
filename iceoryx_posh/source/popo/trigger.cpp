// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/popo/trigger.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_signaler.hpp"

namespace iox
{
namespace popo
{
std::atomic<uint64_t> Trigger::uniqueIdCounter{0U};

Trigger::~Trigger()
{
    reset();
}

bool Trigger::hasTriggered() const noexcept
{
    return (isValid() && m_hasTriggeredCallback) ? m_hasTriggeredCallback().value() : false;
}

void Trigger::reset() noexcept
{
    if (!isValid())
    {
        return;
    }

    if (m_resetCallback)
    {
        m_resetCallback(*this);
    }

    invalidate();
}

void Trigger::invalidate() noexcept
{
    m_conditionVariableDataPtr = nullptr;
    m_origin = nullptr;
}

void Trigger::trigger() noexcept
{
    if (isValid())
    {
        ConditionVariableSignaler(m_conditionVariableDataPtr).notifyOne();
    }
}

Trigger::operator bool() const noexcept
{
    return isValid();
}

bool Trigger::isValid() const noexcept
{
    return m_origin != nullptr && m_conditionVariableDataPtr != nullptr && m_hasTriggeredCallback;
}

ConditionVariableData* Trigger::getConditionVariableData() noexcept
{
    return m_conditionVariableDataPtr;
}

bool Trigger::isLogicalEqualTo(const Trigger& rhs) const noexcept
{
    return (isValid() && rhs.isValid() && m_origin == rhs.m_origin
            && m_hasTriggeredCallback == rhs.m_hasTriggeredCallback
            && m_conditionVariableDataPtr == rhs.m_conditionVariableDataPtr && m_triggerId == rhs.m_triggerId);
}

Trigger::Trigger(Trigger&& rhs) noexcept
{
    *this = std::move(rhs);
}

Trigger& Trigger::operator=(Trigger&& rhs) noexcept
{
    if (this != &rhs)
    {
        reset();

        // TriggerState
        m_origin = rhs.m_origin;
        m_originTypeHash = rhs.m_originTypeHash;
        m_triggerId = rhs.m_triggerId;
        m_callbackPtr = rhs.m_callbackPtr;
        m_callback = rhs.m_callback;

        // Trigger
        m_conditionVariableDataPtr = rhs.m_conditionVariableDataPtr;
        m_resetCallback = rhs.m_resetCallback;
        m_hasTriggeredCallback = rhs.m_hasTriggeredCallback;

        rhs.m_origin = nullptr;
        rhs.m_originTypeHash = 0U;
        rhs.m_triggerId = Trigger::INVALID_TRIGGER_ID;
        rhs.m_callbackPtr = decltype(m_callbackPtr)();
        rhs.m_callback = decltype(m_callback)();

        rhs.m_conditionVariableDataPtr = nullptr;
        rhs.m_resetCallback = decltype(m_resetCallback)();
        rhs.m_hasTriggeredCallback = decltype(m_hasTriggeredCallback)();
    }
    return *this;
}

uint64_t Trigger::getUniqueId() const noexcept
{
    return m_uniqueId;
}


} // namespace popo
} // namespace iox
