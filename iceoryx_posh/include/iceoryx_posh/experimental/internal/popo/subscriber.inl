// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_SUBSCRIBER_INL
#define IOX_EXPERIMENTAL_POSH_POPO_SUBSCRIBER_INL

#include <iostream>

namespace iox {
namespace popo {

// ======================================== Base Subscriber ======================================== //

template<typename T, typename recvport_t>
BaseSubscriber<T, recvport_t>::BaseSubscriber(const capro::ServiceDescription& service)
    : m_port(iox::runtime::PoshRuntime::getInstance().getMiddlewareReceiver(service, ""))
{}

template<typename T, typename recvport_t>
inline cxx::expected<SubscriberError>
BaseSubscriber<T, recvport_t>::subscribe(const uint64_t cacheSize) noexcept
{
    std::cout << "subscribe" << std::endl;
    return cxx::success<>();
}

template<typename T, typename recvport_t>
inline SubscriptionState
BaseSubscriber<T, recvport_t>::getSubscriptionState() const noexcept
{
    std::cout << "getSubscriptionState" << std::endl;
    return SubscriptionState::NOT_SUBSCRIBED;
}

template<typename T, typename recvport_t>
inline void
BaseSubscriber<T, recvport_t>::unsubscribe() noexcept
{
    std::cout << "unsubscribe" << std::endl;
}

template<typename T, typename recvport_t>
inline cxx::optional<cxx::unique_ptr<T>>
BaseSubscriber<T, recvport_t>::receive() noexcept
{
    std::cout << "receive" << std::endl;
    return cxx::nullopt_t();
}

// ======================================== Typed Subscriber ======================================== //

template<typename T>
TypedSubscriber<T>::TypedSubscriber(const capro::ServiceDescription& service)
    : BaseSubscriber<T>(service)
{}

template<typename T>
inline cxx::expected<SubscriberError>
TypedSubscriber<T>::subscribe(const uint64_t cacheSize) noexcept
{
    return BaseSubscriber<T>::subscribe(cacheSize);
}

template<typename T>
inline SubscriptionState
TypedSubscriber<T>::getSubscriptionState() const noexcept
{
    return BaseSubscriber<T>::getSubscriptionState();
}

template<typename T>
inline void
TypedSubscriber<T>::unsubscribe() noexcept
{
    return BaseSubscriber<T>::unsubscribe();
}

template<typename T>
inline cxx::optional<cxx::unique_ptr<T>>
TypedSubscriber<T>::receive() noexcept
{
    return BaseSubscriber<T>::receive();
}

// ======================================== Untyped Subscriber ======================================== //


} // namespace popo
} // namespace iox

#endif // IOX_EXPERIMENTAL_POSH_POPO_SUBSCRIBER_INL
