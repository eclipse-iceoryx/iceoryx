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

//template<typename T, typename port_t>
//Publisher<T, port_t>::Publisher(const capro::ServiceDescription& service)
//    : m_port();
//{

//}

template<typename T, typename port_t>
inline uid_t
Publisher<T, port_t>::uid() const noexcept
{
    std::cout << "uid()" << std::endl;
    return 0u;
}

template<typename T, typename port_t>
inline cxx::expected<Sample<T>, AllocationError>
Publisher<T, port_t>::loan() noexcept
{
//    return m_port.allocateChunk(sizeof(T))
//        .and_then([&](mepoo::ChunkHeader* header){
//            auto sample = Sample<T>(iox::cxx::unique_ptr<T>(header->payload(), [this](T* const p){
//                this->release(p);
//            }));
//            return iox::cxx::success<Sample<T>>(std::move(sample));
//        })
//        .or_else([](AllocationError err){
//            return iox::cxx::error<AllocationError>(err);
//        });

    uint8_t* buf = new uint8_t[sizeof(T)];
    Sample<T> sample(reinterpret_cast<T*>(buf), [this, buf](T* const p){
        delete[] buf;
    }, *this);
    return iox::cxx::success<Sample<T>>(std::move(sample));
}

//// This one maybe excessive... why not just use loan.and_then() ?
//template<typename T, typename port_t>
//cxx::expected<AllocationError>
//Publisher<T, port_t>::loan(cxx::function_ref<void(Sample<T>&) noexcept> f) noexcept
//{
//    auto result = loan()
//        .and_then([&f](Sample<T> s){
//            f(s);
//            return cxx::success<>();
//        })
//        .or_else([](AllocationError err){
//            return cxx::error<AllocationError>(err);
//        });
//    return result;
//};

template<typename T, typename port_t>
inline void
Publisher<T, port_t>::release(Sample<T>&& sample) noexcept
{
    auto header = iox::mepoo::convertPayloadPointerToChunkHeader(sample.allocation());
    m_port.freeChunk(header);
}

template<typename T, typename port_t>
inline void
Publisher<T, port_t>::publish(Sample<T>&& sample) noexcept
{
    auto header = iox::mepoo::convertPayloadPointerToChunkHeader(reinterpret_cast<void* const>(sample.allocation()));
    m_port.sendChunk(header);
}

template<typename T, typename port_t>
inline void
Publisher<T, port_t>::publish(cxx::function_ref<void(T*)> f) noexcept
{
    loan()
        .and_then([&](Sample<T>& sample){
            f(sample.allocation()); // Populate the sample with the given function.
            publish(std::move(sample));
        });
}

template<typename T, typename port_t>
inline void
Publisher<T, port_t>::publishCopyOf(const T& val) noexcept
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
Publisher<T, port_t>::offer() const noexcept
{
    m_port.offer();
}

template<typename T, typename port_t>
inline void
Publisher<T, port_t>::stopOffer() const noexcept
{
    m_port.stopOffer();
}

template<typename T, typename port_t>
inline bool
Publisher<T, port_t>::isOffered() const noexcept
{
    return m_port.isOffered();
}

template<typename T, typename port_t>
inline bool
Publisher<T, port_t>::hasSubscribers() const noexcept
{
    return m_port.hasSubscribers();
}

} // namespace popo
} // namespace iox

#endif // IOX_EXPERIMENTAL_POSH_POPO_PUBLISHER_INL
