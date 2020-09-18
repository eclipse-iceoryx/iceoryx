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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_TYPED_SUBSCRIBER_INL
#define IOX_EXPERIMENTAL_POSH_POPO_TYPED_SUBSCRIBER_INL

namespace iox
{
namespace popo
{
template <typename T>
TypedSubscriber<T>::TypedSubscriber(const capro::ServiceDescription& service)
    : BaseSubscriber<T>(service)
{
}

template <typename T>
inline void TypedSubscriber<T>::subscribe(const uint64_t queueCapacity) noexcept
{
    BaseSubscriber<T>::subscribe(queueCapacity);
}

template <typename T>
inline SubscribeState TypedSubscriber<T>::getSubscriptionState() const noexcept
{
    return BaseSubscriber<T>::getSubscriptionState();
}

template <typename T>
inline void TypedSubscriber<T>::unsubscribe() noexcept
{
    return BaseSubscriber<T>::unsubscribe();
}

template <typename T>
inline bool TypedSubscriber<T>::hasNewSamples() const noexcept
{
    return BaseSubscriber<T>::hasNewSamples();
}

template <typename T>
inline cxx::expected<cxx::optional<Sample<T>>> TypedSubscriber<const T>::receive() noexcept
{
    return BaseSubscriber<T>::receive();
}

template <typename T>
inline void TypedSubscriber<T>::clearReceiveBuffer() noexcept
{
    BaseSubscriber<T>::clearReceiveBuffer();
}

} // namespace popo
} // namespace iox

#endif // IOX_EXPERIMENTAL_POSH_POPO_TYPED_SUBSCRIBER_INL
