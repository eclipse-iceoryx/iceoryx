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

#ifndef IOX_POSH_POPO_TRIGGER_INL
#define IOX_POSH_POPO_TRIGGER_INL

namespace iox
{
namespace popo
{
template <typename T>
inline void myCallback(void* const origin, void (*callbackPtr)(void* const)) noexcept
{
    (*reinterpret_cast<void (*)(T* const)>(callbackPtr))(reinterpret_cast<T*>(origin));
}

template <typename T>
inline TriggerState::TriggerState(T* const origin, const uint64_t triggerId, void (*callback)(T* const)) noexcept
    : m_origin(origin)
    , m_triggerId(triggerId)
    , m_callbackPtr(reinterpret_cast<void (*)(void* const)>(callback))
    , m_callback(myCallback<T>)
{
}

template <typename T>
inline bool TriggerState::doesOriginateFrom(T* const origin) const noexcept
{
    return m_origin == origin;
}

template <typename T>
inline Trigger::Trigger(T* const origin,
                        ConditionVariableData* conditionVariableDataPtr,
                        const cxx::ConstMethodCallback<bool>& hasTriggeredCallback,
                        const cxx::MethodCallback<void>& invalidationCallback,
                        const uint64_t triggerId,
                        void (*callback)(T* const)) noexcept
    : TriggerState(origin, triggerId, callback)
    , m_conditionVariableDataPtr(conditionVariableDataPtr)
    , m_hasTriggeredCallback(hasTriggeredCallback)
    , m_invalidationCallback(invalidationCallback)
{
}


} // namespace popo
} // namespace iox

#endif
