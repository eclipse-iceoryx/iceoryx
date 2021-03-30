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
template <typename H, typename BasePublisher_t>
inline UntypedPublisherImpl<H, BasePublisher_t>::UntypedPublisherImpl(const capro::ServiceDescription& service,
                                                                       const PublisherOptions& publisherOptions)
    : BasePublisher_t(service, publisherOptions)
{
}

template <typename H, typename BasePublisher_t>
inline void UntypedPublisherImpl<H, BasePublisher_t>::publish(const void* userPayload) noexcept
{
    auto chunkHeader = mepoo::ChunkHeader::fromUserPayload(userPayload);
    port().sendChunk(chunkHeader);
}

template <typename H, typename BasePublisher_t>
inline cxx::expected<void*, AllocationError>
UntypedPublisherImpl<H, BasePublisher_t>::loan(const uint32_t userPayloadSize, const uint32_t userPayloadAlignment) noexcept
{
    static constexpr uint32_t USER_HEADER_SIZE{std::is_same<H, mepoo::NoUserHeader>::value ? 0U : sizeof(H)};

    auto result = port().tryAllocateChunk(userPayloadSize, userPayloadAlignment, USER_HEADER_SIZE, alignof(H));
    if (result.has_error())
    {
        return cxx::error<AllocationError>(result.get_error());
    }
    else
    {
        return cxx::success<void*>(result.value()->userPayload());
    }
}

template <typename H, typename BasePublisher_t>
cxx::optional<void*> UntypedPublisherImpl<H, BasePublisher_t>::loanPreviousChunk() noexcept
{
    auto result = port().tryGetPreviousChunk();
    if (result.has_value())
    {
        return result.value()->userPayload();
    }
    return cxx::nullopt;
}

template <typename H, typename BasePublisher_t>
inline void UntypedPublisherImpl<H, BasePublisher_t>::release(const void* userPayload) noexcept
{
    auto chunkHeader = mepoo::ChunkHeader::fromUserPayload(userPayload);
    port().releaseChunk(chunkHeader);
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_UNTYPED_PUBLISHER_INL
