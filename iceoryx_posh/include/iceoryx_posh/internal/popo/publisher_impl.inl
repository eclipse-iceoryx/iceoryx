// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 NXP. All rights reserved.
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

#ifndef IOX_POSH_POPO_TYPED_PUBLISHER_IMPL_INL
#define IOX_POSH_POPO_TYPED_PUBLISHER_IMPL_INL

#include "iceoryx_posh/internal/popo/publisher_impl.hpp"

#include <cstdint>

namespace iox
{
namespace popo
{
template <typename T, typename H, typename BasePublisherType>
inline PublisherImpl<T, H, BasePublisherType>::PublisherImpl(const capro::ServiceDescription& service,
                                                             const PublisherOptions& publisherOptions)
    : BasePublisherType(service, publisherOptions)
{
}

template <typename T, typename H, typename BasePublisherType>
template <typename... Args>
inline expected<Sample<T, H>, AllocationError> PublisherImpl<T, H, BasePublisherType>::loan(Args&&... args) noexcept
{
    return std::move(loanSample().and_then([&](auto& sample) { new (sample.get()) T(std::forward<Args>(args)...); }));
}

template <typename T, typename H, typename BasePublisherType>
template <typename Callable, typename... ArgTypes>
inline expected<void, AllocationError>
PublisherImpl<T, H, BasePublisherType>::publishResultOf(Callable c, ArgTypes... args) noexcept
{
    static_assert(is_invocable<Callable, T*, ArgTypes...>::value,
                  "Publisher<T>::publishResultOf expects a valid callable with a specific signature as the "
                  "first argument");
    static_assert(is_invocable_r<void, Callable, T*, ArgTypes...>::value,
                  "callable provided to Publisher<T>::publishResultOf must have signature void(T*, ArgsTypes...)");

    return loanSample().and_then([&](auto& sample) {
        c(new (sample.get()) T, std::forward<ArgTypes>(args)...);
        sample.publish();
    });
}

template <typename T, typename H, typename BasePublisherType>
inline expected<void, AllocationError> PublisherImpl<T, H, BasePublisherType>::publishCopyOf(const T& val) noexcept
{
    return loanSample().and_then([&](auto& sample) {
        new (sample.get()) T(val); // Placement new copy-construction of sample, avoid copy-assigment because there
                                   // might not be an existing instance of T in the sample memory
        sample.publish();
    });
}

template <typename T, typename H, typename BasePublisherType>
inline expected<Sample<T, H>, AllocationError> PublisherImpl<T, H, BasePublisherType>::loanSample() noexcept
{
    static constexpr uint32_t USER_HEADER_SIZE{std::is_same<H, mepoo::NoUserHeader>::value ? 0U : sizeof(H)};

    auto result = port().tryAllocateChunk(sizeof(T), alignof(T), USER_HEADER_SIZE, alignof(H));
    if (result.has_error())
    {
        return err(result.error());
    }
    else
    {
        return ok(convertChunkHeaderToSample(result.value()));
    }
}

template <typename T, typename H, typename BasePublisherType>
inline void PublisherImpl<T, H, BasePublisherType>::publish(Sample<T, H>&& sample) noexcept
{
    auto userPayload = sample.release(); // release the Samples ownership of the chunk before publishing
    auto chunkHeader = mepoo::ChunkHeader::fromUserPayload(userPayload);
    port().sendChunk(chunkHeader);
}

template <typename T, typename H, typename BasePublisherType>
inline Sample<T, H>
PublisherImpl<T, H, BasePublisherType>::convertChunkHeaderToSample(mepoo::ChunkHeader* const header) noexcept
{
    return Sample<T, H>(iox::unique_ptr<T>(reinterpret_cast<T*>(header->userPayload()),
                                           [this](T* userPayload) {
                                               auto* chunkHeader =
                                                   iox::mepoo::ChunkHeader::fromUserPayload(userPayload);
                                               this->port().releaseChunk(chunkHeader);
                                           }),
                        *this);
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_TYPED_PUBLISHER_IMPL_INL
