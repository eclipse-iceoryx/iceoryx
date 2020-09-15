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
    /* : m_port(iox::runtime::PoshRuntime::getInstance().getMiddlewareReceiver(service, "")) */
{}

template<typename T, typename port_t>
inline cxx::expected<SubscriberError>
BaseSubscriber<T, port_t>::subscribe(const uint64_t queueCapacity) noexcept
{
    m_subscriptionRequested = true;
    uint32_t size = queueCapacity;
    if (size > MAX_RECEIVER_QUEUE_CAPACITY)
    {
        LogWarn() << "Cache size for subscribe too large " << size
                  << ", limiting to MAX_RECEIVER_QUEUE_CAPACITY = " << MAX_RECEIVER_QUEUE_CAPACITY;
        size = MAX_RECEIVER_QUEUE_CAPACITY;
    }
    m_port.subscribe(size);
    return cxx::success<>();
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
BaseSubscriber<T, port_t>::hasData() noexcept
{
    return m_port.hasNewChunks();
}

template<typename T, typename port_t>
inline cxx::optional<cxx::unique_ptr<T>>
BaseSubscriber<T, port_t>::receive() noexcept
{
    auto result = receiveWithHeader();
    if(result.has_error())
    {
        return result;
    }
    else
    {
        auto header = result.get_value();
        return cxx::optional<cxx::unique_ptr<T>>(cxx::unique_ptr<T>(
                                    reinterpret_cast<T*>(header.payload()),
                                    [this](T* const val){
                                        auto header = mepoo::convertPayloadPointerToChunkHeader(val);
                                        this->m_port.releaseChunk(header);
                                    }
                                ));
    }
}

template<typename T, typename port_t>
inline cxx::optional<cxx::unique_ptr<mepoo::ChunkHeader>>
BaseSubscriber<T, port_t>::receiveWithHeader() noexcept
{
    auto result = m_port.getChunk();
    if(result.has_error())
    {
        /// @todo - what should we do when getting ChunkReceiveError ?
    }
    else
    {
        auto optionalHeader = result.get_value();
        if(optionalHeader.has_value())
        {
            auto header = optionalHeader.value();
            return cxx::optional<cxx::unique_ptr<T>>(cxx::unique_ptr<T>(
                                        reinterpret_cast<T*>(header),
                                        [this](mepoo::ChunkHeader* const header){
                                            this->m_port.releaseChunk(header);
                                        }
                                    ));
        }
        return cxx::nullopt_t();
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
    return m_port.attachConditionVariable(conditionVariableDataPtr);
}

template<typename T, typename port_t>
inline bool
BaseSubscriber<T, port_t>::unsetConditionVariable() noexcept
{
    return m_port.detachConditionVariable();
}

template<typename T, typename port_t>
inline bool
BaseSubscriber<T, port_t>::hasTriggered() const noexcept
{
    /// @todo Add implementation
    return false;
}

} // namespace popo
} // namespace iox

#endif // IOX_EXPERIMENTAL_POSH_POPO_BASE_SUBSCRIBER_INL
