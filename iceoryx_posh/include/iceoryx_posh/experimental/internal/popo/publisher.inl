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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_PUBLISHER_INL
#define IOX_EXPERIMENTAL_POSH_POPO_PUBLISHER_INL

#include "iceoryx_posh/experimental/popo/publisher.hpp"

#include <iostream>

namespace iox
{
namespace popo
{

template<typename T>
using SamplePtr = iox::cxx::unique_ptr<T>;

using uid_t = uint64_t;

template<typename T, typename port_t>
inline uid_t
Publisher<T, port_t>::uid() const noexcept
{
    std::cout << "uid()" << std::endl;
    return 0u;
}

template<typename T, typename port_t>
inline cxx::expected<SamplePtr<T>, AllocationError>
Publisher<T, port_t>::allocate() const noexcept
{
    return m_sender.allocateChunk(sizeof(T))
        .and_then([&](mepoo::ChunkHeader* header){
            auto ptr = iox::cxx::unique_ptr<T>(header->payload(), [this](T* const p){
                this->release(p);
            });
            return iox::cxx::success<SamplePtr>(std::move(ptr));
        })
        .or_else([](AllocationError err){
            return iox::cxx::error<AllocationError>(err);
        });
}

template<typename T, typename port_t>
cxx::expected<AllocationError>
Publisher<T, port_t>::allocate(cxx::function_ref<void(SamplePtr&) noexcept> f) const noexcept
{
    auto result = allocate()
        .and_then([&f](SamplePtr s){
            f(s);
            return cxx::success<>();
        })
        .or_else([](AllocationError err){
            return cxx::error<AllocationError>(err);
        });
    return result;
};

template<typename T, typename port_t>
inline void
Publisher<T, port_t>::release(SamplePtr&& sample) const noexcept
{
    auto header = iox::mepoo::convertPayloadPointerToChunkHeader(sample.release());
    m_sender.freeChunk(header);
}

template<typename T, typename port_t>
inline void
Publisher<T, port_t>::publish(SamplePtr&& sample) const noexcept
{
    auto header = iox::mepoo::convertPayloadPointerToChunkHeader(const_cast<void* const>(sample.release()));
    m_sender.deliverChunk(header);
}

template<typename T, typename port_t>
inline void
Publisher<T, port_t>::publishCopyOf(const T& val) const noexcept
{
    std::cout << "publishCopyOf()" << std::endl;
}

template<typename T, typename port_t>
inline cxx::expected<ChunkRecallError>
Publisher<T, port_t>::previous() const noexcept
{
    assert(false && "Not yet supported");
    return iox::cxx::error<ChunkRecallError>(ChunkRecallError::NO_PREVIOUS_CHUNK);
}

template<typename T, typename port_t>
inline void
Publisher<T, port_t>::offer() noexcept
{
    m_sender.activate();
}

template<typename T, typename port_t>
inline void
Publisher<T, port_t>::stopOffer() noexcept
{
    m_sender.deactivate();
}

template<typename T, typename port_t>
inline bool
Publisher<T, port_t>::isOffered() const noexcept
{
    return m_sender.isPortActive();
}

template<typename T, typename port_t>
inline bool
Publisher<T, port_t>::hasSubscribers() const noexcept
{
    return m_sender.hasSubscribers();
}

} // namespace popo
} // namespace iox

#endif // IOX_EXPERIMENTAL_POSH_POPO_PUBLISHER_INL
