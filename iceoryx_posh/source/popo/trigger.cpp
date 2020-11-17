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
bool Trigger::hasTriggered() const noexcept
{
    return (m_hasTriggeredCallback) ? m_hasTriggeredCallback() : false;
}

bool Trigger::operator==(const Trigger& rhs) const noexcept
{
    return (m_condition == rhs.m_condition && m_hasTriggeredCallback == rhs.m_hasTriggeredCallback);
}

bool Trigger::operator==(const void* rhs) const noexcept
{
    return (m_condition == rhs);
}

Trigger::Trigger(Trigger&& rhs) noexcept
{
    *this = std::move(rhs);
}

Trigger& Trigger::operator=(Trigger&& rhs) noexcept
{
    if (this != &rhs)
    {
        m_condition = rhs.m_condition;
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
