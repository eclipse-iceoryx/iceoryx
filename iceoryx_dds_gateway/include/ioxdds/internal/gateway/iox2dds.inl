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

#include <chrono>
#include <thread>

#include <iceoryx_posh/mepoo/chunk_header.hpp>
#include <iceoryx_utils/cxx/string.hpp>

#include "ioxdds/gateway/iox2dds.hpp"
#include "ioxdds/internal/log/logging.hpp"

namespace iox
{
namespace gateway
{
namespace dds
{

// ======================================== SubscriberDataWriterPair ======================================== //
// Typedefs
template<typename subscriber_t>
using SubscriberPool = iox::cxx::ObjectPool<subscriber_t, MAX_CHANNEL_NUMBER>;
template<typename data_writer_t>
using DataWriterPool = iox::cxx::ObjectPool<data_writer_t, MAX_CHANNEL_NUMBER>;

// Statics
template<typename subscriber_t, typename data_writer_t>
SubscriberPool<subscriber_t> Channel<subscriber_t, data_writer_t>::s_subscriberPool = SubscriberPool();
template<typename subscriber_t, typename data_writer_t>
DataWriterPool<data_writer_t> Channel<subscriber_t, data_writer_t>::s_dataWriterPool = DataWriterPool();

template<typename subscriber_t, typename data_writer_t>
inline Channel<subscriber_t, data_writer_t>::Channel(const iox::capro::ServiceDescription& service, SubscriberPtr subscriber, DataWriterPtr dataWriter)
{
    this->service = service;
    this->subscriber = subscriber;
    this->dataWriter = dataWriter;
}

template<typename subscriber_t, typename data_writer_t>
inline Channel<subscriber_t, data_writer_t> Channel<subscriber_t, data_writer_t>::create(const iox::capro::ServiceDescription& service)
{
    // Create objects in the pool.
    auto rawSubscriberPtr = s_subscriberPool.create(
                    std::forward<const iox::capro::ServiceDescription>(service));
    auto rawDataWriterPtr = s_dataWriterPool.create(
                    std::forward<const iox::dds::IdString>(service.getServiceIDString()),
                    std::forward<const iox::dds::IdString>(service.getInstanceIDString()),
                    std::forward<const iox::dds::IdString>(service.getEventIDString()));

    // Wrap in smart pointer with custom deleter to ensure automatic cleanup.
    auto subscriberPtr = SubscriberPtr(rawSubscriberPtr, [](subscriber_t* p) -> void {s_subscriberPool.free(p);});
    auto dataWriterPtr = DataWriterPtr(rawDataWriterPtr, [](data_writer_t* p) -> void {s_dataWriterPool.free(p);});

    return Channel(service, subscriberPtr, dataWriterPtr);
}

template<typename subscriber_t, typename data_writer_t>
inline iox::capro::ServiceDescription Channel<subscriber_t, data_writer_t>::getService()
{
    return this->service;
}

template<typename subscriber_t, typename data_writer_t>
inline std::shared_ptr<subscriber_t> Channel<subscriber_t, data_writer_t>::getSubscriber()
{
    return this->subscriber;
}

template<typename subscriber_t, typename data_writer_t>
inline std::shared_ptr<data_writer_t> Channel<subscriber_t, data_writer_t>::getDataWriter()
{
    return this->dataWriter;
}

// ======================================== Public ======================================== //
template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::Iceoryx2DDSGateway()
    : gateway_t(iox::capro::Interfaces::DDS)
{
    m_channelFactory = Channel<subscriber_t, data_writer_t>::create;
};

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::Iceoryx2DDSGateway(ChannelFactory channelFactory)
    : gateway_t(iox::capro::Interfaces::DDS)
{
    m_channelFactory = channelFactory;
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::~Iceoryx2DDSGateway()
{
    m_channels.clear();
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline void Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::discoveryLoop() noexcept
{
    iox::LogDebug() << "[Iceoryx2DDSGateway] Starting discovery.";
    m_runDiscoveryLoop.store(true, std::memory_order_relaxed);
    while (m_runDiscoveryLoop.load(std::memory_order_relaxed))
    {
        iox::capro::CaproMessage msg;
        while (static_cast<gateway_t*>(this)->getCaProMessage(msg))
        {
            discover(msg);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(iox::gateway::dds::DISCOVERY_PERIOD_MS));
    }
    iox::LogDebug() << "[Iceoryx2DDSGateway] Stopped discovery.";
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline void Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::discover(const iox::capro::CaproMessage& msg) noexcept
{
    iox::LogDebug() << "[Iceoryx2DDSGateway] <CaproMessage> "
                    << iox::capro::CaproMessageTypeString[static_cast<uint8_t>(msg.m_type)]
                    << " { Service: " << msg.m_serviceDescription.getServiceIDString()
                    << ", Instance: " << msg.m_serviceDescription.getInstanceIDString()
                    << ", Event: " << msg.m_serviceDescription.getEventIDString() << " }";

    // Ignore instrospection ports
    if (msg.m_serviceDescription.getServiceIDString() == iox::capro::IdString("Introspection"))
    {
        return;
    }
    // Ignore services
    if (msg.m_subType == iox::capro::CaproMessageSubType::SERVICE)
    {
        return;
    }

    switch (msg.m_type)
    {
    case iox::capro::CaproMessageType::OFFER:
    {
        auto channel = setupChannel(msg.m_serviceDescription);
        channel.getSubscriber()->subscribe(SUBSCRIBER_CACHE_SIZE);
        channel.getDataWriter()->connect();
        break;
    }
    case iox::capro::CaproMessageType::STOP_OFFER:
    {
        takeDownChannel(msg.m_serviceDescription);
        break;
    }
    default:
    {
        break;
    }
    }
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline void Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::forwardingLoop() noexcept
{
    iox::LogDebug() << "[Iceoryx2DDSGateway] Starting forwarding.";
    m_runForwardingLoop.store(true, std::memory_order_relaxed);
    while (m_runForwardingLoop.load(std::memory_order_relaxed))
    {
        forward();
        std::this_thread::sleep_for(std::chrono::milliseconds(iox::gateway::dds::FORWARDING_PERIOD_MS));
    };
    iox::LogDebug() << "[Iceoryx2DDSGateway] Stopped forwarding.";
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline void Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::forward() noexcept
{
    auto index = 0;
    for (auto& channel : m_channels)
    {
        auto subscriber = channel.getSubscriber();
        auto writer = channel.getDataWriter();
        if (subscriber->hasNewChunks())
        {
            const iox::mepoo::ChunkHeader* header;
            subscriber->getChunk(&header);
            if (header->m_info.m_payloadSize > 0)
            {
                writer->write(static_cast<uint8_t*>(header->payload()), header->m_info.m_payloadSize);
            }
            subscriber->releaseChunk(header);
        }
        index++;
    }
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline size_t Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::getNumberOfChannels() const noexcept
{
    return m_channels.size();
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline void Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::shutdown() noexcept
{
    iox::LogDebug() << "[Iceoryx2DDSGateway] Shutting down Posh2DDSGateway.";
    m_runDiscoveryLoop.store(false, std::memory_order_relaxed);
    m_runForwardingLoop.store(false, std::memory_order_relaxed);
}

// ======================================== Private ======================================== //

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
Channel<subscriber_t, data_writer_t> Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::setupChannel(const iox::capro::ServiceDescription& service)
{
    iox::LogDebug() << "[Iceoryx2DDSGateway] Channel set up for service: "
                    << "/" << service.getInstanceIDString() << "/" << service.getServiceIDString() << "/"
                    <<  service.getEventIDString();
    auto channel = m_channelFactory(service);
    m_channels.push_back(channel);
    return channel;
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
void Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::takeDownChannel(const iox::capro::ServiceDescription& service)
{

    for(auto& channel : m_channels)
    {
        if(channel.getService() == service)
        {
            m_channels.erase(&channel);
            iox::LogDebug() << "[Iceoryx2DDSGateway] Channel taken down for service: "
                            << "/" << service.getInstanceIDString() << "/" << service.getServiceIDString() << "/"
                            <<  service.getEventIDString();
            break;
        }
    }

}

} // namespace dds
} // namespace gateway
} // namespace iox
