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

#ifndef IOX_POSH_POPO_TRIGGER_STATE_INL
#define IOX_POSH_POPO_TRIGGER_STATE_INL

namespace iox
{
namespace popo
{
template <typename T>
inline void myCallback(void* const origin, TriggerState::Callback<void> callbackPtr) noexcept
{
    (*reinterpret_cast<TriggerState::Callback<T>>(callbackPtr))(reinterpret_cast<T*>(origin));
}

template <typename T>
inline TriggerState::TriggerState(T* const origin, const uint64_t triggerId, const Callback<T> callback) noexcept
    : m_origin(origin)
    , m_originTypeHash(typeid(T).hash_code())
    , m_triggerId(triggerId)
    , m_callbackPtr(reinterpret_cast<Callback<void>>(callback))
    , m_callback(myCallback<T>)
{
}

template <typename T>
inline bool TriggerState::doesOriginateFrom(T* const origin) const noexcept
{
    if (m_origin == nullptr)
    {
        return false;
    }
    return m_origin == origin;
}

template <typename T>
inline T* TriggerState::getOrigin() noexcept
{
    if (m_originTypeHash != typeid(T).hash_code())
    {
        errorHandler(Error::kPOPO__TRIGGER_STATE_TYPE_INCONSISTENCY_IN_GET_ORIGIN, nullptr, iox::ErrorLevel::MODERATE);
        return nullptr;
    }

    return static_cast<T*>(m_origin);
}

template <typename T>
inline const T* TriggerState::getOrigin() const noexcept
{
    return const_cast<const T*>(const_cast<TriggerState*>(this)->getOrigin<T>());
}
} // namespace popo
} // namespace iox

#endif
