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

#ifndef IOX_POSH_POPO_TYPED_PUBLISHER_INL
#define IOX_POSH_POPO_TYPED_PUBLISHER_INL

#include <cstdint>

namespace iox
{
namespace popo
{
template <typename T, typename H, typename base_publisher_t>
inline Publisher<T, H, base_publisher_t>::Publisher(const capro::ServiceDescription& service,
                                                    const PublisherOptions& publisherOptions)
    : base_publisher_t(service, publisherOptions)
{
}

template <typename T, typename H, typename base_publisher_t>
template <typename... Args>
inline cxx::expected<Sample<T, H>, AllocationError> Publisher<T, H, base_publisher_t>::loan(Args&&... args) noexcept
{
    return std::move(loanSample().and_then([&](auto& sample) { new (sample.get()) T(std::forward<Args>(args)...); }));
}

template <typename T, typename H, typename base_publisher_t>
template <typename Callable, typename... ArgTypes>
inline cxx::expected<AllocationError> Publisher<T, H, base_publisher_t>::publishResultOf(Callable c,
                                                                                         ArgTypes... args) noexcept
{
    static_assert(cxx::is_invocable<Callable, T*, ArgTypes...>::value,
                  "Publisher<T>::publishResultOf expects a valid callable with a specific signature as the "
                  "first argument");
    static_assert(cxx::has_signature<Callable, void(T*, ArgTypes...)>::value,
                  "callable provided to Publisher<T>::publishResultOf must have signature void(T*, ArgsTypes...)");

    return loanSample().and_then([&](auto& sample) {
        c(sample.get(), std::forward<ArgTypes>(args)...);
        publish(std::move(sample));
    });
}

template <typename T, typename H, typename base_publisher_t>
inline cxx::expected<AllocationError> Publisher<T, H, base_publisher_t>::publishCopyOf(const T& val) noexcept
{
    return loanSample().and_then([&](auto& sample) {
        *sample.get() = val; // Copy assignment of value into sample's memory allocation.
        publish(std::move(sample));
    });
}

template <typename T, typename H, typename base_publisher_t>
inline cxx::expected<Sample<T, H>, AllocationError> Publisher<T, H, base_publisher_t>::loanSample() noexcept
{
    static constexpr uint32_t CUSTOM_HEADER_SIZE{std::is_same<H, mepoo::NoCustomHeader>::value ? 0U : sizeof(H)};

    auto result = port().tryAllocateChunk(sizeof(T), alignof(T), CUSTOM_HEADER_SIZE, alignof(H));
    if (result.has_error())
    {
        return cxx::error<AllocationError>(result.get_error());
    }
    else
    {
        return cxx::success<Sample<T, H>>(convertChunkHeaderToSample(result.value()));
    }
}

template <typename T, typename H, typename base_publisher_t>
inline void Publisher<T, H, base_publisher_t>::publish(Sample<T, H>&& sample) noexcept
{
    auto chunkPayload = sample.release(); // release the Samples ownership of the chunk before publishing
    auto header = mepoo::ChunkHeader::fromPayload(chunkPayload);
    port().sendChunk(header);
}

template <typename T, typename H, typename base_publisher_t>
inline cxx::optional<Sample<T, H>> Publisher<T, H, base_publisher_t>::loanPreviousSample() noexcept
{
    auto result = port().tryGetPreviousChunk();
    if (result.has_value())
    {
        return cxx::make_optional<Sample<T, H>>(convertChunkHeaderToSample(result.value()));
    }
    return cxx::nullopt;
}

template <typename T, typename H, typename base_publisher_t>
inline Sample<T, H>
Publisher<T, H, base_publisher_t>::convertChunkHeaderToSample(const mepoo::ChunkHeader* const header) noexcept
{
    return Sample<T, H>(cxx::unique_ptr<T>(reinterpret_cast<T*>(header->payload()), m_sampleDeleter), *this);
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_TYPED_PUBLISHER_INL
