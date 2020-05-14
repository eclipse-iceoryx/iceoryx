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
        std::this_thread::sleep_until(std::chrono::steady_clock::now()
                                      + std::chrono::milliseconds(iox::gateway::dds::DISCOVERY_PERIOD_MS));
    }
    iox::LogDebug() << "[Iceoryx2DDSGateway] Stopped discovery.";
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline void
Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::discover(const iox::capro::CaproMessage& msg) noexcept
{
    iox::LogDebug() << "[Iceoryx2DDSGateway] <CaproMessage> "
                    << iox::capro::CaproMessageTypeString[static_cast<uint8_t>(msg.m_type)]
                    << " { Service: " << msg.m_serviceDescription.getServiceIDString()
                    << ", Instance: " << msg.m_serviceDescription.getInstanceIDString()
                    << ", Event: " << msg.m_serviceDescription.getEventIDString() << " }";

    if (msg.m_serviceDescription.getServiceIDString() == iox::capro::IdString("Introspection"))
    {
        return;
    }
    if (msg.m_subType == iox::capro::CaproMessageSubType::SERVICE)
    {
        return;
    }

    switch (msg.m_type)
    {
    case iox::capro::CaproMessageType::OFFER:
    {
        auto channel = setupChannelUnsafe(msg.m_serviceDescription);
        channel.getSubscriber()->subscribe(SUBSCRIBER_CACHE_SIZE);
        channel.getDataWriter()->connect();
        break;
    }
    case iox::capro::CaproMessageType::STOP_OFFER:
    {
        discardChannelUnsafe(msg.m_serviceDescription);
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
        std::this_thread::sleep_until(std::chrono::steady_clock::now()
                                      + std::chrono::milliseconds(iox::gateway::dds::FORWARDING_PERIOD_MS));
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
inline size_t Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::getNumberOfChannels() noexcept
{
    const std::lock_guard<std::mutex> lock(m_channelAccessMutex);
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
Channel<subscriber_t, data_writer_t> Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::setupChannelUnsafe(
    const iox::capro::ServiceDescription& service)
{
    auto channel = m_channelFactory(service);
    iox::LogDebug() << "[Iceoryx2DDSGateway] Channel set up for service: "
                    << "/" << service.getInstanceIDString() << "/" << service.getServiceIDString() << "/"
                    << service.getEventIDString();
    const std::lock_guard<std::mutex> lock(m_channelAccessMutex);
    m_channels.push_back(channel);
    return channel;
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
void Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::discardChannelUnsafe(
    const iox::capro::ServiceDescription& service)
{
    const std::lock_guard<std::mutex> lock(m_channelAccessMutex);
    for (auto& channel : m_channels)
    {
        if (channel.getService() == service)
        {
            m_channels.erase(&channel);
            iox::LogDebug() << "[Iceoryx2DDSGateway] Channel taken down for service: "
                            << "/" << service.getInstanceIDString() << "/" << service.getServiceIDString() << "/"
                            << service.getEventIDString();
            break;
        }
    }
}

} // namespace dds
} // namespace gateway
} // namespace iox
