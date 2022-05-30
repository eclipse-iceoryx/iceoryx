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
inline EventSubscriber<T>::EventSubscriber(const core::String& service,
                                                                   const core::String& instance,
                                                                   const core::String& event) noexcept
    : m_subscriber({service, instance, event},
                   {QUEUE_CAPACITY, HISTORY_REQUEST, iox::NodeName_t(), NOT_OFFERED_ON_CREATE})
{
}

template <typename T>
inline void EventSubscriber<T>::Subscribe(std::size_t) noexcept
{
    // maxSampleCount is ignored, because it is an argument to the c'tor of m_subscriber as part of SubscriberOptions.
    // Utilizing late initalization by wrapping m_subscriber in an cxx::optional and calling the c'tor here would be an
    // option
    m_subscriber.subscribe();
}

template <typename T>
inline void EventSubscriber<T>::Unsubscribe() noexcept
{
    m_subscriber.unsubscribe();
}

template <typename T>
template <typename Callable>
inline core::Result<size_t>
EventSubscriber<T>::GetNewSamples(Callable&& callable, size_t maxNumberOfSamples) noexcept
{
    IOX_DISCARD_RESULT(maxNumberOfSamples);

    core::Result<size_t> numberOfSamples{0};

    while (m_subscriber.take()
               .and_then([&](const auto& sample) {
                   callable(sample.get());
                   numberOfSamples++;
               })
               .or_else([](auto& result) {
                   if (result != iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)
                   {
                       std::cerr << "Error receiving chunk!" << std::endl;
                   }
               }))
    {
    }
    return numberOfSamples;
}

//! [EventSubscriber setReceiveHandler]
template <typename T>
inline void EventSubscriber<T>::SetReceiveHandler(EventReceiveHandler handler) noexcept
{
    std::lock_guard<iox::posix::mutex> guard(m_mutex);
    m_listener
        .attachEvent(m_subscriber,
                     iox::popo::SubscriberEvent::DATA_RECEIVED,
                     iox::popo::createNotificationCallback(onSampleReceivedCallback, *this))
        .or_else([](auto) {
            std::cerr << "unable to attach subscriber" << std::endl;
            std::exit(EXIT_FAILURE);
        });
    m_receiveHandler.emplace(handler);
}
//! [EventSubscriber setReceiveHandler]

template <typename T>
inline void EventSubscriber<T>::UnsetReceiveHandler() noexcept
{
    std::lock_guard<iox::posix::mutex> guard(m_mutex);
    m_listener.detachEvent(m_subscriber, iox::popo::SubscriberEvent::DATA_RECEIVED);
    m_receiveHandler.reset();
}

template <typename T>
inline bool EventSubscriber<T>::HasReceiveHandler() noexcept
{
    std::lock_guard<iox::posix::mutex> guard(m_mutex);
    return m_receiveHandler.has_value();
}

//! [EventSubscriber invoke callback]
template <typename T>
inline void EventSubscriber<T>::onSampleReceivedCallback(iox::popo::Subscriber<T>*,
                                                                                 EventSubscriber* self) noexcept
{
    std::lock_guard<iox::posix::mutex> guard(self->m_mutex);
    self->m_receiveHandler.and_then([](iox::cxx::function<void()>& userCallable) { userCallable(); });
}
//! [EventSubscriber invoke callback]

} // namespace kom
} // namespace owl

#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_EVENT_SUBSCRIBER_INL
