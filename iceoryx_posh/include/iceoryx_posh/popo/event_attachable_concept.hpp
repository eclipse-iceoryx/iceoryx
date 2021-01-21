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

    static_assert(HasDisableEvent<T>::value && HasEnableEvent<T>::value && HasGetHasTriggeredCallbackForEvent<T>::value
                      && HasInvalidateTrigger<T>::value,
                  "\n\n  EventAttachables are not allowed to be copy- or movable and "
                  "they are requiring the following methods:\n"
                  "    void T::disableEvent(EventType)\n "
                  "    void T::enableEvent(iox::popo::TriggerHandle&&, EventType)\n "
                  "    void T::invalidateTrigger(uint64_t)\n "
                  "    WaitSetHasTriggeredCallback "
                  "T::getHasTriggeredCallbackForEvent(EventType)\n\n");

    using verify = bool;
};


} // namespace popo
} // namespace iox

#endif
