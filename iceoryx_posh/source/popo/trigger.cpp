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

namespace iox
{
namespace popo
{
Trigger::Trigger(const cxx::ConstMethodCallback<bool>& hasTriggeredCallback,
                 const cxx::MethodCallback<void>& invalidationCallback,
                 ConditionVariableData* conditionVariableDataPtr,
                 const uint64_t classId) noexcept
    : m_conditionVariableDataPtr(conditionVariableDataPtr)
    , m_invalidationCallback(invalidationCallback)
    , m_hasTriggeredCallback(hasTriggeredCallback)
    , m_triggerId(classId)
{
    getTriggerId();
}

Trigger::Trigger(const Trigger& other, const cxx::MethodCallback<void, Trigger&>& removalCallback) noexcept
    : Trigger(other.m_hasTriggeredCallback,
              other.m_invalidationCallback,
              other.m_conditionVariableDataPtr,
              other.getTriggerId().getClassId())
{
    m_triggerId = other.m_triggerId;
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
        m_invalidationCallback();
    }
}

void Trigger::reset() noexcept
{
    invalidate();
    if (isValid() && m_removalCallback)
    {
        m_removalCallback(*this);
    }
    m_conditionVariableDataPtr = nullptr;
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

const TriggerId& Trigger::getTriggerId() const noexcept
{
    return m_triggerId;
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
        m_conditionVariableDataPtr = rhs.m_conditionVariableDataPtr;
        m_removalCallback = rhs.m_removalCallback;
        m_invalidationCallback = rhs.m_invalidationCallback;
        m_hasTriggeredCallback = rhs.m_hasTriggeredCallback;
        m_triggerId = rhs.m_triggerId;

        rhs.m_conditionVariableDataPtr = nullptr;
    }
    return *this;
}

} // namespace popo
} // namespace iox
