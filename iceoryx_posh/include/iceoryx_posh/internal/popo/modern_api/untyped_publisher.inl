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

#ifndef IOX_POSH_POPO_UNTYPED_PUBLISHER_INL
#define IOX_POSH_POPO_UNTYPED_PUBLISHER_INL

namespace iox
{
namespace popo
{
template <typename base_publisher_t>
inline UntypedPublisherImpl<base_publisher_t>::UntypedPublisherImpl(const capro::ServiceDescription& service)
    : base_publisher_t(service)
{
}

template <typename base_publisher_t>
inline uid_t UntypedPublisherImpl<base_publisher_t>::getUid() const noexcept
{
    return base_publisher_t::getUid();
}

template <typename base_publisher_t>
inline capro::ServiceDescription UntypedPublisherImpl<base_publisher_t>::getServiceDescription() const noexcept
{
    return base_publisher_t::getServiceDescription();
}

template <typename base_publisher_t>
inline cxx::expected<Sample<void>, AllocationError>
UntypedPublisherImpl<base_publisher_t>::loan(const uint32_t size) noexcept
{
    return base_publisher_t::loan(size);
}

template <typename base_publisher_t>
inline void UntypedPublisherImpl<base_publisher_t>::publish(Sample<void>&& sample) noexcept
{
    base_publisher_t::publish(std::move(sample));
}

template <typename base_publisher_t>
inline void UntypedPublisherImpl<base_publisher_t>::publish(void* allocatedMemory) noexcept
{
    auto header = mepoo::convertPayloadPointerToChunkHeader(allocatedMemory);
    base_publisher_t::m_port.sendChunk(header);
}

template <typename base_publisher_t>
inline cxx::optional<Sample<void>> UntypedPublisherImpl<base_publisher_t>::loanPreviousSample() noexcept
{
    return base_publisher_t::loanPreviousSample();
}

template <typename base_publisher_t>
inline void UntypedPublisherImpl<base_publisher_t>::offer() noexcept
{
    return base_publisher_t::offer();
}

template <typename base_publisher_t>
inline void UntypedPublisherImpl<base_publisher_t>::stopOffer() noexcept
{
    return base_publisher_t::stopOffer();
}

template <typename base_publisher_t>
inline bool UntypedPublisherImpl<base_publisher_t>::isOffered() const noexcept
{
    return base_publisher_t::isOffered();
}

template <typename base_publisher_t>
inline bool UntypedPublisherImpl<base_publisher_t>::hasSubscribers() const noexcept
{
    return base_publisher_t::hasSubscribers();
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_UNTYPED_PUBLISHER_INL
