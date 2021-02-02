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

#ifndef IOX_POSH_POPO_ACTIVE_CALL_SET_INL
#define IOX_POSH_POPO_ACTIVE_CALL_SET_INL
namespace iox
{
namespace popo
{
namespace internal
{
template <typename T>
inline void callsetCallback(void* const origin, void (*underlyingCallback)(void* const))
{
    reinterpret_cast<void (*)(T* const)>(underlyingCallback)(static_cast<T*>(origin));
}
} // namespace internal

template <typename T>
inline cxx::expected<ActiveCallSetError> ActiveCallSet::attachEvent(T& eventOrigin,
                                                                    CallbackRef_t<T> eventCallback) noexcept
{
    return addEvent(&eventOrigin,
                    static_cast<uint64_t>(NoEnumUsed::PLACEHOLDER),
                    typeid(NoEnumUsed).hash_code(),
                    reinterpret_cast<CallbackRef_t<void>>(eventCallback),
                    internal::callsetCallback<T>,
                    EventAttorney::getInvalidateTriggerMethod(eventOrigin))
        .and_then([&](auto& eventId) {
            EventAttorney::enableEvent(eventOrigin,
                                       TriggerHandle(m_eventVariable, {*this, &ActiveCallSet::removeTrigger}, eventId));
        });
}

template <typename T, typename EventType, typename = std::enable_if_t<std::is_enum<EventType>::value>>
inline cxx::expected<ActiveCallSetError>
ActiveCallSet::attachEvent(T& eventOrigin, const EventType eventType, CallbackRef_t<T> eventCallback) noexcept
{
    return addEvent(&eventOrigin,
                    static_cast<uint64_t>(eventType),
                    typeid(EventType).hash_code(),
                    reinterpret_cast<CallbackRef_t<void>>(eventCallback),
                    internal::callsetCallback<T>,
                    EventAttorney::getInvalidateTriggerMethod(eventOrigin))
        .and_then([&](auto& eventId) {
            EventAttorney::enableEvent(eventOrigin,
                                       TriggerHandle(m_eventVariable, {*this, &ActiveCallSet::removeTrigger}, eventId),
                                       eventType);
        });
}

template <typename T, typename EventType, typename = std::enable_if_t<std::is_enum<EventType>::value>>
inline void ActiveCallSet::detachEvent(T& eventOrigin, const EventType eventType) noexcept
{
    removeEvent(&eventOrigin, static_cast<uint64_t>(eventType), typeid(EventType).hash_code());
}

template <typename T>
inline void ActiveCallSet::detachEvent(T& eventOrigin) noexcept
{
    detachEvent(eventOrigin, NoEnumUsed::PLACEHOLDER);
}


} // namespace popo
} // namespace iox
#endif
