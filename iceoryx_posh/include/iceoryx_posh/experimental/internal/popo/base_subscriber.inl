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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_BASE_SUBSCRIBER_INL
#define IOX_EXPERIMENTAL_POSH_POPO_BASE_SUBSCRIBER_INL

namespace iox {
namespace popo {

template<typename T, typename port_t>
BaseSubscriber<T, port_t>::BaseSubscriber(const capro::ServiceDescription&)
/// @todo #25  : m_port(iox::runtime::PoshRuntime::getInstance().getMiddlewareReceiver(service, ""))
{}

template<typename T, typename port_t>
inline uid_t
BaseSubscriber<T, port_t>::uid() const noexcept
{
    return m_uid;
}

template<typename T, typename port_t>
inline capro::ServiceDescription /// todo #25 make this a reference.
BaseSubscriber<T, port_t>::getServiceDescription() const noexcept
{
    /// @todo #25 return reference to ServiceDescription from base port.
    return capro::ServiceDescription{};
}

template<typename T, typename port_t>
inline void
BaseSubscriber<T, port_t>::subscribe(const uint64_t queueCapacity) noexcept
{
    m_subscriptionRequested = true;
    m_port.subscribe(queueCapacity);
}

template<typename T, typename port_t>
inline SubscribeState
BaseSubscriber<T, port_t>::getSubscriptionState() const noexcept
{
   return m_port.getSubscriptionState();
}

template<typename T, typename port_t>
inline void
BaseSubscriber<T, port_t>::unsubscribe() noexcept
{
    m_port.unsubscribe();
    m_subscriptionRequested = false;
}

template<typename T, typename port_t>
inline bool
BaseSubscriber<T, port_t>::hasNewSamples() const noexcept
{
    return m_port.hasNewChunks();
}

template<typename T, typename port_t>
inline cxx::expected<cxx::optional<Sample<T>>, ChunkReceiveError>
BaseSubscriber<T, port_t>::receive() noexcept
{
    auto result = m_port.tryGetChunk();
    if(result.has_error())
    {
        return cxx::error<ChunkReceiveError>(result.get_error());
    }
    else
    {
        auto optionalHeader = result.get_value();
        if(optionalHeader.has_value())
        {
            auto header = optionalHeader.value();
            auto samplePtr = cxx::unique_ptr<T>(reinterpret_cast<T*>(header->payload()),
                                             [this](T* const allocation){
                                                auto header = mepoo::convertPayloadPointerToChunkHeader(allocation);
                                                this->m_port.releaseChunk(header);
                                             });
            return cxx::success<cxx::optional<Sample<T>>>(cxx::make_optional<Sample<T>>(std::move(samplePtr)));
        }
        else
        {
            return cxx::success<cxx::optional<Sample<T>>>(cxx::nullopt);
        }
    }
}

template<typename T, typename port_t>
inline void
BaseSubscriber<T, port_t>::clearReceiveBuffer() noexcept
{
    m_port.releaseQueuedChunks();
}

template<typename T, typename port_t>
inline bool
BaseSubscriber<T, port_t>::setConditionVariable(ConditionVariableData* const conditionVariableDataPtr) noexcept
{
    return m_port.setConditionVariable(conditionVariableDataPtr);
}

template<typename T, typename port_t>
inline bool
BaseSubscriber<T, port_t>::unsetConditionVariable() noexcept
{
    return m_port.unsetConditionVariable();
}

template<typename T, typename port_t>
inline bool
BaseSubscriber<T, port_t>::hasTriggered() const noexcept
{
    return m_port.hasNewChunks();
}

} // namespace popo
} // namespace iox

#endif // IOX_EXPERIMENTAL_POSH_POPO_BASE_SUBSCRIBER_INL
