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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_SUBSCRIBER_INL
#define IOX_EXPERIMENTAL_POSH_POPO_SUBSCRIBER_INL

#include <iostream>

namespace iox {
namespace popo {

// ======================================== Base Subscriber ======================================== //

template<typename T, typename recvport_t>
BaseSubscriber<T, recvport_t>::BaseSubscriber(const capro::ServiceDescription& service)
    : m_port(iox::runtime::PoshRuntime::getInstance().getMiddlewareReceiver(service, ""))
{}

template<typename T, typename recvport_t>
inline cxx::expected<SubscriberError>
BaseSubscriber<T, recvport_t>::subscribe(const uint64_t cacheSize) noexcept
{
    m_subscriptionRequested = true;
    uint32_t size = cacheSize;
    if (size > MAX_RECEIVER_QUEUE_CAPACITY)
    {
        LogWarn() << "Cache size for subscribe too large " << size
                  << ", limiting to MAX_RECEIVER_QUEUE_CAPACITY = " << MAX_RECEIVER_QUEUE_CAPACITY;
        size = MAX_RECEIVER_QUEUE_CAPACITY;
    }
    m_port.subscribe(true, size);
    return cxx::success<>();
}

template<typename T, typename recvport_t>
inline SubscriptionState
BaseSubscriber<T, recvport_t>::getSubscriptionState() const noexcept
{
    if (!m_subscriptionRequested)
    {
        return SubscriptionState::NOT_SUBSCRIBED;
    }
    else
    {
        if (m_port.isSubscribed())
        {
            return SubscriptionState::SUBSCRIBED;
        }
        else
        {
            return SubscriptionState::SUBSCRIPTION_PENDING;
        }
    }
}

template<typename T, typename recvport_t>
inline void
BaseSubscriber<T, recvport_t>::unsubscribe() noexcept
{
    m_port.unsubscribe();
    m_subscriptionRequested = false;
}

template<typename T, typename recvport_t>
inline bool
BaseSubscriber<T, recvport_t>::hasData() const noexcept
{
    return m_port.newData();
}

template<typename T, typename recvport_t>
inline cxx::optional<cxx::unique_ptr<T>>
BaseSubscriber<T, recvport_t>::receive() noexcept
{

    const mepoo::ChunkHeader* header = nullptr;
    if (m_port.getChunk(header))
    {
        return cxx::optional<cxx::unique_ptr<T>>(cxx::unique_ptr<T>(
                                    reinterpret_cast<T*>(header->payload()),
                                    [this](T* const p){
                                        auto header = iox::mepoo::convertPayloadPointerToChunkHeader(reinterpret_cast<void*>(p));
                                        this->m_port.releaseChunk(header);
                                    }
                                ));
    }
    else
    {
        return cxx::nullopt_t();
    }
}


template<typename T, typename recvport_t>
inline cxx::optional<cxx::unique_ptr<mepoo::ChunkHeader>>
BaseSubscriber<T, recvport_t>::receiveWithHeader() noexcept
{
    const mepoo::ChunkHeader* header = nullptr;
    if (m_port.getChunk(header))
    {
        return cxx::optional<cxx::unique_ptr<T>>(cxx::unique_ptr<T>(
                                    reinterpret_cast<T*>(header),
                                    [this](T* const header){
                                        this->m_port.releaseChunk(header);
                                    }
                                ));
    }
    else
    {
        return cxx::nullopt_t();
    }
}


template<typename T, typename recvport_t>
inline void
BaseSubscriber<T, recvport_t>::clearReceiveBuffer() noexcept
{
    m_port.clearDeliveryFiFo();
}

template<typename T, typename recvport_t>
inline bool
BaseSubscriber<T, recvport_t>::setConditionVariable(ConditionVariableData* const conditionVariableDataPtr) noexcept
{

}

template<typename T, typename recvport_t>
inline bool
BaseSubscriber<T, recvport_t>::unsetConditionVariable() noexcept
{

}

template<typename T, typename recvport_t>
inline bool
BaseSubscriber<T, recvport_t>::hasTriggered() const noexcept
{

}

// ======================================== Typed Subscriber ======================================== //

template<typename T>
TypedSubscriber<T>::TypedSubscriber(const capro::ServiceDescription& service)
    : BaseSubscriber<T>(service)
{}

template<typename T>
inline cxx::expected<SubscriberError>
TypedSubscriber<T>::subscribe(const uint64_t cacheSize) noexcept
{
    return BaseSubscriber<T>::subscribe(cacheSize);
}

template<typename T>
inline SubscriptionState
TypedSubscriber<T>::getSubscriptionState() const noexcept
{
    return BaseSubscriber<T>::getSubscriptionState();
}

template<typename T>
inline void
TypedSubscriber<T>::unsubscribe() noexcept
{
    return BaseSubscriber<T>::unsubscribe();
}

template<typename T>
inline bool
TypedSubscriber<T>::hasData() const noexcept
{
    return BaseSubscriber<T>::hasData();
}

template<typename T>
inline cxx::optional<cxx::unique_ptr<T>>
TypedSubscriber<T>::receive() noexcept
{
    return BaseSubscriber<T>::receive();
}

template<typename T>
inline cxx::optional<cxx::unique_ptr<mepoo::ChunkHeader>>
TypedSubscriber<T>::receiveWithHeader() noexcept
{
    return BaseSubscriber<T>::receiveWithHeader();
}

template<typename T>
inline void
TypedSubscriber<T>::clearReceiveBuffer() noexcept
{
    BaseSubscriber<T>::clearReceiveBuffer();
}

// ======================================== Untyped Subscriber ======================================== //


} // namespace popo
} // namespace iox

#endif // IOX_EXPERIMENTAL_POSH_POPO_SUBSCRIBER_INL
