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

#ifndef IOX_POSH_POPO_BASE_PUBLISHER_INL
#define IOX_POSH_POPO_BASE_PUBLISHER_INL

#include "iceoryx_posh/runtime/posh_runtime.hpp"

namespace iox
{
namespace popo
{
template <typename T, typename port_t>
inline BasePublisher<T, port_t>::BasePublisher(const capro::ServiceDescription& service)
    : m_port(iox::runtime::PoshRuntime::getInstance().getMiddlewarePublisher(service))
{
}

template <typename T, typename port_t>
inline uid_t BasePublisher<T, port_t>::getUid() const noexcept
{
    return m_port.getUniqueID();
}

template <typename T, typename port_t>
inline capro::ServiceDescription BasePublisher<T, port_t>::getServiceDescription() const noexcept
{
    return m_port.getCaProServiceDescription();
}

template <typename T, typename port_t>
inline cxx::expected<Sample<T>, AllocationError> BasePublisher<T, port_t>::loan(const uint32_t size) noexcept
{
    auto result = m_port.tryAllocateChunk(size);
    if (result.has_error())
    {
        return cxx::error<AllocationError>(result.get_error());
    }
    else
    {
        return cxx::success<Sample<T>>(convertChunkHeaderToSample(result.get_value()));
    }
}

template <typename T, typename port_t>
inline void BasePublisher<T, port_t>::publish(Sample<T>&& sample) noexcept
{
    auto header = mepoo::convertPayloadPointerToChunkHeader(reinterpret_cast<void* const>(sample.get()));
    m_port.sendChunk(header);
    sample.release(); // Must release ownership of the sample as the sender port takes it when publishing.
}

template <typename T, typename port_t>
inline cxx::optional<Sample<T>> BasePublisher<T, port_t>::loanPreviousSample() noexcept
{
    auto result = m_port.tryGetPreviousChunk();
    if (result.has_value())
    {
        return cxx::make_optional<Sample<T>>(convertChunkHeaderToSample(result.value()));
    }
    return cxx::nullopt;
}

template <typename T, typename port_t>
inline void BasePublisher<T, port_t>::offer() noexcept
{
    m_port.offer();
}

template <typename T, typename port_t>
inline void BasePublisher<T, port_t>::stopOffer() noexcept
{
    m_port.stopOffer();
}

template <typename T, typename port_t>
inline bool BasePublisher<T, port_t>::isOffered() const noexcept
{
    return m_port.isOffered();
}

template <typename T, typename port_t>
inline bool BasePublisher<T, port_t>::hasSubscribers() const noexcept
{
    return m_port.hasSubscribers();
}

template <typename T, typename port_t>
inline Sample<T> BasePublisher<T, port_t>::convertChunkHeaderToSample(const mepoo::ChunkHeader* const header) noexcept
{
    return Sample<T>(cxx::unique_ptr<T>(reinterpret_cast<T*>(header->payload()), m_sampleDeleter), *this);
}

// ============================== Sample Deleter ============================== //

template <typename T, typename port_t>
inline BasePublisher<T, port_t>::PublisherSampleDeleter::PublisherSampleDeleter(port_t& port)
    : m_port(std::ref(port))
{
}

template <typename T, typename port_t>
inline void BasePublisher<T, port_t>::PublisherSampleDeleter::operator()(T* const ptr) const
{
    auto header = mepoo::convertPayloadPointerToChunkHeader(reinterpret_cast<void*>(ptr));
    m_port.get().freeChunk(header);
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BASE_PUBLISHER_INL
