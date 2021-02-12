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
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IOX_POSH_POPO_TRIGGER_INFO_INL
#define IOX_POSH_POPO_TRIGGER_INFO_INL

namespace iox
{
namespace popo
{
template <typename T>
inline void myCallback(void* const triggerOrigin, EventInfo::Callback<void> callbackPtr) noexcept
{
    (*reinterpret_cast<EventInfo::Callback<T>>(callbackPtr))(reinterpret_cast<T*>(triggerOrigin));
}

template <typename T>
inline EventInfo::EventInfo(T* const eventOrigin, const uint64_t eventId, const Callback<T> callback) noexcept
    : m_eventOrigin(eventOrigin)
    , m_eventOriginTypeHash(typeid(T).hash_code())
    , m_eventId(eventId)
    , m_callbackPtr(reinterpret_cast<Callback<void>>(callback))
    , m_callback(myCallback<T>)
{
}

template <typename T>
inline bool EventInfo::doesOriginateFrom(T* const eventOrigin) const noexcept
{
    if (m_eventOrigin == nullptr)
    {
        return false;
    }
    return m_eventOrigin == eventOrigin;
}

template <typename T>
inline T* EventInfo::getOrigin() const noexcept
{
    if (m_eventOriginTypeHash != typeid(T).hash_code())
    {
        errorHandler(Error::kPOPO__EVENT_INFO_TYPE_INCONSISTENCY_IN_GET_ORIGIN, nullptr, iox::ErrorLevel::MODERATE);
        return nullptr;
    }

    return static_cast<T*>(m_eventOrigin);
}

} // namespace popo
} // namespace iox

#endif
