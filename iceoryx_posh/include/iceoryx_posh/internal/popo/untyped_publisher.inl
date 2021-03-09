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
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IOX_POSH_POPO_UNTYPED_PUBLISHER_INL
#define IOX_POSH_POPO_UNTYPED_PUBLISHER_INL

namespace iox
{
namespace popo
{
template <typename base_publisher_t>
inline UntypedPublisherImpl<base_publisher_t>::UntypedPublisherImpl(const capro::ServiceDescription& service,
                                                                    const PublisherOptions& publisherOptions)
    : base_publisher_t(service, publisherOptions)
{
}

template <typename base_publisher_t>
inline void UntypedPublisherImpl<base_publisher_t>::publish(const void* chunk) noexcept
{
    auto header = mepoo::ChunkHeader::fromPayload(chunk);
    port().sendChunk(header);
}

template <typename base_publisher_t>
inline cxx::expected<void*, AllocationError> UntypedPublisherImpl<base_publisher_t>::loan(const uint32_t size) noexcept
{
    auto result = port().tryAllocateChunk(size);
    if (result.has_error())
    {
        return cxx::error<AllocationError>(result.get_error());
    }
    else
    {
        return cxx::success<void*>(result.value()->payload());
    }
}

template <typename base_publisher_t>
cxx::optional<void*> UntypedPublisherImpl<base_publisher_t>::loanPreviousChunk() noexcept
{
    auto result = port().tryGetPreviousChunk();
    if (result.has_value())
    {
        return result.value()->payload();
    }
    return cxx::nullopt;
}

template <typename base_publisher_t>
inline void UntypedPublisherImpl<base_publisher_t>::release(const void* chunk) noexcept
{
    auto header = mepoo::ChunkHeader::fromPayload(chunk);
    port().releaseChunk(header);
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_UNTYPED_PUBLISHER_INL
