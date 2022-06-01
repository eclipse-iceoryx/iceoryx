// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_EXAMPLES_AUTOMOTIVE_SOA_EVENT_SUBSCRIBER_INL
#define IOX_EXAMPLES_AUTOMOTIVE_SOA_EVENT_SUBSCRIBER_INL

#include "owl/kom/event_subscriber.hpp"

namespace owl
{
namespace kom
{
template <typename T>
inline EventSubscriber<T>::EventSubscriber(const ServiceIdentifier& service,
                                           const InstanceIdentifier& instance,
                                           const EventIdentifier& event) noexcept
    : m_serviceDescription(service, instance, event)
{
}

template <typename T>
inline void EventSubscriber<T>::Subscribe(const uint32_t queueCapacity) noexcept
{
    if (HasReceiveCallback())
    {
        UnsetReceiveCallback();
    }

    m_subscriber.emplace(
        m_serviceDescription,
        iox::popo::SubscriberOptions{queueCapacity, HISTORY_REQUEST, iox::NodeName_t(), NOT_OFFERED_ON_CREATE});
    m_subscriber.value().subscribe();
}

template <typename T>
inline void EventSubscriber<T>::Unsubscribe() noexcept
{
    if (!m_subscriber.has_value())
    {
        return;
    }

    if (HasReceiveCallback())
    {
        UnsetReceiveCallback();
    }

    m_subscriber.value().unsubscribe();
    m_subscriber.reset();
}

template <typename T>
template <typename Callable>
inline core::Result<uint32_t> EventSubscriber<T>::TakeNewSamples(Callable&& callable, uint32_t maxNumberOfSamples) noexcept
{
    core::Result<uint32_t> numberOfSamples{0};

    if (!m_subscriber.has_value())
    {
        return numberOfSamples;
    }

    // Hint: Depending on the type of callable, it needs to be checked for null or restricting to lambdas

    while (m_subscriber.value()
               .take()
               .and_then([&](const auto& sample) {
                   callable(sample.get());
                   numberOfSamples++;
               })
               .or_else([](auto& result) {
                   if (result != iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)
                   {
                       std::cerr << "Error receiving chunk!" << std::endl;
                   }
               })
           && numberOfSamples < maxNumberOfSamples)
    {
    }
    return numberOfSamples;
}

//! [EventSubscriber setReceiveHandler]
template <typename T>
inline void EventSubscriber<T>::SetReceiveCallback(EventReceiveCallback handler) noexcept
{
    if (!handler)
    {
        std::cerr << "Can't attach empty receive callback!" << std::endl;
        return;
    }
    if (!m_subscriber.has_value())
    {
        std::cerr << "Call Subscribe() before setting a receive callback!" << std::endl;
        return;
    }

    m_listener
        .attachEvent(m_subscriber.value(),
                     iox::popo::SubscriberEvent::DATA_RECEIVED,
                     iox::popo::createNotificationCallback(onSampleReceivedCallback, *this))
        .expect("Unable to attach subscriber!");
    m_receiveCallback->emplace(handler);
}
//! [EventSubscriber setReceiveHandler]

template <typename T>
inline void EventSubscriber<T>::UnsetReceiveCallback() noexcept
{
    if (!m_subscriber.has_value())
    {
        return;
    }

    m_listener.detachEvent(m_subscriber.value(), iox::popo::SubscriberEvent::DATA_RECEIVED);
    m_receiveCallback->reset();
}

template <typename T>
inline bool EventSubscriber<T>::HasReceiveCallback() const noexcept
{
    auto receiveHandlerGuard = m_receiveCallback.getScopeGuard();
    return receiveHandlerGuard->has_value() && receiveHandlerGuard->value();
}

//! [EventSubscriber invoke callback]
template <typename T>
inline void EventSubscriber<T>::onSampleReceivedCallback(iox::popo::Subscriber<T>*, EventSubscriber* self) noexcept
{
    if (self == nullptr)
    {
        std::cerr << "Callback was invoked with EventSubscriber* being a nullptr!" << std::endl;
        return;
    }

    self->m_receiveCallback->and_then([](iox::cxx::function<void()>& userCallable) {
        if (!userCallable)
        {
            std::cerr << "Tried to call an empty receive callback!" << std::endl;
            return;
        }
        userCallable();
    });
}
//! [EventSubscriber invoke callback]

} // namespace kom
} // namespace owl

#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_EVENT_SUBSCRIBER_INL
