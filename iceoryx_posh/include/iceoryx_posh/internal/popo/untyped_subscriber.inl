// Copyright (c) 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_UNTYPED_SUBSCRIBER_INL
#define IOX_POSH_POPO_UNTYPED_SUBSCRIBER_INL

namespace iox
{
namespace popo
{
template <template <typename, typename, typename> class base_subscriber_t>
inline UntypedSubscriberImpl<base_subscriber_t>::UntypedSubscriberImpl(const capro::ServiceDescription& service,
                                                                       const SubscriberOptions& subscriberOptions)
    : BaseSubscriber(service, subscriberOptions)
{
}

template <template <typename, typename, typename> class base_subscriber_t>
inline cxx::expected<const void*, ChunkReceiveResult> UntypedSubscriberImpl<base_subscriber_t>::take_1_0() noexcept
{
    auto result = BaseSubscriber::takeChunk();
    if (result.has_error())
    {
        return cxx::error<ChunkReceiveResult>(result.get_error());
    }
    return cxx::success<const void*>(result.value()->payload());
}

template <template <typename, typename, typename> class base_subscriber_t>
bool UntypedSubscriberImpl<base_subscriber_t>::hasChunks() const noexcept
{
    return BaseSubscriber::hasSamples();
}

template <template <typename, typename, typename> class base_subscriber_t>
bool UntypedSubscriberImpl<base_subscriber_t>::hasMissedChunks() noexcept
{
    return BaseSubscriber::hasMissedData();
}

template <template <typename, typename, typename> class base_subscriber_t>
void UntypedSubscriberImpl<base_subscriber_t>::releaseQueuedChunks() noexcept
{
    BaseSubscriber::releaseQueuedSamples();
}

template <template <typename, typename, typename> class base_subscriber_t>
void UntypedSubscriberImpl<base_subscriber_t>::releaseChunk(const void* payload) noexcept
{
    auto header = mepoo::ChunkHeader::fromPayload(payload);
    BaseSubscriber::releaseChunk(header);
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_UNTYPED_SUBSCRIBER_INL
