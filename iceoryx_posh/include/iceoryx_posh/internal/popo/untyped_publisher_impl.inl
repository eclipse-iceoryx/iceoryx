// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_UNTYPED_PUBLISHER_IMPL_INL
#define IOX_POSH_POPO_UNTYPED_PUBLISHER_IMPL_INL

#include "iceoryx_posh/internal/popo/untyped_publisher_impl.hpp"

namespace iox
{
namespace popo
{
template <typename BasePublisherType>
inline UntypedPublisherImpl<BasePublisherType>::UntypedPublisherImpl(const capro::ServiceDescription& service,
                                                                     const PublisherOptions& publisherOptions)
    : BasePublisherType(service, publisherOptions)
{
}

template <typename BasePublisherType>
inline UntypedPublisherImpl<BasePublisherType>::UntypedPublisherImpl(PortType&& port) noexcept
    : BasePublisherType(std::move(port))
{
}

template <typename BasePublisherType>
inline void UntypedPublisherImpl<BasePublisherType>::publish(void* const userPayload) noexcept
{
    auto chunkHeader = mepoo::ChunkHeader::fromUserPayload(userPayload);
    port().sendChunk(chunkHeader);
}

template <typename BasePublisherType>
inline expected<void*, AllocationError>
UntypedPublisherImpl<BasePublisherType>::loan(const uint64_t userPayloadSize,
                                              const uint32_t userPayloadAlignment,
                                              const uint32_t userHeaderSize,
                                              const uint32_t userHeaderAlignment) noexcept
{
    auto result = port().tryAllocateChunk(userPayloadSize, userPayloadAlignment, userHeaderSize, userHeaderAlignment);
    if (result.has_error())
    {
        return err(result.error());
    }
    else
    {
        return ok(result.value()->userPayload());
    }
}

template <typename BasePublisherType>
inline void UntypedPublisherImpl<BasePublisherType>::release(void* const userPayload) noexcept
{
    auto chunkHeader = mepoo::ChunkHeader::fromUserPayload(userPayload);
    port().releaseChunk(chunkHeader);
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_UNTYPED_PUBLISHER_IMPL_INL
