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

#ifndef IOX_POSH_POPO_EVENT_ATTACHABLE_CONCEPT_HPP
#define IOX_POSH_POPO_EVENT_ATTACHABLE_CONCEPT_HPP

#include <type_traits>

namespace iox
{
namespace popo
{
enum class REQUIRES
{
    Placeholder
};

template <typename T>
struct EventAttachableConcept
{
    template <typename C, class = void>
    struct HasDisableEvent : std::false_type
    {
    };

    template <typename C>
    struct HasDisableEvent<C, std::void_t<decltype(&C::disableEvent)>> : std::true_type
    {
    };

    template <typename C, class = void>
    struct HasEnableEvent : std::false_type
    {
    };

    template <typename C>
    struct HasEnableEvent<C, std::void_t<decltype(&C::enableEvent)>> : std::true_type
    {
    };

    template <typename C, class = void>
    struct HasGetHasTriggeredCallbackForEvent : std::false_type
    {
    };

    template <typename C>
    struct HasGetHasTriggeredCallbackForEvent<C, std::void_t<decltype(&C::getHasTriggeredCallbackForEvent)>>
        : std::true_type
    {
    };

    template <typename C, class = void>
    struct HasInvalidateTrigger : std::false_type
    {
    };

    template <typename C>
    struct HasInvalidateTrigger<C, std::void_t<decltype(&C::invalidateTrigger)>> : std::true_type
    {
    };

    static_assert(HasDisableEvent<T>::value, "Type requires void T::disableEvent(EventType) method");
    static_assert(HasEnableEvent<T>::value,
                  "Type requires void T::enableEvent(iox::popo::TriggerHandle&&, EventType) method");
    static_assert(HasGetHasTriggeredCallbackForEvent<T>::value,
                  "Type requires void T::getHasTriggeredCallbackForEvent(EventType) method");
    static_assert(HasInvalidateTrigger<T>::value, "Type requires void T::invalidateTrigger(uint64_t) method");

    static_assert(!std::is_copy_constructible<T>::value,
                  "Type is not allowed to be copy constructible. Declare T(const T&) = delete;");
    static_assert(!std::is_copy_assignable<T>::value,
                  "Type is not allowed to be copy assignable. Declare T& operator=(const T&) =delete;");
    static_assert(!std::is_move_assignable<T>::value,
                  "Type is not allowed to be move assignable. Declare T& operator=(T&&) =delete;");
    static_assert(!std::is_move_constructible<T>::value,
                  "Type is not allowed to be move constructible. Declare T(T&&) =delete;");

    static constexpr REQUIRES VALUE = REQUIRES::Placeholder;
};


template <typename T>
struct SingleEventAttachableConcept
{
    template <typename C, class = void>
    struct HasDisableEvent : std::false_type
    {
    };

    template <typename C>
    struct HasDisableEvent<C, std::void_t<decltype(&C::disableEvent)>> : std::true_type
    {
    };

    template <typename C, class = void>
    struct HasEnableEvent : std::false_type
    {
    };

    template <typename C>
    struct HasEnableEvent<C, std::void_t<decltype(&C::enableEvent)>> : std::true_type
    {
    };

    template <typename C, class = void>
    struct HasGetHasTriggeredCallbackForEvent : std::false_type
    {
    };

    template <typename C>
    struct HasGetHasTriggeredCallbackForEvent<C, std::void_t<decltype(&C::getHasTriggeredCallbackForEvent)>>
        : std::true_type
    {
    };

    template <typename C, class = void>
    struct HasInvalidateTrigger : std::false_type
    {
    };

    template <typename C>
    struct HasInvalidateTrigger<C, std::void_t<decltype(&C::invalidateTrigger)>> : std::true_type
    {
    };

    static_assert(HasDisableEvent<T>::value, "Type requires void T::disableEvent() method");
    static_assert(HasEnableEvent<T>::value, "Type requires void T::enableEvent(iox::popo::TriggerHandle&&) method");
    static_assert(HasGetHasTriggeredCallbackForEvent<T>::value,
                  "Type requires void T::getHasTriggeredCallbackForEvent() method");
    static_assert(HasInvalidateTrigger<T>::value, "Type requires void T::invalidateTrigger(uint64_t) method");

    static_assert(!std::is_copy_constructible<T>::value,
                  "Type is not allowed to be copy constructible. Declare T(const T&) = delete;");
    static_assert(!std::is_copy_assignable<T>::value,
                  "Type is not allowed to be copy assignable. Declare T& operator=(const T&) =delete;");
    static_assert(!std::is_move_assignable<T>::value,
                  "Type is not allowed to be move assignable. Declare T& operator=(T&&) =delete;");
    static_assert(!std::is_move_constructible<T>::value,
                  "Type is not allowed to be move constructible. Declare T(T&&) =delete;");

    static constexpr REQUIRES VALUE = REQUIRES::Placeholder;
};


} // namespace popo
} // namespace iox

#endif
