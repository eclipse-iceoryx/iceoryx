// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_TYPED_SUBSCRIBER_INL
#define IOX_POSH_POPO_TYPED_SUBSCRIBER_INL

namespace iox
{
namespace popo
{
template <typename T, typename base_subscriber_t>
inline TypedSubscriber<T, base_subscriber_t>::TypedSubscriber(const capro::ServiceDescription& service,
                                                              const SubscriberOptions& subscriberOptions)
    : base_subscriber_t(service, subscriberOptions)
{
}

template <typename T, typename base_subscriber_t>
inline cxx::expected<Sample<const T>, ChunkReceiveResult> TypedSubscriber<T, base_subscriber_t>::take() noexcept
{
    auto result = base_subscriber_t::takeChunk();
    if (result.has_error())
    {
        return cxx::error<ChunkReceiveResult>(result.get_error());
    }
    auto payloadPtr = static_cast<T*>(result.value()->payload());
    SubscriberSampleDeleter deleter(port());
    auto samplePtr = cxx::unique_ptr<T>(static_cast<T*>(payloadPtr), deleter);
    return cxx::success<Sample<const T>>(std::move(samplePtr));
}

template <typename T, typename base_subscriber_t>
template <uint64_t WaitSetCapacity>
inline cxx::expected<WaitSetError>
TypedSubscriber<T, base_subscriber_t>::enableEvent(WaitSet<WaitSetCapacity>& waitset,
                                                   const SubscriberEvent subscriberEvent,
                                                   const uint64_t eventId,
                                                   const EventInfo::Callback<SelfType> callback) noexcept
{
    return base_subscriber_t::enableEventInternal(waitset, subscriberEvent, eventId, callback);
}

template <typename T, typename base_subscriber_t>
template <uint64_t WaitSetCapacity>
inline cxx::expected<WaitSetError>
TypedSubscriber<T, base_subscriber_t>::enableEvent(WaitSet<WaitSetCapacity>& waitset,
                                                   const SubscriberEvent subscriberEvent,
                                                   const EventInfo::Callback<SelfType> callback) noexcept
{
    return base_subscriber_t::enableEventInternal(waitset, subscriberEvent, EventInfo::INVALID_ID, callback);
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_TYPED_SUBSCRIBER_INL
