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

#include "iceoryx_posh/runtime/posh_runtime.hpp"

namespace iox
{
namespace popo
{

template<typename T>
using SamplePtr = iox::cxx::unique_ptr<T>;

using uid_t = uint64_t;

// ======================================== Base Publisher ======================================== //

template<typename T, typename port_t>
BasePublisher<T, port_t>::BasePublisher(const capro::ServiceDescription& service)
    : m_port(iox::runtime::PoshRuntime::getInstance().getMiddlewareSender(service, ""))
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
BasePublisher<T, port_t>::loan(uint64_t size) noexcept
{
    auto header = m_port.reserveChunk(size, m_useDynamicPayloadSize);
    if (header == nullptr)
    {
        // Old API does not provide error handling, so return unknown error.
        return cxx::error<AllocationError>(AllocationError::UNKNOWN);
    }
    return cxx::success<Sample<T>>(
                cxx::unique_ptr<T>(
                    reinterpret_cast<T*>(header->payload()),
                    [this](T* const p){
                        auto header = iox::mepoo::convertPayloadPointerToChunkHeader(reinterpret_cast<void*>(p));
                        this->m_port.freeChunk(header);
                    }
                ),
                *this
            );
}

template<typename T, typename port_t>
inline void
BasePublisher<T, port_t>::release(Sample<T>& sample) noexcept
{
    auto header = iox::mepoo::convertPayloadPointerToChunkHeader(sample.get());
    m_port.freeChunk(header);
}

template<typename T, typename port_t>
inline cxx::expected<AllocationError>
BasePublisher<T, port_t>::publish(Sample<T>& sample) noexcept
{
    /// @todo - ensure sample points to valid shared memory location
    auto header = iox::mepoo::convertPayloadPointerToChunkHeader(reinterpret_cast<void* const>(sample.get()));
    m_port.deliverChunk(header);
    return iox::cxx::success<>();
}

template<typename T, typename port_t>
inline cxx::expected<SampleRecallError>
BasePublisher<T, port_t>::previousSample() const noexcept
{
    assert(false && "Not yet supported");
    return iox::cxx::error<SampleRecallError>(SampleRecallError::NO_PREVIOUS_CHUNK);
}

template<typename T, typename port_t>
inline void
BasePublisher<T, port_t>::offer() noexcept
{
    m_port.activate();
}

template<typename T, typename port_t>
inline void
BasePublisher<T, port_t>::stopOffer() noexcept
{
    m_port.deactivate();
}

template<typename T, typename port_t>
inline bool
BasePublisher<T, port_t>::isOffered() noexcept
{
    assert(false && "Not yet supported");
}

template<typename T, typename port_t>
inline bool
BasePublisher<T, port_t>::hasSubscribers() noexcept
{
    return m_port.hasSubscribers();
}


// ======================================== Typed Publisher ======================================== //

template<typename T>
TypedPublisher<T>::TypedPublisher(const capro::ServiceDescription& service)
    : BasePublisher<T>(service)
{}

template<typename T>
inline uid_t
TypedPublisher<T>::uid() const noexcept
{
    return BasePublisher<T>::uid();
}

template<typename T>
inline cxx::expected<Sample<T>, AllocationError>
TypedPublisher<T>::loan() noexcept
{
    return BasePublisher<T>::loan(sizeof(T));
}

template<typename T>
inline void
TypedPublisher<T>::release(Sample<T>& sample) noexcept
{
    return BasePublisher<T>::release(sample);
}

template<typename T>
inline cxx::expected<AllocationError>
TypedPublisher<T>::publish(Sample<T>& sample) noexcept
{
    return BasePublisher<T>::publish(sample);
}

//template<typename T>
//template<typename Callable, typename... ArgTypes>
//inline cxx::expected<AllocationError>
//TypedPublisher<T>::publishResultOf(Callable c, ArgTypes... args) noexcept
//{
//    auto result = loan();
//    if(!result.has_error())
//    {
//        auto& sample = result.get_value();
//        return publish(sample);
//    }
//    else
//    {
//        return result;
//    }
//}

template<typename T>
inline cxx::expected<AllocationError>
TypedPublisher<T>::publishCopyOf(const T& val) noexcept
{
    return BasePublisher<T>::loan(sizeof(T))
            .and_then([&](Sample<T>& sample){
                *sample.get() = val;
                BasePublisher<T>::publish(sample);
            });
}

template<typename T>
inline cxx::expected<SampleRecallError>
TypedPublisher<T>::previousSample() const noexcept
{
    return BasePublisher<T>::previousSample();
}

template<typename T>
inline void
TypedPublisher<T>::offer() noexcept
{
    return BasePublisher<T>::offer();
}

template<typename T>
inline void
TypedPublisher<T>::stopOffer() noexcept
{
    return BasePublisher<T>::stopOffer();
}

template<typename T>
inline bool
TypedPublisher<T>::isOffered() noexcept
{
    return BasePublisher<T>::isOffered();
}

template<typename T>
inline bool
TypedPublisher<T>::hasSubscribers() noexcept
{
    return BasePublisher<T>::hasSubscribers();
}

// ======================================== Untyped Publisher ======================================== //

UntypedPublisher::UntypedPublisher(const capro::ServiceDescription& service)
    : BasePublisher<void>(service)
{}

inline uid_t
UntypedPublisher::uid() const noexcept
{
    return BasePublisher<void>::uid();
}


inline cxx::expected<Sample<void>, AllocationError>
UntypedPublisher::loan(uint64_t size) noexcept
{
    return BasePublisher<void>::loan(size);
}

inline void
UntypedPublisher::release(Sample<void>& sample) noexcept
{
    return BasePublisher<void>::release(sample);
}

inline cxx::expected<AllocationError>
UntypedPublisher::publish(Sample<void>& sample) noexcept
{
    return BasePublisher<void>::publish(sample);
}

inline cxx::expected<SampleRecallError>
UntypedPublisher::previousSample() const noexcept
{
    return BasePublisher<void>::previousSample();
}

inline void
UntypedPublisher::offer() noexcept
{
    return BasePublisher<void>::offer();
}

inline void
UntypedPublisher::stopOffer() noexcept
{
    return BasePublisher<void>::stopOffer();
}

inline bool
UntypedPublisher::isOffered() noexcept
{
    return BasePublisher<void>::isOffered();
}

inline bool
UntypedPublisher::hasSubscribers() noexcept
{
    return BasePublisher<void>::hasSubscribers();
}

} // namespace popo
} // namespace iox

#endif // IOX_EXPERIMENTAL_POSH_POPO_PUBLISHER_INL
