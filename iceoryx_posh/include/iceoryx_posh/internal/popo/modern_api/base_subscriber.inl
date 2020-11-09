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

#ifndef IOX_POSH_POPO_BASE_SUBSCRIBER_INL
#define IOX_POSH_POPO_BASE_SUBSCRIBER_INL

#include "iceoryx_posh/runtime/posh_runtime.hpp"

namespace iox
{
namespace popo
{
// ============================== BaseSubscriber ============================== //

template <typename T, typename port_t>
inline BaseSubscriber<T, port_t>::BaseSubscriber(const capro::ServiceDescription& service)
    : m_port(iox::runtime::PoshRuntime::getInstance().getMiddlewareSubscriber(service))
{
}

template <typename T, typename port_t>
inline uid_t BaseSubscriber<T, port_t>::getUid() const noexcept
{
    return m_port.getUniqueID();
}

template <typename T, typename port_t>
inline capro::ServiceDescription /// todo #25 make this a reference.
BaseSubscriber<T, port_t>::getServiceDescription() const noexcept
{
    return m_port.getCaProServiceDescription();
}

template <typename T, typename port_t>
inline void BaseSubscriber<T, port_t>::subscribe(const uint64_t queueCapacity) noexcept
{
    m_port.subscribe(queueCapacity);
}

template <typename T, typename port_t>
inline SubscribeState BaseSubscriber<T, port_t>::getSubscriptionState() const noexcept
{
    return m_port.getSubscriptionState();
}

template <typename T, typename port_t>
inline void BaseSubscriber<T, port_t>::unsubscribe() noexcept
{
    m_port.unsubscribe();
}

template <typename T, typename port_t>
inline bool BaseSubscriber<T, port_t>::hasNewSamples() const noexcept
{
    return m_port.hasNewChunks();
}

template <typename T, typename port_t>
inline bool BaseSubscriber<T, port_t>::hasMissedSamples() noexcept
{
    return m_port.hasLostChunksSinceLastCall();
}

template <typename T, typename port_t>
inline cxx::expected<cxx::optional<Sample<const T>>, ChunkReceiveError> BaseSubscriber<T, port_t>::take() noexcept
{
    auto result = m_port.tryGetChunk();
    if (result.has_error())
    {
        return cxx::error<ChunkReceiveError>(result.get_error());
    }
    else
    {
        auto optionalHeader = result.get_value();
        if (optionalHeader.has_value())
        {
            auto header = optionalHeader.value();
            auto samplePtr = cxx::unique_ptr<T>(reinterpret_cast<T*>(header->payload()), m_sampleDeleter);
            return cxx::success<cxx::optional<Sample<const T>>>(
                cxx::make_optional<Sample<const T>>(std::move(samplePtr)));
        }
        else
        {
            return cxx::success<cxx::optional<Sample<const T>>>(cxx::nullopt);
        }
    }
}

template <typename T, typename port_t>
inline void BaseSubscriber<T, port_t>::releaseQueuedSamples() noexcept
{
    m_port.releaseQueuedChunks();
}

template <typename T, typename port_t>
inline bool
BaseSubscriber<T, port_t>::setConditionVariable(ConditionVariableData* const conditionVariableDataPtr) noexcept
{
    return m_port.setConditionVariable(conditionVariableDataPtr);
}

template <typename T, typename port_t>
inline bool BaseSubscriber<T, port_t>::unsetConditionVariable() noexcept
{
    return m_port.unsetConditionVariable();
}

template <typename T, typename port_t>
inline bool BaseSubscriber<T, port_t>::hasTriggered() const noexcept
{
    return m_port.hasNewChunks();
}

// ============================== Sample Deleter ============================== //

template <typename T, typename port_t>
inline BaseSubscriber<T, port_t>::SubscriberSampleDeleter::SubscriberSampleDeleter(port_t& port)
    : m_port(std::ref(port))
{
}

template <typename T, typename port_t>
inline void BaseSubscriber<T, port_t>::SubscriberSampleDeleter::operator()(T* const ptr) const
{
    auto header = mepoo::convertPayloadPointerToChunkHeader(reinterpret_cast<void*>(ptr));
    m_port.get().releaseChunk(header);
}

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BASE_SUBSCRIBER_INL
