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

#include "iceoryx_posh/internal/popo/trigger.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_signaler.hpp"

namespace iox
{
namespace popo
{
const uint64_t& TriggerState::getTriggerId() const noexcept
{
    return m_triggerId;
}

void TriggerState::operator()() const noexcept
{
    if (m_origin != nullptr && m_callbackPtr != nullptr)
    {
        m_callback(m_origin, m_callbackPtr);
    }
}

Trigger::Trigger(const Trigger& other, const cxx::MethodCallback<void, Trigger&>& removalCallback) noexcept
    : Trigger(other.m_origin,
              other.m_conditionVariableDataPtr,
              other.m_hasTriggeredCallback,
              other.m_invalidationCallback,
              other.getTriggerId(),
              other.m_callbackPtr)
{
    m_originTypeHash = other.m_originTypeHash;
    m_callbackPtr = other.m_callbackPtr;
    m_callback = other.m_callback;
    m_removalCallback = removalCallback;
}

Trigger::~Trigger()
{
    reset();
}

bool Trigger::hasTriggered() const noexcept
{
    return (isValid() && m_hasTriggeredCallback) ? m_hasTriggeredCallback() : false;
}

void Trigger::invalidate() noexcept
{
    if (isValid() && m_invalidationCallback)
    {
        m_conditionVariableDataPtr = nullptr;
        m_invalidationCallback();
    }
}

void Trigger::reset() noexcept
{
    if (isValid())
    {
        invalidate();
        if (m_removalCallback)
        {
            m_removalCallback(*this);
        }
    }
}

void Trigger::trigger() noexcept
{
    if (m_conditionVariableDataPtr)
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
    return m_conditionVariableDataPtr != nullptr;
}

ConditionVariableData* Trigger::getConditionVariableData() noexcept
{
    return m_conditionVariableDataPtr;
}

bool Trigger::operator==(const Trigger& rhs) const noexcept
{
    return (m_hasTriggeredCallback == rhs.m_hasTriggeredCallback);
}

Trigger::Trigger(Trigger&& rhs) noexcept
{
    *this = std::move(rhs);
}

Trigger& Trigger::operator=(Trigger&& rhs) noexcept
{
    if (this != &rhs)
    {
        m_origin = rhs.m_origin;
        m_originTypeHash = rhs.m_originTypeHash;
        m_triggerId = rhs.m_triggerId;
        m_callbackPtr = rhs.m_callbackPtr;
        m_callback = rhs.m_callback;

        m_conditionVariableDataPtr = rhs.m_conditionVariableDataPtr;
        m_removalCallback = rhs.m_removalCallback;
        m_invalidationCallback = rhs.m_invalidationCallback;
        m_hasTriggeredCallback = rhs.m_hasTriggeredCallback;

        rhs.m_conditionVariableDataPtr = nullptr;
    }
    return *this;
}

} // namespace popo
} // namespace iox
