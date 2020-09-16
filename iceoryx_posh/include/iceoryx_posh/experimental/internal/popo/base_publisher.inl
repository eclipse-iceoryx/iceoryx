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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_BASE_PUBLISHER_INL
#define IOX_EXPERIMENTAL_POSH_POPO_BASE_PUBLISHER_INL

namespace iox {
namespace popo {

using uid_t = uint64_t;

template<typename T, typename port_t>
BasePublisher<T, port_t>::BasePublisher(const capro::ServiceDescription&)
    /* : m_port(iox::runtime::PoshRuntime::getInstance().getMiddlewareSender(service, "")) */
{}

template<typename T, typename port_t>
inline uid_t
BasePublisher<T, port_t>::uid() const noexcept
{
    std::cout << "uid()" << std::endl;
    return 0u;
}

template<typename T, typename port_t>
inline cxx::expected<Sample<T>, AllocationError>
BasePublisher<T, port_t>::loan(uint32_t size) noexcept
{
    auto result = m_port.tryAllocateChunk(size);
    if(result.has_error())
    {
        return cxx::error<AllocationError>(result.get_error());
    }
    else
    {
        return cxx::success<Sample<T>>(convertChunkHeaderToSample(result.get_value()));
    }
}

template<typename T, typename port_t>
inline void
BasePublisher<T, port_t>::publish(Sample<T>& sample) noexcept
{
    if(!isOffered())
    {
        offer();
    }
    auto header = mepoo::convertPayloadPointerToChunkHeader(reinterpret_cast<void* const>(sample.get()));
    m_port.sendChunk(header);
}

template<typename T, typename port_t>
inline cxx::optional<Sample<T>>
BasePublisher<T, port_t>::previousSample() noexcept
{
    auto result = m_port.getLastChunk();
    if(result.has_value())
    {
        return cxx::make_optional<Sample<T>>(convertChunkHeaderToSample(result.value()));
    }
    return cxx::nullopt;
}

template<typename T, typename port_t>
inline void
BasePublisher<T, port_t>::offer() noexcept
{
    m_port.offer();
}

template<typename T, typename port_t>
inline void
BasePublisher<T, port_t>::stopOffer() noexcept
{
    m_port.stopOffer();
}

template<typename T, typename port_t>
inline bool
BasePublisher<T, port_t>::isOffered() noexcept
{
    return m_port.isOffered();
}

template<typename T, typename port_t>
inline bool
BasePublisher<T, port_t>::hasSubscribers() noexcept
{
    return m_port.hasSubscribers();
}

template<typename T, typename port_t>
inline Sample<T>
BasePublisher<T, port_t>::convertChunkHeaderToSample(const mepoo::ChunkHeader* header) noexcept
{
    return Sample<T>(
                cxx::unique_ptr<T>(
                    reinterpret_cast<T*>(header->payload()),
                    [this](T* const p){
                        auto header = mepoo::convertPayloadPointerToChunkHeader(reinterpret_cast<void*>(p));
                        this->m_port.freeChunk(header);
                    }
                ),
                *this
            );
}

} // namespace popo
} // namespace iox

#endif // IOX_EXPERIMENTAL_POSH_POPO_BASE_PUBLISHER_INL
