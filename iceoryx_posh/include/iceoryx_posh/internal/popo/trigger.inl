// Copyright (c) 2020, 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_TRIGGER_INL
#define IOX_POSH_POPO_TRIGGER_INL

#include <type_traits>
namespace iox
{
namespace popo
{
template <typename T>
inline Trigger::Trigger(T* const eventOrigin,
                        const cxx::ConstMethodCallback<bool>& hasTriggeredCallback,
                        const cxx::MethodCallback<void, uint64_t>& resetCallback,
                        const uint64_t eventId,
                        const Callback<T> callback) noexcept
    : m_eventInfo(eventOrigin, eventId, callback)
    , m_hasTriggeredCallback(hasTriggeredCallback)
    , m_resetCallback(resetCallback)
    , m_uniqueId(uniqueIdCounter.fetch_add(1U))
{
}

template <typename T>
inline void Trigger::updateOrigin(T* const newOrigin) noexcept
{
    if (newOrigin != m_eventInfo.m_eventOrigin)
    {
        if (m_hasTriggeredCallback && m_hasTriggeredCallback.getObjectPointer<T>() == m_eventInfo.m_eventOrigin)
        {
            m_hasTriggeredCallback.setCallback(*newOrigin, m_hasTriggeredCallback.getMethodPointer<T>());
        }

        if (m_resetCallback && m_resetCallback.getObjectPointer<T>() == m_eventInfo.m_eventOrigin)
        {
            m_resetCallback.setCallback(*newOrigin, m_resetCallback.getMethodPointer<T>());
        }

        m_eventInfo.m_eventOrigin = newOrigin;
    }
}


} // namespace popo
} // namespace iox

#endif
